#include "global.h"

// GlobState globstate;

SERIALIZER(GlobState) {
	SER_CHECK;
} SERIALIZER_END

void LoadGlobState()
{
	//R r = RRead("save.dat");
	//if (r.file) {
	//	SER_REV(r);
	//	SERIALIZE(r, globstate);
	//}
	//RClose(r);
}

void SaveGlobState() {
	/*R r = RWrite("save.dat");
	if (r.file) {
		SER_REV(r);
		SERIALIZE(r, globstate);
	}
	RClose(r);*/
}