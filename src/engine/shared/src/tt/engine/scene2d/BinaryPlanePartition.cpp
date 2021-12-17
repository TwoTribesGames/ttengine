#include <tt/code/helpers.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#ifndef TT_BUILD_FINAL
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#endif
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene2d/BinaryPlanePartition.h>
#include <tt/engine/scene2d/PlaneScene.h>
#include <iterator>


namespace tt {
namespace engine {
namespace scene2d {

BinaryPlanePartition::BinaryPlanePartition()
:
m_planes(),
m_notPartitionedPlanes(),
m_visiblePlanes(),
m_lastRect(),
m_partition(0)
{
}


BinaryPlanePartition::~BinaryPlanePartition()
{
	clear();
}


void BinaryPlanePartition::addPlane(PlaneScene* p_plane)
{
	TT_ASSERTMSG(p_plane != 0, "No plane specified.");
	
	if (p_plane->getAnimations() != 0 &&
	    ( //p_plane->animations->hasScaleAnimation() ||
	      p_plane->getAnimations()->hasRotationAnimation()))
	{
		m_notPartitionedPlanes.emplace_back(static_cast<int>(m_notPartitionedPlanes.size()), p_plane);
	}
	else
	{
		m_planes.emplace_back(static_cast<int>(m_planes.size()), p_plane);
	}
}


void BinaryPlanePartition::partition()
{
	tt::code::helpers::safeDelete(m_partition);
	
	m_partition = new Partition;
	m_partition->planes = m_planes;
	m_partition->balance(8);
}


void BinaryPlanePartition::getVisiblePlanes(const math::VectorRect& p_rect, Planes& p_planesOUT) const
{
	TT_ASSERTMSG(m_partition != 0,"Please call partition() prior to calling getVisiblePlanes.");
	
	m_partition->getVisiblePlanes(p_rect, p_planesOUT);
}


void BinaryPlanePartition::initialUpdate(real p_deltaTime)
{
	for (Planes::iterator it = m_notPartitionedPlanes.begin();
	     it != m_notPartitionedPlanes.end(); ++it)
	{
		(*it).second->update(p_deltaTime);
	}
	
	for (Planes::iterator it = m_planes.begin(); it != m_planes.end(); ++it)
	{
		(*it).second->update(p_deltaTime);
	}
}


void BinaryPlanePartition::update(real p_deltaTime)
{
	for (Planes::iterator it = m_notPartitionedPlanes.begin();
	     it != m_notPartitionedPlanes.end(); ++it)
	{
		if ((*it).second->needsUpdate())
		{
			(*it).second->update(p_deltaTime);
		}
	}
	
	for (Planes::iterator it = m_planes.begin(); it != m_planes.end(); ++it)
	{
		if ((*it).second->needsUpdate())
		{
			(*it).second->update(p_deltaTime);
		}
	}
}


class PlanePriorityPredicate
{
public:
	typedef BinaryPlanePartition::Plane first_argument_type;
	typedef BinaryPlanePartition::Plane second_argument_type;
	typedef bool result_type;
	
	inline bool operator()(const first_argument_type& p_lhs, const second_argument_type& p_rhs) const
	{
		if (p_lhs.second->getPriority() == p_rhs.second->getPriority())
		{
			return p_lhs.first < p_rhs.first;
		}
		return p_lhs.second->getPriority() < p_rhs.second->getPriority();
	}
};


void BinaryPlanePartition::render(const math::VectorRect& p_rect) const
{
	if (p_rect != m_lastRect)
	{
		m_lastRect = p_rect;
		m_visiblePlanes.clear();
		if (m_partition != 0)
		{
			m_partition->getVisiblePlanes(p_rect, m_visiblePlanes);
		}
		
		// Always render the m_notPartitionedPlanes.
		std::copy(m_notPartitionedPlanes.begin(), m_notPartitionedPlanes.end(), std::back_inserter(m_visiblePlanes));
		
		std::sort(m_visiblePlanes.begin(), m_visiblePlanes.end(), PlanePriorityPredicate());
	}
	
	for (Planes::const_iterator it = m_visiblePlanes.begin(); it != m_visiblePlanes.end(); ++it)
	{
		(*it).second->render();
	}
	
	// Restore Texture Transform
	engine::renderer::MatrixStack::getInstance()->resetTextureMatrix();
}


#ifndef TT_BUILD_FINAL
void BinaryPlanePartition::renderDebug() const
{
	if (m_partition != 0)
	{
		m_partition->renderDebug();
	}
}
#endif


class PlanePositionPredicate
{
public:
	typedef BinaryPlanePartition::Plane first_argument_type;
	typedef BinaryPlanePartition::Plane second_argument_type;
	typedef bool result_type;
	
	PlanePositionPredicate(bool p_horizontal)
	:
	m_horizontal(p_horizontal)
	{}
	
	inline bool operator()(const first_argument_type& p_lhs, const second_argument_type& p_rhs) const
	{
		if (m_horizontal)
		{
			return p_lhs.second->getPosition().x < p_rhs.second->getPosition().x;
		}
		else
		{
			return p_lhs.second->getPosition().y < p_rhs.second->getPosition().y;
		}
	}
	
private:
	bool m_horizontal;
};


bool BinaryPlanePartition::Partition::balance(int p_maxPlanes)
{
	if (left != 0 || right != 0)
	{
		TT_PANIC("Attempt to balance an already balanced partition!");
		return false;
	}
	
	// calculate bounding rectangle
	tt::math::Vector2 min;
	tt::math::Vector2 max;
	for (Planes::iterator it = planes.begin(); it != planes.end(); ++it)
	{
		PlaneScene* plane((*it).second);
		if (it == planes.begin())
		{
			min.x = plane->getBoundingRect().getLeft();
			min.y = plane->getBoundingRect().getTop();
			max.x = plane->getBoundingRect().getRight();
			max.y = plane->getBoundingRect().getBottom();
		}
		else
		{
			min.x = std::min(min.x, plane->getBoundingRect().getLeft());
			min.y = std::min(min.y, plane->getBoundingRect().getTop());
			max.x = std::max(max.x, plane->getBoundingRect().getRight());
			max.y = std::max(max.y, plane->getBoundingRect().getBottom());
		}
	}
	rect = tt::math::VectorRect(min, max);
	
	if (static_cast<int>(planes.size()) <= p_maxPlanes)
	{
		// small enough, done
		return true;
	}
	
	// sort all planes (horizontally or vertically)
	std::sort(planes.begin(), planes.end(), PlanePositionPredicate(rect.getWidth() >= rect.getHeight()));
	
	// cut the partition in half
	
	left  = new Partition;
	right = new Partition;
	
	Planes::iterator middle = planes.begin() + (static_cast<int>(planes.size()) / 2);
	std::copy(planes.begin(), middle, std::back_inserter(left->planes));
	std::copy(middle, planes.end(), std::back_inserter(right->planes));
	
	if ((left->balance(p_maxPlanes) == false) ||
	    (right->balance(p_maxPlanes) == false))
	{
		tt::code::helpers::safeDelete(left);
		tt::code::helpers::safeDelete(right);
		return false;
	}
	planes.clear();
	return true;
}


void BinaryPlanePartition::Partition::getVisiblePlanes(const math::VectorRect& p_rect, Planes& p_planesOUT) const
{
	if (rect.intersects(p_rect) == false)
	{
		return;
	}
	
	if (left == 0 || right == 0)
	{
		std::copy(planes.begin(), planes.end(), std::back_inserter(p_planesOUT));
	}
	else
	{
		TT_ASSERT(left != 0);
		TT_ASSERT(right != 0);
		left->getVisiblePlanes(p_rect, p_planesOUT);
		right->getVisiblePlanes(p_rect, p_planesOUT);
	}
}


void BinaryPlanePartition::clear()
{
	code::helpers::freePairSecondContainer(m_notPartitionedPlanes);
	code::helpers::freePairSecondContainer(m_planes);
	code::helpers::safeDelete(m_partition);
	m_visiblePlanes.clear();
	m_lastRect.setWidth(0);
	m_lastRect.setHeight(0);
}


#ifndef TT_BUILD_FINAL
void BinaryPlanePartition::Partition::renderDebug(const renderer::ColorRGB& p_color) const
{
	if (left == 0 || right == 0)
	{
		renderer::Renderer::getInstance()->getDebug()->renderRect(p_color, rect);
	}
	else
	{
		left->renderDebug();
		right->renderDebug();
	}
}
#endif


//namespace end
}
}
}


