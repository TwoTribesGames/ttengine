#if !defined(INC_TT_ENGINE_RENDERER_SHADOWVOLUMEMANAGER_H)
#define INC_TT_ENGINE_RENDERER_SHADOWVOLUMEMANAGER_H


#include <list>

#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/scene/fwd.h>


namespace tt {
namespace engine {
namespace renderer {

class ShadowVolumeManager
{
public:
	/*! \brief Initialize shadow volumes, returns false if not supported */
	static bool init  (const ColorRGBA& p_shadowColor);
	static void destroy();
	
	static void queueForRender(const scene::InstancePtr& p_instance);
	
	static void renderShadows();
	
private:
	static void initShadowVolumePass();
	static void beginShadowPass();
	static void endShadowPass();
	
	static MaterialPtr ms_material;
	static u32         ms_colorMask;
	static bool        ms_depthEnabled;
	static bool        ms_depthWriteEnabled;
	
	typedef std::list<scene::InstancePtr> ShadowVolumes;
	static ShadowVolumes ms_shadowVolumes;
	
	static bool ms_initialized;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_SHADOWVOLUMEMANAGER_H)
