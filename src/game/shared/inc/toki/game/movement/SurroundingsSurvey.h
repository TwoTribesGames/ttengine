#if !defined(INC_TOKI_GAME_MOVEMENT_SURROUNDINGSSURVEY_H)
#define INC_TOKI_GAME_MOVEMENT_SURROUNDINGSSURVEY_H

#include <tt/code/fwd.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/game/movement/TileCollisionHelper.h>
#include <toki/game/movement/Validator.h>
#include <toki/level/fwd.h>
#include <toki/level/types.h>
#include <toki/level/skin/types.h>


namespace toki {
namespace game {
namespace movement {

class SurroundingsSurvey
{
public:
	SurroundingsSurvey();
	explicit SurroundingsSurvey(const entity::Entity& p_entity);
	
	inline const SurveyResults&         getCheckMask()            const { return m_checkResults;     }
	inline const tt::math::Vector2&     getSnappedPos()           const { return m_snappedPos;       }
	inline const tt::math::PointRect&   getSnappedTiles()         const { return m_snappedTiles;     }
	inline const level::CollisionTypes& getInsideAnyCollisionTypes()        const { return m_insideAnyCollisions; }
	inline       level::CollisionType   getInsideCollisionTypeForAllTiles() const { return m_inside;              }
	inline const level::CollisionTypes& getTouchCollisionTypes()            const { return m_touchCollisions;     }
	
	inline       fluid::FluidType   getInsideFluidTypeForAllTiles() const { return m_fluidInside;          }
	inline const fluid::FluidTypes& getTouchFluidTypes()            const { return m_fluidTouchCollisions; }
	
	inline void setInsideFluidTypeForAllTiles(fluid::FluidType p_type) { m_fluidInside          = p_type;  }
	inline void setTouchFluidTypes(const fluid::FluidTypes& p_types)   { m_fluidTouchCollisions = p_types; }
	
	inline       fluid::FluidType   getInsideWaterfallTypeForAllTiles() const { return m_waterfallInside;          }
	inline const fluid::FluidTypes& getTouchWaterfallTypes()            const { return m_waterfallTouchCollisions; }
	
	inline void setInsideWaterfallTypeForAllTiles(fluid::FluidType p_type) { m_waterfallInside          = p_type;  }
	inline void setTouchWaterfallTypes(const fluid::FluidTypes& p_types)   { m_waterfallTouchCollisions = p_types; }
	
	inline void setOnWaterResult(SurveyResult p_result)
	{
		// Make sure the input is a valid onWater result.
		TT_ASSERT(p_result == SurveyResult_OnWaterLeft   ||
		          p_result == SurveyResult_OnWaterStatic ||
		          p_result == SurveyResult_OnWaterRight  ||
		          p_result == SurveyResult_NotInWater    ||
		          p_result == SurveyResult_SubmergedInWater);
		// Make sure no onWater result was set.
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnWaterLeft)      == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnWaterStatic)    == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnWaterRight)     == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_NotInWater)       == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_SubmergedInWater) == false);
		
		// Set onWater result
		m_checkResults.setFlag(p_result);
	}
	
	inline void setOnLavaResult(SurveyResult p_result)
	{
		// Make sure the input is a valid onLava result.
		TT_ASSERT(p_result == SurveyResult_OnLavaLeft   ||
		          p_result == SurveyResult_OnLavaStatic ||
		          p_result == SurveyResult_OnLavaRight  ||
		          p_result == SurveyResult_NotInLava    ||
		          p_result == SurveyResult_SubmergedInLava);
		// Make sure no onWater result was set.
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnLavaLeft)      == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnLavaStatic)    == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_OnLavaRight)     == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_NotInLava)       == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_SubmergedInLava) == false);
		
		// Set onWater result
		m_checkResults.setFlag(p_result);
	}
	
	inline void setWaterDirection(SurveyResult p_result)
	{
		// Make sure no onWater result was set.
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_WaterFlowLeft  ) == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_WaterFlowRight ) == false);
		TT_ASSERT(m_checkResults.checkFlag(SurveyResult_WaterFlowStatic) == false);
		
		if (p_result == SurveyResult_Invalid)
		{
			return;
		}
		
		// Make sure only a waterflow result was passed.
		TT_ASSERT(p_result == SurveyResult_WaterFlowLeft  ||
		          p_result == SurveyResult_WaterFlowRight ||
		          p_result == SurveyResult_WaterFlowStatic);
		m_checkResults.setFlag(p_result);
	}
	
	inline level::skin::MaterialTheme getStandOnTheme()      const { return m_standOnTheme;      }
	inline bool                       getStandOnEntityTile() const { return m_standOnEntityTile; }
	
	void serialize(  tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void setEdgeResults(const entity::Entity&    p_entity,
	                           SurveyResults&           p_checkResults_OUT);
	
private:
	static void setEdgeResults(const tt::math::Vector2& p_pos,
	                           const tt::math::Vector2& p_snappedPos,
	                           SurveyResults& p_checkResults_OUT);
	
	inline void setResultFlag(TileCollisionHelper::CollisionResultMask p_mask,
	                          TileCollisionHelper::CollisionResult p_collisionResult,
	                          SurveyResult p_surveyResult)
	{
		if(p_mask.checkFlag(p_collisionResult)) m_checkResults.setFlag(p_surveyResult);
	}
	
	inline void setResultFlag(level::CollisionTypes p_collisionResult,
	                          level::CollisionTypes p_collisionMask,
	                          SurveyResult p_surveyResult)
	{
		if(p_collisionResult.checkAnyFlags(p_collisionMask) &&
			p_collisionResult.noOtherFlags(p_collisionMask))
			{
				m_checkResults.setFlag(p_surveyResult);
			}
	}
	
	// Keep order of members! (Needed for initializer list)
	tt::math::Vector2                        m_snappedPos;
	tt::math::PointRect                      m_snappedTiles;
	tt::math::PointRect                      m_surroundingTiles;
	tt::math::PointRect                      m_surroundingTwoTiles;
	
	// FIXME: Merge the calculation of the following two. (m_collResultSnappedPos and m_*Collisions).
	TileCollisionHelper::CollisionResultMask m_collResultSnappedPos;
	TileCollisionHelper::CollisionResultMask m_collResultSnappedTwoTiles;
	level::CollisionTypes                    m_insideAnyCollisions; // All the tiles inside the entity.
	level::CollisionType                     m_inside;              // Type which fills the entity completely.
	level::CollisionTypes                    m_touchCollisions;     // All the tiles the entity is touching.
	
	// Checks for fluids
	fluid::FluidType                         m_fluidInside;              // For fluids: Type which fills the entity completely.
	fluid::FluidTypes                        m_fluidTouchCollisions;     // For fluids: All the tiles the entity is touching.
	fluid::FluidType                         m_waterfallInside;          // For waterfalls: Type which fills the entity completely.
	fluid::FluidTypes                        m_waterfallTouchCollisions; // For waterfalls: All the tiles the entity is touching.
	
	SurveyResults                            m_checkResults;
	
	level::skin::MaterialTheme               m_standOnTheme;
	bool                                     m_standOnEntityTile;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_MOVEMENT_SURROUNDINGSSURVEY_H)
