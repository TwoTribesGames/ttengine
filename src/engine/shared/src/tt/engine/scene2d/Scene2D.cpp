///////////////////////////////////////////////////////////////////////////////
///  Description   : Interface for renderable objects
///                  previously known as RenderNode

#include <tt/code/helpers.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/scene2d/Scene2D.h>
#include <tt/engine/scene2d/SceneInterface.h>


namespace tt {
namespace engine {
namespace scene2d {

Scene2D::Scene2D()
:
m_priority(0),
m_position(),
m_screenSpace(false),
m_scene(0),
m_blendMode(renderer::BlendMode_Blend),
m_fogEnabled(true)
{
}


Scene2D::~Scene2D()
{
}


void Scene2D::registerScene(SceneInterface* p_scene)
{
	// Nodes cannot be inserted in more than 1 scene at once
	TT_ASSERTMSG((m_scene == 0 || m_scene == p_scene),
	             "Node already registered with another scene.");
	
	// Store scene pointer
	m_scene = p_scene;
}


void Scene2D::unregisterScene()
{
	m_scene = 0;
}


void Scene2D::setDepth(real p_depth)
{
	if(m_position.z != p_depth)
	{
		// Change current depth
		m_position.z = p_depth;
		
		// If this node is part of a scene, trigger re-insert
		resort();
		onPositionChanged();
	}
}


void Scene2D::setPriority(s32 p_priority)
{
	if(m_priority != p_priority)
	{
		// Change priority
		m_priority = p_priority;
		
		// If this node is part of a scene, trigger re-insert
		resort();
	}
}


void Scene2D::setPosition(real p_x, real p_y)
{
	m_position.x = p_x;
	m_position.y = p_y;
	onPositionChanged();
}


tt::math::Vector3 Scene2D::getPosition() const
{
	return m_position;
}


void Scene2D::setScreenSpace(bool p_screenSpace)
{
	m_screenSpace = p_screenSpace;
}


bool Scene2D::isScreenSpace() const
{
	return m_screenSpace;
}


void Scene2D::setBlendMode(renderer::BlendMode p_mode)
{
	m_blendMode = p_mode;
}


renderer::BlendMode Scene2D::getBlendMode() const
{
	return m_blendMode;
}


//--------------------------------------------------------------------------------------------------
// Protected

Scene2D::Scene2D(const Scene2D& p_rhs)
:
m_priority   (p_rhs.m_priority),
m_position   (p_rhs.m_position),
m_screenSpace(p_rhs.m_screenSpace),
m_scene      (p_rhs.m_scene),
m_blendMode  (p_rhs.m_blendMode),
m_fogEnabled (p_rhs.m_fogEnabled)
{
	TT_ASSERTMSG(m_scene == 0,
		"Trying to copy a Scene2D that has a non-zero m_scene member, \
		SceneInterface will not be copied correctly!");
}


void Scene2D::resort()
{
	if(m_scene != 0)
	{
		m_scene->rearrange(this);
	}
}


//namespace end
}
}
}
