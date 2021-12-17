#if !defined(INC_TT_ENGINE_SCENE_EVENTOBJECT_H)
#define INC_TT_ENGINE_SCENE_EVENTOBJECT_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/engine/renderer/fwd.h>


namespace tt {
namespace engine {
namespace scene {

class EventObject : public SceneObject
{
public:
	EventObject();
	virtual ~EventObject();
	
	inline s32 getEventMaterialCount() const  {return m_eventMaterialCount;}
	inline renderer::Material* getMaterial(s32 p_index) {return m_eventMaterials[p_index];}
	
protected:
	virtual bool load(const fs::FilePtr& p_file);
	virtual void calculateBoundingBox();
	virtual void renderObject(renderer::RenderContext& p_renderContext);
	
private:
	// No copying
	EventObject(const EventObject&);
	EventObject& operator=(const EventObject&);
	
	
	math::Vector3 m_AABB[2];
	
	s32                  m_eventMaterialCount;
	renderer::Material** m_eventMaterials;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_EVENTOBJECT_H
