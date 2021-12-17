#include <tt/code/bufferutils.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/fluid/FluidMgr.h>
#include <toki/game/movement/SurroundingsSurvey.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/Game.h>
#include <toki/level/AttributeLayer.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/level/skin/MaterialCache.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace movement {

//--------------------------------------------------------------------------------------------------
// Public member functions

SurroundingsSurvey::SurroundingsSurvey()
:
m_snappedPos(),
m_snappedTiles(),
m_surroundingTiles(),
m_collResultSnappedPos(),
m_collResultSnappedTwoTiles(),
m_insideAnyCollisions(),
m_inside(level::CollisionType_Invalid),
m_touchCollisions(),
m_fluidInside(fluid::FluidType_Invalid),
m_fluidTouchCollisions(),
m_waterfallInside(fluid::FluidType_Invalid),
m_waterfallTouchCollisions(),
m_checkResults(),
m_standOnTheme(level::skin::MaterialTheme_None),
m_standOnEntityTile(false)
{
}


SurroundingsSurvey::SurroundingsSurvey(const entity::Entity& p_entity)
:
m_snappedPos(p_entity.getSnappedToTilePos()),
m_snappedTiles(level::worldToTile(p_entity.applyOrientationToVectorRect(p_entity.getCollisionRect()).translate(m_snappedPos))),
m_surroundingTiles(m_snappedTiles.getMin() - tt::math::Point2::allOne, m_snappedTiles.getMaxInside() + tt::math::Point2::allOne),
m_surroundingTwoTiles(m_snappedTiles.getMin() - tt::math::Point2(2,2), m_snappedTiles.getMaxInside() + tt::math::Point2(2,2)),
// FIXME: This needs to check the snappedRect and rects moved one tile.
//        Not the one tile wide surrounding tiles we have now.
m_collResultSnappedPos(TileCollisionHelper::hasTileCollision(m_snappedTiles, m_surroundingTiles, p_entity)),
m_collResultSnappedTwoTiles(TileCollisionHelper::hasTileCollision(m_snappedTiles, m_surroundingTwoTiles, p_entity)),
m_insideAnyCollisions(),
m_inside(level::CollisionType_Invalid),
m_touchCollisions(),
m_fluidInside(fluid::FluidType_Invalid),
m_fluidTouchCollisions(),
m_waterfallInside(fluid::FluidType_Invalid),
m_waterfallTouchCollisions(),
m_checkResults(),
m_standOnTheme(level::skin::MaterialTheme_None),
m_standOnEntityTile(false)
{
	const tt::math::PointRect  registeredTileRect(p_entity.calcRegisteredTileRect());
	
	
	level::CollisionTypes oneUpResult;
	level::CollisionTypes oneDownResult;
	level::CollisionTypes oneLeftResult;
	level::CollisionTypes oneRightResult;
	
	level::CollisionTypes twoUpResult;
	level::CollisionTypes twoDownResult;
	level::CollisionTypes twoLeftResult;
	level::CollisionTypes twoRightResult;
	
	// NOTE: Do not call functions on AttributeLayer directly! Pass this to TileCollisionHelper and related helper code.
	const level::AttributeLayerPtr& layer = AppGlobal::getGame()->getAttributeLayer();
	const tt::math::Point2& min = m_snappedTiles.getMin();
	const tt::math::Point2  max = m_snappedTiles.getMaxInside();
	level::CollisionTypes insideAnyCollisionsRaw;
	{
		m_insideAnyCollisions  = TileCollisionHelper::getCollisionTypes(p_entity, layer, min, max, &m_inside);
		insideAnyCollisionsRaw = TileCollisionHelper::getCollisionTypes(p_entity, layer, min, max, 0        , true);
		m_touchCollisions     = TileCollisionHelper::getCollisionTypes(p_entity, layer,
		                                                               registeredTileRect.getMin(),
		                                                               registeredTileRect.getMaxInside());
		
		using tt::math::Point2;
		oneUpResult    = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x,     max.y    ), Point2(max.x,     max.y + 1));
		oneDownResult  = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x,     min.y - 1), Point2(max.x,     min.y    ));
		oneLeftResult  = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x - 1, min.y    ), Point2(min.x,     max.y    ));
		oneRightResult = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(max.x,     min.y    ), Point2(max.x + 1, max.y    ));

		twoUpResult    = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x,     max.y + 1), Point2(max.x,     max.y + 2));
		twoDownResult  = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x,     min.y - 2), Point2(max.x,     min.y - 1));
		twoLeftResult  = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(min.x - 2, min.y    ), Point2(min.x - 1, max.y    ));
		twoRightResult = TileCollisionHelper::getCollisionTypes(p_entity, layer, Point2(max.x + 1, min.y    ), Point2(max.x + 2, max.y    ));
	}
	
	// Check whether entity is aligned on any tile edges
	setEdgeResults(p_entity, m_checkResults);
	
	// Check directly around
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_Bottom, SurveyResult_Floor);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_Top,    SurveyResult_Ceiling);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_Right,  SurveyResult_WallRight);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_Left,   SurveyResult_WallLeft);
	
	// Check corners
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_BottomLeft,  SurveyResult_BottomLeft);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_BottomRight, SurveyResult_BottomRight);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_TopRight,    SurveyResult_TopRight);
	setResultFlag(m_collResultSnappedPos, TileCollisionHelper::CollisionResult_TopLeft,     SurveyResult_TopLeft);
	
	// Check two tiles from entity
	setResultFlag(m_collResultSnappedTwoTiles, TileCollisionHelper::CollisionResult_Bottom, SurveyResult_TwoDown);
	setResultFlag(m_collResultSnappedTwoTiles, TileCollisionHelper::CollisionResult_Top,    SurveyResult_TwoUp);
	setResultFlag(m_collResultSnappedTwoTiles, TileCollisionHelper::CollisionResult_Right,  SurveyResult_TwoRight);
	setResultFlag(m_collResultSnappedTwoTiles, TileCollisionHelper::CollisionResult_Left,   SurveyResult_TwoLeft);
	
	
	static const tt::math::Point2 upRight(1, 1);
	if (TileCollisionHelper::hasTileCollision(m_snappedTiles.getMin()       + upRight,
	                                          m_snappedTiles.getMaxInside() + upRight,
	                                          p_entity) == false)
	{
		m_checkResults.setFlag(SurveyResult_OneUpRight);
	}
	
	static const tt::math::Point2 upLeft(-1, 1);
	if (TileCollisionHelper::hasTileCollision(m_snappedTiles.getMin()       + upLeft,
	                                          m_snappedTiles.getMaxInside() + upLeft,
	                                          p_entity) == false)
	{
		m_checkResults.setFlag(SurveyResult_OneUpLeft);
	}
	
	static const tt::math::Point2 downRight(1, -1);
	if (TileCollisionHelper::hasTileCollision(m_snappedTiles.getMin()       + downRight,
	                                          m_snappedTiles.getMaxInside() + downRight,
	                                          p_entity) == false)
	{
		m_checkResults.setFlag(SurveyResult_OneDownRight);
	}
	
	static const tt::math::Point2 downLeft(-1, -1);
	if (TileCollisionHelper::hasTileCollision(m_snappedTiles.getMin()       + downLeft,
	                                          m_snappedTiles.getMaxInside() + downLeft,
	                                          p_entity) == false)
	{
		m_checkResults.setFlag(SurveyResult_OneDownLeft);
	}
	
	// The following algorithm depends on this specific order in the enum.
	// (Each step is a counter-clockwise rotation and +1 in value.)
	TT_STATIC_ASSERT(movement::Direction_Down  == 0); // 'Normal' orientation.
	TT_STATIC_ASSERT(movement::Direction_Right == 1); // 1 rotation.
	TT_STATIC_ASSERT(movement::Direction_Up    == 2); // 2 rotations.
	TT_STATIC_ASSERT(movement::Direction_Left  == 3); // 3 rotations.
	TT_STATIC_ASSERT(movement::Direction_None  == 4); // This value is used to wrap our rotation around again.
	TT_STATIC_ASSERT(movement::Direction_Count == 5); // This assert is to make sure there are no more values.
	
	// Check SurveyResult_DropRight for all orientations.
	TT_STATIC_ASSERT(SurveyResult_DropRight + movement::Direction_Down  == SurveyResult_DropRight);
	TT_STATIC_ASSERT(SurveyResult_DropRight + movement::Direction_Right == SurveyResult_DropRight_fromRightAsDown);
	TT_STATIC_ASSERT(SurveyResult_DropRight + movement::Direction_Up    == SurveyResult_DropRight_fromUpAsDown);
	TT_STATIC_ASSERT(SurveyResult_DropRight + movement::Direction_Left  == SurveyResult_DropRight_fromLeftAsDown);
	
	for (s32 i = 0; i < movement::Direction_None; ++i)
	{
		Direction dir = static_cast<movement::Direction>(i);
		
		const tt::math::Point2 widthRightOneDown(m_snappedTiles.getWidth(), -1);
		const tt::math::Point2 dropRightMin(m_snappedTiles.getMin()       + entity::applyOrientationToPoint2(widthRightOneDown, dir));
		const tt::math::Point2 dropRightMax(m_snappedTiles.getMaxInside() + entity::applyOrientationToPoint2(widthRightOneDown, dir));
		if (TileCollisionHelper::hasTileCollision(dropRightMin, dropRightMax, p_entity) == false)
		{
			m_checkResults.setFlag(static_cast<SurveyResult>(SurveyResult_DropRight + i));
		}
	}
	
	// Check SurveyResult_DropLeft for all orientations.
	TT_STATIC_ASSERT(SurveyResult_DropLeft + movement::Direction_Down  == SurveyResult_DropLeft);
	TT_STATIC_ASSERT(SurveyResult_DropLeft + movement::Direction_Right == SurveyResult_DropLeft_fromRightAsDown);
	TT_STATIC_ASSERT(SurveyResult_DropLeft + movement::Direction_Up    == SurveyResult_DropLeft_fromUpAsDown);
	TT_STATIC_ASSERT(SurveyResult_DropLeft + movement::Direction_Left  == SurveyResult_DropLeft_fromLeftAsDown);
	
	for (s32 i = 0; i < movement::Direction_None; ++i)
	{
		Direction dir = static_cast<movement::Direction>(i);
		
		const tt::math::Point2 widthLeftOneDown(-m_snappedTiles.getWidth(), -1);
		const tt::math::Point2 dropLeftMin(m_snappedTiles.getMin()       + entity::applyOrientationToPoint2(widthLeftOneDown, dir));
		const tt::math::Point2 dropLeftMax(m_snappedTiles.getMaxInside() + entity::applyOrientationToPoint2(widthLeftOneDown, dir));
		
		if (TileCollisionHelper::hasTileCollision(dropLeftMin, dropLeftMax, p_entity) == false)
		{
			m_checkResults.setFlag(static_cast<SurveyResult>(SurveyResult_DropLeft + i));
		}
	}
	
	const level::CollisionTypes solidMask = level::g_collisionTypesSolid;
	
	if (m_insideAnyCollisions.checkAnyFlags(solidMask))
	{
		m_checkResults.setFlag(SurveyResult_InsideCollision);
	}
	
	if (insideAnyCollisionsRaw.checkAnyFlags(solidMask))
	{
		m_checkResults.setFlag(SurveyResult_InsideCollisionRaw);
	}
	
	const entity::movementcontroller::DirectionalMovementController* controller = 
			p_entity.getDirectionalMovementController();
	const movement::Direction downDir = (p_entity.getOrientationDown() != movement::Direction_None) ?
	                                     p_entity.getOrientationDown() :  movement::Direction_Down;
	if (controller != 0)
	{
		if (controller->getTouchingCollisionDirections(p_entity).checkFlag(downDir))
		{
			m_checkResults.setFlag(SurveyResult_StandOnSolid);
		}
		if (controller->getCollisionParentEntity().isEmpty() == false)
		{
			m_checkResults.setFlag(SurveyResult_OnParent);
			
			const entity::Entity* parentEntity = controller->getCollisionParentEntity().getPtr();
			if (parentEntity != 0)
			{
				const entity::movementcontroller::DirectionalMovementController* parentController =
						parentEntity->getDirectionalMovementController();
				if (parentController != 0)
				{
					const Direction parentWorldDir = parentController->getActualMovementDirection();
					if (isValidDirection(parentWorldDir))
					{
						const entity::LocalDir checkDir = p_entity.getLocalDirFromDirection(parentWorldDir);
						if (checkDir == entity::LocalDir_Up ||
						    checkDir == entity::LocalDir_Down)
						{
							m_checkResults.setFlag(SurveyResult_OnParentVertical);
						}
						/* TODO: We could add this for a possible "horizontal parent" flag (if we need one):
						else if (checkDir == entity::LocalDir_Forward ||
						         checkDir == entity::LocalDir_Back)
						{
							m_checkResults.setFlag(SurveyResult_OnParentHorizontal);
						}
						// */
					}
				}
			}
		}
	}
	
	AppGlobal::getGame()->getFluidMgr().fillSurvey(*this, registeredTileRect, p_entity);
	
	m_standOnTheme = level::skin::MaterialTheme_None; // Reset to none, maybe override later.
	// Find the first valid result and use that.
	for (tt::math::Point2 standOnTile(min.x, min.y - 1); standOnTile.x <= max.x; ++standOnTile.x)
	{
		if (layer->contains(standOnTile))
		{
			const level::skin::TileMaterial& tileMat = AppGlobal::getGame()->getTileMaterial(standOnTile);
			if (tileMat.getMaterialType() == level::skin::MaterialType_Solid)
			{
				m_standOnTheme = tileMat.getMaterialTheme();
				break;
			}
		}
	}
	m_standOnEntityTile = false;
	for (tt::math::Point2 standOnTile(min.x, min.y - 1); standOnTile.x <= max.x; ++standOnTile.x)
	{
		if (AppGlobal::getGame()->getTileRegistrationMgr().hasSolidEntityTilesAtPosition(standOnTile))
		{
			m_standOnEntityTile = true;
			break;
		}
	}
}


void SurroundingsSurvey::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_snappedPos                        , p_context); // tt::math::Vector2                        
	bu::put(m_snappedTiles                      , p_context); // tt::math::PointRect                      
	bu::put(m_surroundingTiles                  , p_context); // tt::math::PointRect                      
	bu::put(m_surroundingTwoTiles               , p_context); // tt::math::PointRect                      
	bu::putBitMask(  m_collResultSnappedPos     , p_context); // TileCollisionHelper::CollisionResultMask 
	bu::putBitMask(  m_collResultSnappedTwoTiles, p_context); // TileCollisionHelper::CollisionResultMask 
	bu::putBitMask(  m_insideAnyCollisions      , p_context); // level::CollisionTypes                    
	bu::putEnum<u32>(m_inside                   , p_context); // level::CollisionType                     
	bu::putBitMask(  m_touchCollisions          , p_context); // level::CollisionTypes                    
	bu::putEnum<u32>(m_fluidInside              , p_context); // fluid::FluidType                         
	bu::putBitMask(  m_fluidTouchCollisions     , p_context); // fluid::FluidTypes                        
	bu::putEnum<u32>(m_waterfallInside          , p_context); // fluid::FluidType                         
	bu::putBitMask(  m_waterfallTouchCollisions , p_context); // fluid::FluidTypes                        
	bu::putBitMask(  m_checkResults             , p_context); // SurveyResults
	bu::putEnum<u8>( m_standOnTheme             , p_context); // level::skin::MaterialTheme
	bu::put(         m_standOnEntityTile        , p_context); // bool
}


void SurroundingsSurvey::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Serialize members
	namespace bu = tt::code::bufferutils;
	
	m_snappedPos                = bu::get<tt::math::Vector2                       >(p_context);
	m_snappedTiles              = bu::get<tt::math::PointRect                     >(p_context);
	m_surroundingTiles          = bu::get<tt::math::PointRect                     >(p_context);
	m_surroundingTwoTiles       = bu::get<tt::math::PointRect                     >(p_context);
	m_collResultSnappedPos      = bu::getBitMask<TileCollisionHelper::CollisionResultMask::Flag,
	                                             TileCollisionHelper::CollisionResultMask::FlagCount>(p_context);
	m_collResultSnappedTwoTiles = bu::getBitMask<TileCollisionHelper::CollisionResultMask::Flag,
	                                             TileCollisionHelper::CollisionResultMask::FlagCount>(p_context);
	m_insideAnyCollisions       = bu::getBitMask<level::CollisionTypes::Flag,
	                                             level::CollisionTypes::FlagCount>(p_context);
	m_inside                    = bu::getEnum<u32, level::CollisionType>(p_context);
	m_touchCollisions           = bu::getBitMask<level::CollisionTypes::Flag,
	                                             level::CollisionTypes::FlagCount>(p_context);
	m_fluidInside               = bu::getEnum<u32, fluid::FluidType>(p_context);
	m_fluidTouchCollisions      = bu::getBitMask<fluid::FluidTypes::Flag,
	                                             fluid::FluidTypes::FlagCount>(p_context);
	m_waterfallInside           = bu::getEnum<u32, fluid::FluidType>(p_context);
	m_waterfallTouchCollisions  = bu::getBitMask<fluid::FluidTypes::Flag,
	                                             fluid::FluidTypes::FlagCount>(p_context);
	m_checkResults              = bu::getBitMask<SurveyResults::Flag,
	                                             SurveyResults::FlagCount>(p_context);
	m_standOnTheme              = bu::getEnum<u8, level::skin::MaterialTheme>(p_context);
	m_standOnEntityTile         = bu::get<bool>(p_context);
}


void SurroundingsSurvey::setEdgeResults(const entity::Entity&    p_entity,
                                        SurveyResults&           p_checkResults_OUT)
{
	// Fillin level tile edges.
	setEdgeResults(p_entity.getPosition(), p_entity.getSnappedToTilePosLevelOnly(), p_checkResults_OUT);
	
	// Add the parent edges on top of previous results. (Kind of a HACK, but fixes falling through level collision when on a falling parent.)
	entity::Entity* collisionParent = p_entity.getCollisionParentEntity().getPtr();
	if (collisionParent != 0)
	{
		setEdgeResults(p_entity.getPosition(), p_entity.getSnappedToTilePos(), p_checkResults_OUT);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


void SurroundingsSurvey::setEdgeResults(const tt::math::Vector2& p_pos,
                                        const tt::math::Vector2& p_snappedPos,
                                        SurveyResults& p_checkResults_OUT)
{
	const tt::math::Vector2  posDiff(p_pos - p_snappedPos);
	
	static const real edgeEpsilon = 0.05f;  // wiggle room for "on edge"
	
	if (tt::math::realEqual(p_pos.y, p_snappedPos.y))
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnTopEdge);
		p_checkResults_OUT.setFlag(SurveyResult_OnBottomEdge);
	}
	else if (posDiff.y > 0.0f && posDiff.y < edgeEpsilon)
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnTopEdge);
	}
	else if (posDiff.y < 0.0f && posDiff.y > -edgeEpsilon)
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnBottomEdge);
	}
	
	if (tt::math::realEqual(p_pos.x, p_snappedPos.x))
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnRightEdge);
		p_checkResults_OUT.setFlag(SurveyResult_OnLeftEdge);
	}
	else if (posDiff.x > 0.0f && posDiff.x < edgeEpsilon)
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnRightEdge);
	}
	else if (posDiff.x < 0.0f && posDiff.x > -edgeEpsilon)
	{
		p_checkResults_OUT.setFlag(SurveyResult_OnLeftEdge);
	}
}


// Namespace end
}
}
}
