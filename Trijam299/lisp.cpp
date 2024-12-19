#include <gc.h>
#include <cassert>
#include <string>
#include <unordered_map>

struct str {
	char *s;
	int start;
	int end;
};

enum datatype {
	DATA_NUMBER,
	DATA_SYMBOL,
	DATA_STRING,
	DATA_PAIR
};

struct data {
	datatype type;
	union {
		str str;
		float num;
		int sym;
		struct {
			data *left;
			data *right;
		};
	};
};

#define S(st) (str{.s=GC_strdup(st),.start=0,.end=(int)strlen(st)})

static str substring(str s, int amount) {
	s.start += amount;
	return s;
}

static char *strdup(str s) {
	int len = s.end - s.start;
	char *c = (char *)GC_MALLOC(len + 1);
	for (int i = 0; i < len; i++) {
		c[i] = s.s[s.start + i];
	}
	c[len] = 0;
	return c;
}

static bool hasmore(str s) {
	return s.start < s.end;
}

static bool whitespace(char c) {
	if (c == ' ')
		return true;
	if (c == '\t')
		return true;
	if (c == '\n')
		return true;
}

struct readsymbolret {
	str sym;
	str rem;
};

readsymbolret readstring(str s) {
	str n = s;
	n.end = n.start;
	while (hasmore(s) && s.s[0] != '"') {
		n.end++;
		s.start++;
	}
	while (hasmore(s) && (s.s[0] == '"' || whitespace(s.s[0])))
		s.start++;
	return { n, s };
}

readsymbolret readsymbol(str s) {
	while (hasmore(s) && whitespace(s.s[0]))
		s.start++;
	if (s.s[0] == '(')
		return { S("("), substring(s, 1) };
	if (s.s[0] == '\'')
		return { S("'"), substring(s, 1) };
	if (s.s[0] == '`')
		return { S("`"), substring(s, 1) };
	if (s.s[0] == ',')
		return { S(","), substring(s, 1) };
	if (s.s[0] == '"')
		return readstring(s);
	str n = s;
	n.end = n.start;
	bool first = true;
	while (hasmore(s) && !whitespace(s.s[0])) {
		if (!first && s.s[0] == ')')
			break;
		first = false;
		n.end++;
		s.start++;
	}
	while (hasmore(s) && whitespace(s.s[0]))
		s.start++;
	return { n, s };
}

#define nil nullptr

static data *cons(data *a, data *b) {
	assert(a != nil);
	data *d = (data *)GC_MALLOC(sizeof(*d));
	d->type = DATA_PAIR;
	d->left = a;
	d->right = b;
}

static data *car(data *a) {
	assert(a != nil);
	assert(a->type == DATA_PAIR);
	return a->left;
}
static data *cdr(data *a) {
	assert(a != nil);
	assert(a->type == DATA_PAIR);
	return a->right;
}

static data *reverse(data *a) {
	assert(a != nil);
	assert(a->type == DATA_PAIR);
	data *b = nil;
	while (a != nil) {
		b = cons(car(a), b);
		a = cdr(a);
	}
	return b;
}

static std::unordered_map<int, char *> symbols;

static str print(data *d) {
	if (d == nil)
		return S("nil");
	if (d->type == DATA_NUMBER) {
		std::string s = std::to_string(d->num);
		return S(s.c_str());
	}
	else if (d->type == DATA_SYMBOL) {
		return S(symbols[d->sym]);
	}
}

/*
let whitespace = c => c[0] == ' ' || c[0] == '\t' || c[0] == '\n'

let readstring = s => {
	let n = '"'
	s = s.substring(1)
	while (s[0] != undefined && s[0] != '"') {
		n += s[0]
		s = s.substring(1)
	}
	n += '"'
	while (s[0] != undefined && (s[0] == '"' || whitespace(s)))
		s = s.substring(1)
	return [n, s]
}

let readsymbol = s => {
	while (s[0] != undefined && whitespace(s))
		s = s.substring(1)
	if (s[0] == '(')
		return ['(', s.substring(1)]
	if (s[0] == '\'')
		return ['\'', s.substring(1)]
	if (s[0] == '`')
		return ['`', s.substring(1)]
	if (s[0] == ',')
		return [',', s.substring(1)]
	if (s[0] == '"')
		return readstring(s)
	let n = ''
	let first = true
	while (s[0] != undefined && !whitespace(s)) {
		if (!first && s[0] == ')')
			break
		first = false
		n += s[0]
		s = s.substring(1)
	}
	while (s[0] != undefined && whitespace(s))
		s = s.substring(1)
	return [n, s]
}

let nil = []

let cons = (a, b) => [a, b]
let reverse = a => {
	let b = nil
	while (a != nil) {
		b = cons(a[0], b)
		a = a[1]
	}
	return b
}

let print = c => {
	let s = ''
	if (typeof(c) == 'object') {
		s += '('
		let C = c
		let first = true
		while (C != nil) {
			if (!first)
				s += ' '
			first = false
			s += print(C[0])
			C = C[1]
		}
		s += ')'
	}
	else {
		s += c
	}
	return s
}

let parse = s => {
	let S = readsymbol(s)
	s = S[1]

	if (S[0] == '\'') {
		S = parse(s)
		s = S[1]
		return [cons('QUOTE', cons(S[0], nil)), s]
	}

	if (S[0] == '`') {
		S = parse(s)
		s = S[1]
		return [cons('BACKQUOTE', cons(S[0], nil)), s]
	}

	if (S[0] == ',') {
		S = parse(s)
		s = S[1]
		return [cons('COMMA', cons(S[0], nil)), s]
	}

	if (S[0] == '(') {
		let c = nil
		while (s != '') {
			S = readsymbol(s)
			if (S[0] == ')')
				return [reverse(c), S[1]]
			S = parse(s)
			s = S[1]
			c = cons(S[0], c)
		}
		console.log("ERROR: Unclosed parenthesies")
		return nil
	}

	if (S[0][0] == '"')
		return [S[0], s]

	if (!isNaN(Number.parseFloat(S[0])))
		return [Number.parseFloat(S[0]), s]

	return [S[0].toUpperCase(), s]
}

let islist = (a) => typeof(a) == 'object'
let isnumber = (a) => typeof(a) == 'number'
let issymbol = (a) => typeof(a) == 'string' && a[0] != '"'
let isstring = (a) => typeof(a) == 'string' && a[0] == '"'
let car = (a) => a[0]
let cdr = (a) => a[1]

let lispdefs = {}

let exec = (c, l) => {
	if (isstring(c))
		return c;
	if (isnumber(c))
		return c;
	if (issymbol(c)) {
		if (l.hasOwnProperty(c))
			return l[c]
		return lispdefs[c]
	}
	if (islist(c)) {
		if (car(c) == 'QUOTE')
			return car(cdr(c))
		if (car(c) == 'BACKQUOTE')
			return execbackquote(car(cdr(c)), l)
		if (car(c) == 'LAMBDA') {
			let cmd = nth(2, c)
			return ['f', (C, _l) => {
				let L = structuredClone(l)
				let n = 0
				let args = listmap(nth(1, c), x => {
					let r = cons(x, cons(cons('QUOTE', cons(nth(n, C), nil)), nil))
					n++
					return r
				})
				return exec(cons('LET', cons(args, cons(cmd, nil))), l)
			}]
		}
		if (car(c) == 'COMMA')
			return exec(exec(car(cdr(c)), l), l)
		if (car(c) == 'LET') {
			let vars = nth(1, c)
			let cmd = nth(2, c)
			let L = structuredClone(l)
			while (vars != nil) {
				L[nth(0, car(vars))] = exec(nth(1, car(vars)), l)
				vars = cdr(vars)
			}
			return exec(cmd, L)
		}
		if (car(c) == 'PROGN') {
			let r = nil
			let C = cdr(c)
			while (C != nil) {
				r = exec(car(C), l)
				C = cdr(C)
			}
			return r
		}
		let fn = exec(car(c), l)
		if (fn[0] == 'm') {
			return exec(fn[1](cdr(c), l), l)
		} else {
			let args = listmap(cdr(c), x => exec(x, l))
			return fn[1](args, l)
		}
	}
	console.log("UNKNOWN")
	return undefined
}

let listmap = (c, a) => {
	let C = nil
	while (c != nil) {
		C = cons(a(car(c)), C)
		c = cdr(c)
	}
	return reverse(C)
}

let execbackquote = (c, l) => {
	if (islist(c)) {
		if (car(c) == 'COMMA') {
			return exec(car(cdr(c)), l)
		}
		return listmap(c, C => execbackquote(C, l))
	}
	return c
}

let nthcdr = (n, c) => {
	while (n > 0) {
		c = cdr(c)
		n--
	}
	return c
}

let nth = (n, c) => car(nthcdr(n, c))

let count = (c) => {
	let n = 0
	while (c != nil) {
		n++
		c = cdr(c)
	}
	return n
}

lispdefs['SET'] = ['f', a => {
	let name = nth(0, a)
	let value = nth(1, a)
	if (!issymbol(name)) {
		console.log("ERROR: SET name must be symbol")
		return
	}
	lispdefs[name] = value
}]

lispdefs['+'] = ['f', (a, _l) => nth(0, a) + nth(1, a)]
lispdefs['-'] = ['f', (a, _l) => nth(0, a) - nth(1, a)]
lispdefs['*'] = ['f', (a, _l) => nth(0, a) * nth(1, a)]
lispdefs['/'] = ['f', (a, _l) => nth(0, a) / nth(1, a)]
lispdefs['**'] = ['f', (a, _l) => nth(0, a) ** nth(1, a)]
lispdefs['='] = ['f', (a, _l) => nth(0, a) == nth(1, a)]
lispdefs['NOT'] = ['f', (a, _l) => !car(a)]
lispdefs['!='] = ['f', (a, _l) => nth(0, a) != nth(1, a)]
lispdefs['EXEC'] = ['f', (a, l) => exec(car(a), l)]
lispdefs['CAR'] = ['f', (a, _l) => car(car(a))]
lispdefs['CDR'] = ['f', (a, _l) => cdr(car(a))]
lispdefs['NTH'] = ['f', (a, _l) => nth(nth(0, a), nth(1, a))]
lispdefs['NTHCDR'] = ['f', (a, _l) => nthcdr(nth(0, a), nth(1, a))]
lispdefs['MAP'] = ['f', (a, _l) => listmap(nth(1, a), nth(0, a))]
lispdefs['REV'] = ['f', (a, _l) => reverse(car(a))]

lispdefs['DEFMACRO'] = ['m', (a, l) => {
	let name = nth(0, a)
	let args = nth(1, a)
	let cmd = nth(2, a)
	let fn = (C, p) => {
		let L = structuredClone(l)
		let n = 0
		let arg = listmap(args, x => {
			let r = cons(x, cons(cons('QUOTE', cons(nth(n, C), nil)), nil))
			n++
			return r
		})
		console.log(print(arg))
		let val = cons('LET', cons(arg, cons(cmd, nil)))
		if (p)
			return cons('QUOTE', cons(exec(val, l), nil))
		return exec(val, l)
	}
	lispdefs[name] = ['m', (C, _l) => fn(C, false)]
	lispdefs[name + ":EXPAND"] = ['m', (C, _l) => fn(C, true)]
	return cons('QUOTE', cons(name, nil))
}]

let execs = s => exec(parse(s)[0], {})
let execm = s => execs("(progn " + s + ")")

execs(`
(defmacro defun (name args body)
	\`(progn
		(set ',name (lambda ,args ,body))
		',name))
`)

let pep = s => print(execs(s))
*/