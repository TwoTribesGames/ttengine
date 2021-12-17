#if !defined(INC_TT_ENGINE_SCENE_SCENE_H)
#define INC_TT_ENGINE_SCENE_SCENE_H

#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/engine/scene/fwd.h>
#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/file/FileType.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace scene {

typedef cache::ResourceCache<Scene> SceneCache;


class Scene
{
public:
	static const file::FileType fileType = file::FileType_Scene;
	static const bool hasResourceHeader = true;

	Scene(const EngineID& p_id, u32 p_flags = 0);
	~Scene();

	static Scene* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	
	inline const EngineID& getEngineID() const {return m_id;}
	
	bool load(const fs::FilePtr& p_file);

	inline void add(const InstancePtr& p_instance) {m_instances.push_back(p_instance);}

	void update();
	void render();

	inline s32 getMemSize() const {return sizeof(Scene);}

	void setFog(s32 p_index);
	void setLightSet(s32 /*p_index*/) {}
	
private:
	EngineID m_id;

	typedef std::vector<FogPtr> FogTable;
	FogTable m_fogTable;

	typedef std::vector<LightPtr> Lights;
	Lights m_lights;

	typedef std::vector<CameraPtr> Cameras;
	Cameras m_cameras;

	typedef std::vector<InstancePtr> Instances;
	Instances m_instances;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_SCENE_H
