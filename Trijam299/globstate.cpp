#include "global.h"

// GlobState globstate;

SERIALIZER(GlobState) {
	SER_CHECK;
} SERIALIZER_END

void SaveGlobState() {
	/*R r = RWrite("save.dat");
	if (r.file) {
		SER_REV(r);
		SERIALIZE(r, globstate);
	}
	RClose(r);*/
}