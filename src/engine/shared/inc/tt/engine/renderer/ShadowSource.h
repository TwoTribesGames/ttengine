#if !defined(INC_TT_ENGINE_RENDERER_SHADOWSOURCE_H)
#define INC_TT_ENGINE_RENDERER_SHADOWSOURCE_H


#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene/fwd.h>
#include <tt/math/Vector3.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace renderer {

// NOTE: This is a stub for platforms that do not have shadow mapping implemented

class ShadowSource
{
public:
	ShadowSource(const math::Vector3&, const math::Vector3&, s32, s32, real, bool p_mipmap = true)
	{(void)p_mipmap;}
	~ShadowSource() {}

	void startShadowPass() {}
	void endShadowPass() {}

	void setPosition(const math::Vector3&) {}
	void setTarget  (const math::Vector3&) {}
	void setShadowAlpha(u8) {}
	void setShadowColor(const ColorRGB&) {}

	TexturePtr       getTexture() const {return TexturePtr();}
	scene::CameraPtr getCamera()  const {return scene::CameraPtr();}

	void update() {}
	void visualize() {}

	static ShadowSourcePtr create(const math::Vector3&, const math::Vector3&, real)
	{return ShadowSourcePtr();}

	static ShadowSourcePtr createOrtho(const math::Vector3&, const math::Vector3&, s32)
	{return ShadowSourcePtr();}

	static void setAttenuationEnabled(bool) {}
	static bool isAttenuationEnabled() {return false;}
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_SHADOWSOURCE_H
