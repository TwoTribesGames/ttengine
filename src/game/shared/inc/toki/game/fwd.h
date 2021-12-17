#if !defined(INC_TOKI_GAME_FWD_H)
#define INC_TOKI_GAME_FWD_H


#include <tt/platform/tt_types.h>


namespace toki {
namespace game {

class AttributeDebugView;
typedef tt_ptr<AttributeDebugView>::shared AttributeDebugViewPtr;

class Border;
typedef tt_ptr<Border>::shared BorderPtr;

class Camera;
class CameraEffect;
typedef tt_ptr<CameraEffect>::shared CameraEffectPtr;
typedef tt_ptr<CameraEffect>::weak   CameraEffectWeakPtr;

class CameraMgr;

class CheckPointMgr;
typedef tt_ptr<CheckPointMgr>::shared CheckPointMgrPtr;

class DemoMgr;

//#if !defined(TT_BUILD_FINAL)
class DebugView;
typedef tt_ptr<DebugView>::shared DebugViewPtr;
//#endif

class Game;

class Minimap;
typedef tt_ptr<Minimap>::shared MinimapPtr;

class StartInfo;

// Namespace end
}
}


#endif  // !defined(INC_TOKI_GAME_FWD_H)
