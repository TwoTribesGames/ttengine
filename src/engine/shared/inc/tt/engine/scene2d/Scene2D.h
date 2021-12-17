///////////////////////////////////////////////////////////////////////////////
///  Description   : Interface for renderable objects
///                  previously known as RenderNode

#if !defined(INC_TT_ENGINE_SCENE2D_SCENE2D_H)
#define INC_TT_ENGINE_SCENE2D_SCENE2D_H

#include <tt/engine/anim2d/fwd.h>
#include <tt/engine/particles/WorldObject.h>
#include <tt/engine/renderer/enums.h>
#include <tt/engine/scene2d/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {


class Scene2D : public particles::WorldObject
{
public:
	Scene2D();
	virtual ~Scene2D();
	
	/*! \brief Update logic of this node */
	virtual void update(real p_delta_time) = 0;
	
	/*! \brief Render the node to the screen */
	virtual void render() = 0;
	
	/*! \brief Retrieve the height of the node */
	virtual real getHeight() const = 0;
	/*! \brief Retrieve the width of the node */
	virtual real getWidth() const = 0;
	
	/*! \brief Store a pointer to the scene this node belongs to */
	void registerScene(SceneInterface* p_scene);
	/*! \brief Invalidate pointer to current scene */
	void unregisterScene();
	
	/*! \brief Change the depth of the node */
	void setDepth(real p_depth);
	/*! \brief Change the priority of the node */
	void setPriority(s32 p_priority);
	
	/*! \brief Retrieve the depth of the node */
	inline virtual real getDepth() const { return m_position.z; }
	
	/*! \brief Retrieve the priority of the node */
	inline s32  getPriority() const { return m_priority; }
	
	/*! \brief Used for texture sorting. */
	virtual inline void* getMaterialID() const { return 0; }
	
	/*! \brief Set the position of the node */
	void setPosition(real p_x, real p_y);
	/*! \brief Retrieve the position of the node */
	virtual tt::math::Vector3 getPosition() const;
	// TODO: Implement these
	inline virtual real getScale() const { return 1.0; }
	inline virtual real getScaleForParticles() const { return -1.0; }
	
	/*! \brief Sets whether the node is in screenspace */
	void setScreenSpace(bool p_screenSpace);
	/*! \brief Retrieve whether the node is in screenspace */
	bool isScreenSpace() const;
	
	/*! \brief Change the way this node is alpha blended */
	void setBlendMode(tt::engine::renderer::BlendMode p_mode);
	/*! \brief Change the way this node is alpha blended */
	tt::engine::renderer::BlendMode getBlendMode() const;
	
	inline void setFogEnabled(bool p_enable) { m_fogEnabled = p_enable; }
	inline bool isFogEnabled() const { return m_fogEnabled; }
	
	virtual bool isSuitableForBatching() const { return false; }
	virtual bool isPlaneScene() const { return false; }
	
protected:
	Scene2D(const Scene2D&);
	virtual void onPositionChanged() {}
	
	void resort();
	
private:
	// No copying
	Scene2D& operator=(const Scene2D&);
	
	// Member variables common to each node
	s32                       m_priority;
	math::Vector3             m_position;
	bool                      m_screenSpace;
	SceneInterface*           m_scene;
	renderer::BlendMode       m_blendMode;
	bool                      m_fogEnabled;
};

//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_SCENE2D_H_
