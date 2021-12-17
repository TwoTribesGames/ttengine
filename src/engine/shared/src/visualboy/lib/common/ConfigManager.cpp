#include "ConfigManager.h"
#include "gba/GBA.h"
#include "gba/remote.h"

bool cpuIsMultiBoot = false;;
bool mirroringEnable = true;
bool speedHack = false;
bool speedup = false;
const char *biosFileNameGB  = nullptr;
const char *biosFileNameGBA = nullptr;
const char *loadDotCodeFile = nullptr;
const char *saveDotCodeFile = nullptr;
int cheatsEnabled = false;
int cpuDisableSfx = false;
int layerEnable = 0xff00;
int layerSettings = 0xff00;
int rtcEnabled = false;
int saveType = GBA_SAVE_AUTO;
bool skipBios = false;
bool skipSaveGameBattery = false;
bool skipSaveGameCheats = false;
bool useBios = false;
int throttle = 100;
int speedup_throttle = 0;
int speedup_frame_skip = 9;
FilterFunc filterFunction = 0;

// MR: Below are values that are not declared in ConfigManager.h; but somewhere else in the code
int cpuSaveType = 0;
bool parseDebug = true;
void(*dbgMain)() = nullptr;
void(*dbgSignal)(int, int) = nullptr;
void(*dbgOutput)(const char *, uint32_t) = nullptr;
