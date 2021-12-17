#if !defined(INC_TOKI_GAME_FLUID_FLUIDPARTICLESMGR_H)
#define INC_TOKI_GAME_FLUID_FLUIDPARTICLESMGR_H


#include <map>
#include <set>

#include <tt/engine/particles/fwd.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>

#include <toki/audio/SoundCueWithPosition.h>
#include <toki/game/entity/fwd.h>
#include <toki/game/fluid/FluidSettings.h>
#include <toki/game/fluid/types.h>


namespace toki {
namespace game {
namespace fluid {

/*! \brief Manages Fluid Particles in a level. */
class FluidParticlesMgr
{
public:
	enum TileOrientation
	{
		TileOrientation_TopLeft,
		TileOrientation_TopCenter,
		TileOrientation_TopRight,
		TileOrientation_CenterLeft,
		TileOrientation_CenterCenter,
		TileOrientation_CenterRight,
		TileOrientation_BottomLeft,
		TileOrientation_BottomCenter,
		TileOrientation_BottomRight,
		
		TileOrientation_Count
	};
	static inline bool isTileOrientationValid(TileOrientation p_val) { return p_val >= 0 && p_val < TileOrientation_Count; }
	
	enum ParticleType
	{
		ParticleType_NewTile,
		ParticleType_RemovedTile,
		ParticleType_SingleFall,
		ParticleType_DoubleFall,
		ParticleType_DoubleFall_Center,
		ParticleType_WaterLavaCollision,

		ParticleType_ExpandFall,
		ParticleType_ExpandFlowLeft,
		ParticleType_ExpandFlowRight,

		ParticleType_Count
	};

	inline static bool isBurstParticleType(ParticleType p_type)
	{
		return p_type == ParticleType_NewTile ||
		       p_type == ParticleType_RemovedTile ||
		       p_type == ParticleType_WaterLavaCollision; 
	}
	
	
	FluidParticlesMgr();
	
	void update();
	void triggerParticle(const tt::math::Point2& p_tilePos,
	                     TileOrientation         p_tileOrientation,
	                     ParticleType            p_particleType,
	                     FluidType               p_fluidType,
	                     bool                    p_flipX = false,
	                     bool                    p_flipY = false);

	void updateExpansion(const FluidShape* p_shape, ParticleType p_type);

	void setDirty();
	
	inline void reset()
	{
		m_particles.clear();
		m_retriggeredParticles.clear();
		clearParticleCache();
		m_fallExpandParticles.clear();
		m_flowExpandLeftParticles.clear();
		m_flowExpandRightParticles.clear();
		m_dirty = true;
	}
	
	void clearParticleCache();
	
private:
	enum HorizontalTileOrientation
	{
		HorizontalTileOrientation_Left,
		HorizontalTileOrientation_Center
		// NOTE: Right is Left on the tile to the right
	};
	
	enum VerticalTileOrientation
	{
		VerticalTileOrientation_Top,
		VerticalTileOrientation_Center,
		// NOTE: Bottom is Top on the tile below
	};
	
	struct ParticlePosition
	{
		inline ParticlePosition(const tt::math::Vector2&  p_worldPosition,
		                        const tt::math::Point2&   p_tilePosition,
		                        HorizontalTileOrientation p_horizontalOrientation,
		                        VerticalTileOrientation   p_verticalOrientation,
		                        ParticleType              p_particleType,
		                        FluidType                 p_fluidType)
		:
		worldPosition        (p_worldPosition),
		tilePosition         (p_tilePosition),
		horizontalOrientation(p_horizontalOrientation),
		verticalOrientation  (p_verticalOrientation),
		particleType         (p_particleType),
		fluidType            (p_fluidType)
		{ }
		
		tt::math::Vector2         worldPosition;
		tt::math::Point2          tilePosition;
		HorizontalTileOrientation horizontalOrientation;
		VerticalTileOrientation   verticalOrientation;
		ParticleType              particleType;
		FluidType                 fluidType;
	};
	
	struct ParticlePositionLess
	{
		// Types expected by standard library functions / containers
		typedef ParticlePosition first_argument_type;
		typedef ParticlePosition second_argument_type;
		typedef bool             result_type;
		
		bool operator()(const ParticlePosition& p_a,
		                const ParticlePosition& p_b) const;
	};
	
	typedef std::vector<tt::math::Point2> Point2s;
	typedef std::map<ParticlePosition, tt::engine::particles::ParticleEffectPtr, ParticlePositionLess> TileParticles;
	typedef std::set<ParticlePosition, ParticlePositionLess> ParticlePositions;

	struct FluidExpandEffect
	{
		tt::engine::particles::ParticleEffectPtr effect;
		audio::SoundCueWithPositionPtr           soundCueWithPos;
		u32                                      killCounter;
	};
	typedef std::map<const FluidShape*, FluidExpandEffect> FluidExpandParticles;
	
	inline bool isSurfaceFlowType(FluidFlowType p_flowType) const
	{
		return p_flowType == FluidFlowType_Left ||
		       p_flowType == FluidFlowType_LeftLvl2 ||
		       p_flowType == FluidFlowType_Right ||
		       p_flowType == FluidFlowType_RightLvl2 ||
		       p_flowType == FluidFlowType_Still ||
		       p_flowType == FluidFlowType_StillUnderFall;
	}
	inline bool isAboveSurfaceFlowType(FluidFlowType p_flowType) const
	{
		return p_flowType == FluidFlowType_None ||
		       p_flowType == FluidFlowType_LeftOverFlow ||
		       p_flowType == FluidFlowType_RightOverFlow ||
		       p_flowType == FluidFlowType_Fall;
	}
	
	
	static ParticlePosition convertToParticlePosition(const tt::math::Point2& p_tilePos,
	                                                  TileOrientation         p_tileOrientation,
	                                                  ParticleType            p_particleType,
	                                                  FluidType               p_fluidType);
	
	void updateExpandEffect(FluidExpandParticles& p_particles, const FluidShape* p_shape, ParticleType p_type);
	void checkExpandEffect(FluidExpandParticles& p_particles);
	
	TileParticles       m_particles;
	FluidExpandParticles m_fallExpandParticles;
	FluidExpandParticles m_flowExpandLeftParticles;
	FluidExpandParticles m_flowExpandRightParticles;
	ParticlePositions   m_retriggeredParticles;
	bool                m_dirty;
	
	// No copying
	FluidParticlesMgr(const FluidParticlesMgr&);
	FluidParticlesMgr& operator=(const FluidParticlesMgr&);
	
	tt::engine::particles::ParticleEffectPtr getParticleEffect(FluidType p_fluidType, ParticleType p_particleType);
	toki::audio::SoundCueWithPositionPtr     getSoundCueWithPosition(FluidType p_fluidType, ParticleType p_particleType,
	                                                                 const tt::math::Vector3& p_pos);
	
	tt::engine::particles::ParticleEffectPtr m_cache[FluidType_Count][ParticleType_Count];
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_FLUIDPARTICLESMGR_H)
