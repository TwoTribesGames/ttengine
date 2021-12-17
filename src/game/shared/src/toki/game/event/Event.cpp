#include <tt/pres/PresentationMgr.h>
#include <tt/math/math.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/event/helpers/SoundChecker.h>
#include <toki/game/event/Event.h>
#include <toki/game/event/Signal.h>
#include <toki/game/event/SoundGraphicsMgr.h>
#include <toki/game/DebugView.h>
#include <toki/game/Game.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/fwd.h>
#include <toki/level/helpers.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace event {


bool operator==(const Event& p_lhs, const Event& p_rhs)
{
	return p_lhs.getType()     == p_rhs.getType() && 
	       p_lhs.getPosition() == p_rhs.getPosition() &&
	       p_lhs.getSource()   == p_rhs.getSource();
}


bool operator!=(const Event& p_lhs, const Event& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


bool operator <(const Event& p_lhs, const Event& p_rhs)
{
	if (p_lhs.getType() != p_rhs.getType())
	{
		return p_lhs.getType() < p_rhs.getType();
	}
	
	if (p_lhs.getSource() != p_rhs.getSource())
	{
		return p_lhs.getSource() < p_rhs.getSource();
	}
	
	tt::math::Vector2 positionLhs(p_lhs.getPosition());
	tt::math::Vector2 positionRhs(p_rhs.getPosition());
	
	if (positionLhs.x != positionRhs.x)
	{
		return positionLhs.x < positionRhs.x;
	}
	
	return positionLhs.y < positionRhs.y;
}


//--------------------------------------------------------------------------------------------------
// Public member functions

Event::Event()
:
m_type(EventType_None),
m_radius(0.0f)
{
}


Event::Event(EventType p_type, const tt::math::Point2& p_tilePosition,
             real p_radius, entity::EntityHandle p_source)
:
m_type(p_type),
m_position(level::tileToWorld(p_tilePosition)),
m_source(p_source),
m_radius(p_radius)
{
}


Event::Event(EventType p_type, const tt::math::Vector2& p_worldPosition,
             real p_radius, entity::EntityHandle p_source)
:
m_type(p_type),
m_position(p_worldPosition),
m_source(p_source),
m_radius(p_radius)
{
}


SignalSet Event::process(helpers::SoundChecker& p_soundChecker) const
{
	using namespace entity;
	SignalSet signals;
	
	EntityHandleSet handles;
	switch (m_type)
	{
	case EventType_Sound:
		handles = processSound(p_soundChecker);
		break;
	
	case EventType_Vibration:
		handles = processVibration();
		break;
	
	default:
		TT_PANIC("Unhandled EventType '%d'", m_type);
		break;
	}
	
	if (m_callbackSource)
	{
		entity::Entity* source = m_source.getPtr();
		if (source != 0)
		{
			// This collection of raw pointers should be used right away.
			script::EntityBaseCollection collection;
			for (EntityHandleSet::const_iterator it = handles.begin(); it != handles.end(); ++it)
			{
				entity::Entity* otherEntity = (*it).getPtr();
				if (otherEntity != 0)
				{
					collection.push_back(otherEntity->getEntityScript().get());
				}
			}
			
			source->getEntityScript()->onEventSpawned((*this), m_callbackUserParam, collection);
		}
	}
	
	for (EntityHandleSet::const_iterator it = handles.begin(); it != handles.end(); ++it)
	{
		signals.insert(Signal(*it, this));
	}
	
	return signals;
}


//----------------------------------------------------------------------------------------------------------------
// Private member functions

entity::EntityHandleSet Event::processSound(helpers::SoundChecker& p_soundChecker) const
{
	Game* game = AppGlobal::getGame();
	
	// snap position to 0.5 tiles
	tt::math::Vector2 snappedPos(tt::math::round(m_position.x * 2.0f) / 2.0f,
	                             tt::math::round(m_position.y * 2.0f) / 2.0f);
	
	const helpers::SoundChecker::Locations& locations = p_soundChecker.fill(snappedPos, m_radius);
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	if (AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Event))
	{
		DebugView& debugView = game->getDebugView();
		DebugView::Point2s points;
		for (helpers::SoundChecker::Locations::const_iterator it = locations.begin();
		     it != locations.end(); ++it)
		{
			points.push_back((*it).location);
		}
		debugView.registerTiles(DebugView::TileInfo(points, 0, 0.25f));
	}
#endif
	
	const level::TileRegistrationMgr& tileMgr = game->getTileRegistrationMgr();
	entity::EntityHandleSet returnedHandles;
	
	// Martijn: not needed for RIVE
	/*
	SoundGraphicsMgr& soundGraphicsMgr = game->getSoundGraphicsMgr();
	soundGraphicsMgr.registerSound(m_radius, locations);
	// */
	
	for (helpers::SoundChecker::Locations::const_iterator it = locations.begin();
	     it != locations.end(); ++it)
	{
		tileMgr.findRegisteredEntityHandles((*it).location, returnedHandles);
	}
	
	return returnedHandles;
}


entity::EntityHandleSet Event::processVibration() const
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	if (AppGlobal::getDebugRenderMask().checkFlag(DebugRender_Event))
	{
		Game* game = AppGlobal::getGame();
		DebugView& debugView = game->getDebugView();
	
		using namespace tt::engine::renderer;
		debugView.registerCircle(
			DebugView::CircleInfo(m_position, m_radius, true, ColorRGBA(255, 0, 128, 100), 0.25f));
	}
#endif
	
	level::TileRegistrationMgr& mgr = AppGlobal::getGame()->getTileRegistrationMgr();
	entity::EntityMgr& entityMgr = AppGlobal::getGame()->getEntityMgr();
	
	using namespace tt::math;
	
	// first do a box test to determine entities nearby
	const VectorRect rect(Vector2(m_position.x - m_radius, m_position.y - m_radius),
	                      Vector2(m_position.x + m_radius, m_position.y + m_radius));
	const PointRect tileRect = level::worldToTile(rect);
	
	// now do an actual distance check with the found entities
	entity::EntityHandleSet unfilteredHandles;
	mgr.findRegisteredEntityHandles(tileRect, unfilteredHandles);
	entity::EntityHandleSet returnedHandles;
	
	const real radiusSqrt = m_radius * m_radius;
	
	for (entity::EntityHandleSet::const_iterator it = unfilteredHandles.begin();
	     it != unfilteredHandles.end(); ++it)
	{
		const entity::Entity* target = entityMgr.getEntity(*it);
		if (target != 0 && (*it) != m_source)
		{
			typedef entity::Entity::DetectionPoints Points;
			const Points& points(target->getVibrationDetectionPoints());
			const tt::math::Vector2 centerPos = target->getCenterPosition();
			for (Points::const_iterator ptIt = points.begin(); ptIt != points.end(); ++ptIt)
			{
				if (tt::math::distanceSquared(centerPos + target->applyOrientationToVector2(*ptIt),
				                              m_position) <= radiusSqrt)
				{
					returnedHandles.insert(*it);
					break;
				}
			}
		}
	}
	
	return returnedHandles;
}


// Namespace end
}
}
}
