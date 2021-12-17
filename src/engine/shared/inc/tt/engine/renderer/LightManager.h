#if !defined(INC_TT_ENGINE_RENDERER_LIGHTMANAGER_H)
#define INC_TT_ENGINE_RENDERER_LIGHTMANAGER_H

#include <tt/platform/tt_types.h>
#include <tt/engine/scene/fwd.h>

namespace tt {
namespace engine {
namespace renderer {


class LightManager
{
public:
	LightManager();
	~LightManager();
	
	bool addLight   (scene::Light* p_light);
	bool removeLight(scene::Light* p_light);
	
	void reset();
	
private:
	// No copying
	LightManager(const LightManager&);
	LightManager& operator=(const LightManager&);
	
	static const s32 maxActiveLights = 8;
	scene::Light* m_activeLights[maxActiveLights];
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_LIGHTMANAGER_H
