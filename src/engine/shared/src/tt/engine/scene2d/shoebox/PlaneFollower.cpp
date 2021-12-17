#include <tt/engine/scene2d/shoebox/PlaneFollower.h>
#include <tt/engine/scene2d/PlaneScene.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {


//--------------------------------------------------------------------------------------------------
// Public member functions

tt::math::Vector3 PlaneFollower::getPosition() const
{
	TT_NULL_ASSERT(m_parent);
	
	if (m_parent != 0)
	{
		return m_offset * m_parent->getMatrix();
	}
	
	return tt::math::Vector3::zero;
}


bool PlaneFollower::isCulled() const
{
	TT_NULL_ASSERT(m_parent);
	
	if (m_parent != 0)
	{
		return m_parent->isCulled();
	}
	
	return false;
}

// Namespace end
}
}
}
}
