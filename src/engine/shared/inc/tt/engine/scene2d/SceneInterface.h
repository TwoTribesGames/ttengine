#if !defined(INC_TT_ENGINE_SCENE2D_SCENEINTERFACE_H)
#define INC_TT_ENGINE_SCENE2D_SCENEINTERFACE_H


#include <tt/engine/scene2d/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {

/*! \brief Interface for defining a scene. */
class SceneInterface
{
public:
	SceneInterface() { }
	virtual ~SceneInterface() { }
	
	/*! \brief Update logic of all scene nodes */
	virtual void update(real p_delta_time) = 0;
	
	/*! \brief Render the scene to the screen */
	virtual void render() = 0;
	
	/*! \brief Insert a render node in the scene */
	virtual void insert(Scene2D* p_node) = 0;
	
	/*! \brief Remove a render node from the scene */
	virtual void remove(Scene2D* p_node) = 0;
	
	/*! \brief Rearrange a render node within the scene */
	virtual void rearrange(Scene2D* p_node) = 0;
	
	/*! \brief Remove all render nodes from the scene */
	virtual void removeAll() = 0;
	
	/*! \brief Delete and remove all render nodes from the scene. */
	virtual void deleteAll() = 0;
	
	/* \brief Indicates whether it is a virtual scene or not */
	virtual bool isVirtual() const = 0;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_SCENEINTERFACE_H)
