#ifndef _CONFIGMANAGER_H
#define _CONFIGMANAGER_H

#include "sdl/filters.h"

#define MAX_CHEATS 16384
extern bool cpuIsMultiBoot;
extern bool mirroringEnable;
extern bool speedHack;
extern bool speedup;
extern const char *biosFileNameGB;
extern const char *biosFileNameGBA;
extern const char *loadDotCodeFile;
extern const char *saveDotCodeFile;
extern int cheatsEnabled;
extern int cpuDisableSfx;
extern int layerEnable;
extern int layerSettings;
extern int rtcEnabled;
extern int saveType;
extern bool skipBios;
extern bool skipSaveGameBattery;
extern bool skipSaveGameCheats;
extern bool useBios;
extern int throttle;
extern int speedup_throttle;
extern int speedup_frame_skip;
extern FilterFunc filterFunction;
#endif
