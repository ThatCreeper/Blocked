#pragma once

struct GlobState {
};
// extern GlobState globstate;

DECL_SERIALIZER(GlobState)

void SaveGlobState();