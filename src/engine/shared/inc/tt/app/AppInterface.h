#if !defined(INC_TT_APP_APPINTERFACE_H)
#define INC_TT_APP_APPINTERFACE_H


#include <tt/app/AppSettings.h>
#include <tt/app/PlatformCallbackInterface.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace app {

class AppInterface : public PlatformCallbackInterface
{
public:
	virtual ~AppInterface() { }
	
	virtual void overrideGraphicsSettings(GraphicsSettings* p_current_OUT, const tt::math::Point2& p_desktopSize)
	{ (void)p_current_OUT; (void)p_desktopSize; }
	virtual bool init() = 0;
	virtual void update(real p_elapsedTime) = 0;
	virtual void render() = 0;
	
	virtual bool platformMenuEnabled() const {return true;}
	virtual bool disabledIconAllowed() const {return true;}
	virtual bool hasUnsavedData() const {return false;}
	virtual engine::renderer::TexturePtr getManual() { return engine::renderer::TexturePtr(); }
};

// Namespace end
}
}


#endif // INC_TT_APP_APPINTERFACE_H
