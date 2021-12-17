#include <tt/engine/animation/Animation.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/animation/TransformController.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/platform/tt_error.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace animation {


Animation::Animation(const EngineID& p_id)
:
m_id(p_id),
m_child(),
m_sibling(),
m_animControl(),
m_transformController(),
m_matrix()
{
}


Animation* Animation::create(const fs::FilePtr&, const EngineID& p_id, u32)
{
	return new Animation(p_id);
}


void Animation::setTimeRecursive(real p_time)
{
	m_animControl.setTime(p_time);

	if(m_child != 0)   m_child->setTimeRecursive(p_time);
	if(m_sibling != 0) m_sibling->setTimeRecursive(p_time);
}


bool Animation::load(const fs::FilePtr& p_file)
{
	return loadRecursive(p_file);
}


bool Animation::loadMatrix()
{
	const bool result = m_transformController->getValue(m_animControl.getTime(), m_matrix);
	
	// Load it onto the matrix stack
	renderer::MatrixStack::getInstance()->multiply44(m_matrix);
	
	return result;
}


math::Vector3 Animation::getPosition()
{
	m_transformController->getValue(m_animControl.getTime(), m_matrix);

	return math::Vector3(m_matrix.m_41, m_matrix.m_42, m_matrix.m_43);
}


void Animation::visualize()
{
	m_transformController->drawPath(m_animControl.getEndTime(), 0.1f);
}


s32 Animation::getMemSize() const
{
	// TODO: Take controllers into account
	return sizeof(Animation);
}


bool Animation::loadRecursive(const fs::FilePtr& p_file)
{
	TT_ERR_CREATE("Loading Animation");

	// Load animation control settings
	m_animControl.load(p_file, &errStatus);

	EngineID childID(0,0);
	if(childID.load(p_file) == false)
	{
		return false;
	}

	EngineID siblingID(0,0);
	if(siblingID.load(p_file) == false)
	{
		return false;
	}
	
	// Load controller data
	m_transformController.reset(new TransformController);
	m_transformController->load(p_file, &errStatus);

	// Load the hierarchy
	if(childID.valid())
	{
		m_child = AnimationCache::get(childID, false);
		if(m_child == 0) return false;
	}

	if(siblingID.valid())
	{
		m_sibling = AnimationCache::get(siblingID, false);
		if(m_sibling == 0) return false;
	}

	TT_ERR_ASSERT_ON_ERROR();

	return true;
}


void Animation::resolveEndTime(real& p_time, bool p_recursive)
{
	p_time = std::max(p_time, m_transformController->getEndTime());

	if(p_recursive)
	{
		if(m_child != 0)
		{
			m_child->resolveEndTime(p_time);
		}
		if(m_sibling != 0)
		{
			m_sibling->resolveEndTime(p_time);
		}
	}
}

// Namespace end
}
}
}

