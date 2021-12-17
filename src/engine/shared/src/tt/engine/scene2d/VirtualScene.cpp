#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene2d/VirtualScene.h>


namespace tt {
namespace engine {
namespace scene2d {

//--------------------------------------------------------------------------------------------------
// Public member functions

VirtualScene::VirtualScene()
:
m_scene_nodes()
{
}


VirtualScene::~VirtualScene()
{
}


void VirtualScene::update(real p_delta_time)
{
	for (SceneList::iterator it = m_scene_nodes.begin(); it != m_scene_nodes.end(); ++it)
	{
		(*it)->update(p_delta_time);
	}
}


void VirtualScene::render()
{
	for (SceneList::const_iterator it = m_scene_nodes.begin(); it != m_scene_nodes.end(); ++it)
	{
		tt::math::Vector3 pos = getPosition();
		
		//matrixpoint
		using tt::engine::renderer::MatrixStack;
		MatrixStack* stack(MatrixStack::getInstance());
		
		stack->setMode(MatrixStack::Mode_Position);
		stack->push();
		stack->translate(pos);
		stack->updateWorldMatrix();
		
		(*it)->render();
		
		stack->pop();
		stack->updateWorldMatrix();
	}
}


void VirtualScene::insert(Scene2D* p_node)
{
	TT_NULL_ASSERT(p_node);
	
	// Search for the first node that is closer than this one
	SceneList::iterator it = m_scene_nodes.begin();
	
	for (; it != m_scene_nodes.end(); ++it)
	{
		if (  (p_node->getDepth() < (*it)->getDepth()) ||
		      ( (p_node->getDepth() == (*it)->getDepth()) && 
		        (p_node->getPriority() < (*it)->getPriority()) ) )
		{
			// Node found, exit loop
			break;
		}
	}
	// Insert node
	m_scene_nodes.insert(it, p_node);
	
	// Bind this scene to the node
	p_node->registerScene(this);
}


void VirtualScene::remove(Scene2D* p_node)
{
	if (p_node != 0)
	{
		// Remove from scene
		m_scene_nodes.remove(p_node);
		
		// Unregister
		p_node->unregisterScene();
	}
}


void VirtualScene::rearrange(Scene2D* p_node)
{
	TT_NULL_ASSERT(p_node);
	
	// Remove the node and insert it again
	remove(p_node);
	insert(p_node);
}


void VirtualScene::removeAll()
{
	// Clear all nodes
	m_scene_nodes.clear();
}


void VirtualScene::deleteAll()
{
	for (SceneList::iterator it = m_scene_nodes.begin(); it != m_scene_nodes.end(); ++it)
	{
		delete *it;
	}
	m_scene_nodes.clear();
}

// Namespace end
}
}
}
