#pragma once

inline R RRead(const char *fname) {
	R r;
	r.reading = true;
	fopen_s(&r.file, fname, "r");
	return r;
}

inline R RWrite(const char *fname) {
	R r;
	r.reading = false;
	fopen_s(&r.file, fname, "w");
	char name[256];
	snprintf(name, 256, "%s.debug", fname);
	fopen_s(&r.debugfile, name, "w");
	return r;
}

inline void RClose(R &r) {
	if (r.file)
		fclose(r.file);
	if (r.debugfile)
		fclose(r.debugfile);
}

#define SERIALIZE(r, field) Serialize(r, field, #field)

#define SERIALIZE_COMB(r, field) \
	char _combined2[256]; \
	snprintf(_combined2, 256, "%s->%s", _combined, #field); \
	Serialize(r, field, _combined2)

#define ADD(rev, field) \
	if (r.revision >= rev) { \
		SERIALIZE_COMB(r, s.field); \
	}
#define ADD_CONDITIONAL(rev, cond, field) \
	if (r.revision >= rev && (cond)) { \
		SERIALIZE_COMB(r, s.field); \
	}
#define REM(rev, removed, type, field, def) \
	type field = def; \
	if (r.revision >= rev && r.revision < removed) { \
		SERIALIZE_COMB(r, field); \
	}

inline void SerializeBinary(R &r, void *x, size_t s) {
	if (r.IsReading()) {
		fread(x, s, 1, r.file);
	}
	else {
		fwrite(x, s, 1, r.file);
	}
}
#define SER_BIN(type, fmt) inline void Serialize(R &r, type &s, const char *name) { \
	SerializeBinary(r, &s, sizeof(s)); \
	if (r.IsWriting()) { \
		fprintf(r.debugfile, "%s %s = " fmt "\n", #type, name, s); \
	} \
}

#define SER_CHECK \
	{ \
		int check = r.counter; \
		SERIALIZE(r, check); \
		assert(check == r.counter++); /* no-op when writing */ \
	}

#define SER_REV(r) if (r.IsWriting()) r.revision = SR_LATEST; SERIALIZE(r, r.revision); assert(r.revision <= SR_LATEST)

#define SERIALIZER(type) void Serialize(R &r, type &s, const char *name) { \
	char _combined[256]; \
	snprintf(_combined, 256, "(%s)%s", #type, name);
#define SERIALIZER_END }
#define DECL_SERIALIZER(type) SERIALIZER(type);