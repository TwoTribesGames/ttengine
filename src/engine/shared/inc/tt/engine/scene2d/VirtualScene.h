#if !defined(INC_TT_ENGINE_SCENE2D_VIRTUALSCENE_H)
#define INC_TT_ENGINE_SCENE2D_VIRTUALSCENE_H


#include <list>

#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene2d/SceneInterface.h>


namespace tt {
namespace engine {
namespace scene2d {

/*! \brief Scene for organizing render nodes in a 2D plane. */
class VirtualScene : public Scene2D, public SceneInterface
{
public:
	VirtualScene();
	virtual ~VirtualScene();
	
	/*! \brief Update logic of all scene nodes */
	virtual void update(real p_delta_time);
	
	/*! \brief Render the scene to the screen */
	virtual void render();
	
	/*! \brief Insert a render node in the scene */
	virtual void insert(Scene2D* p_node);
	
	/*! \brief Remove a render node from the scene */
	virtual void remove(Scene2D* p_node);
	
	/*! \brief Rearrange a render node within the scene */
	virtual void rearrange(Scene2D* p_node);
	
	/*! \brief Remove all render nodes from the scene */
	virtual void removeAll();
	
	/*! \brief Delete and remove all render nodes from the scene. */
	virtual void deleteAll();
	
	/* \brief Indicates whether it is a virtual scene or not */
	virtual bool isVirtual() const {return true;}
	
private:
	typedef std::list<Scene2D*> SceneList;
	SceneList m_scene_nodes;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_VIRTUALSCENE_H)
