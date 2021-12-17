#if !defined(INC_TOKI_GAME_FLUID_WAVEGENERATOR_H)
#define INC_TOKI_GAME_FLUID_WAVEGENERATOR_H


#include <map>
#include <vector>

#include <tt/math/fwd.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fluid/graphics_helpers.h>


namespace toki {
namespace game {
namespace fluid {

/*! \brief Manages the generation of waves on the fluid surfaces. */
class WaveGenerator
{
public:
	enum EdgeOption
	{
		EdgeOption_Default,     // use this for the default
		EdgeOption_LeftEdge,    // use this to specify a simulation edge where waves should hit and return
		EdgeOption_RightEdge,
		EdgeOption_LeftFadeout, // use this to specify a simulation edge where waves should fade out
		EdgeOption_RightFadeout
	};
	
	enum PoolSurfaceType
	{
		PoolSurfaceType_Left,
		PoolSurfaceType_Still,
		PoolSurfaceType_Right,
		
		PoolSurfaceType_Count,
		PoolSurfaceType_Invalid
	};

	WaveGenerator();
	
	void update(real p_delta);
	void updateForRender();
	
	void startWave(const tt::math::Point2& p_position, real p_tilePosition, real p_width,
	               real p_strength, real p_duration);
	
	void setHeight(real p_height, const tt::math::Point2& p_position, s32 p_waveQuadIndex);
	real getHeight(               const tt::math::Point2& p_position, s32 p_waveQuadIndex)const;
	
	void addSimulationPosition(const tt::math::Point2& p_position, EdgeOption p_edgeOption,
		PoolSurfaceType p_poolType, FluidType p_fluidType);
	
	void handleLevelResized();
	
	inline s32 getWaveQuadsPerTile() const { return 2;}
	
private:
	enum
	{
		ExtraSimulationHeights = 1 // Amount of Padding infront and behind the simulation points
	};
	
	typedef std::vector<real> Reals;
	struct SimulationData
	{
		SimulationData()
		:
		prevDensity(),
		currDensity(),
		rightEdge(EdgeOption_Default),
		leftEdge(EdgeOption_Default),
		leftToStillIndex(0),
		stillToRightIndex(0),
		isDead(true),
		simulationSize(0),
		fluidType(FluidType_Invalid)
		{ }
		
		Reals prevDensity;
		Reals currDensity;
		EdgeOption rightEdge;
		EdgeOption leftEdge;
		
		Reals::size_type leftToStillIndex;
		Reals::size_type stillToRightIndex;
		
		bool             isDead;
		Reals::size_type simulationSize;
		
		FluidType fluidType;
	};
	
	struct Point2LessYPriority
	{
		// Types expected by standard library functions / containers
		typedef tt::math::Point2 first_argument_type;
		typedef tt::math::Point2 second_argument_type;
		typedef bool   result_type;
		
		inline bool operator()(const tt::math::Point2& p_a,
		                       const tt::math::Point2& p_b) const
		{
			if (p_a.y != p_b.y)
			{
				return p_a.y < p_b.y;
			}
			return p_a.x < p_b.x;
		}
	};
	typedef std::map<tt::math::Point2, SimulationData, Point2LessYPriority> SurfaceHeights;
	
	struct PersistentWaveSettings
	{
		PersistentWaveSettings(real p_moveSpeed, real p_waveWidth, real p_waveHeight)
		:
		moveSpeed(p_moveSpeed),
		waveWidth(p_waveWidth),
		waveHeight(p_waveHeight)
		{}
		
		real moveSpeed;
		real waveWidth;
		real waveHeight;
	};
	typedef std::vector<PersistentWaveSettings> PersistentWaves;
	
	struct DissipatingWave
	{
		DissipatingWave(real p_strength, real p_duration, const tt::math::Point2& p_startPosition,
		                s32 p_startIndex, s32 p_endIndex)
		:
		strength(p_strength),
		duration(p_duration),
		startPosition(p_startPosition),
		timeLeft(p_duration),
		startIndex(p_startIndex),
		endIndex(p_endIndex)
		{}
		
		real             strength;
		real             duration;
		tt::math::Point2 startPosition;
		real             timeLeft;
		u32              startIndex;
		u32              endIndex;
	};
	typedef std::vector<DissipatingWave> DissipatingWaves;
	
	const tt::math::Point2& getStartPos(const tt::math::Point2& p_pos) const;
	u32 getSimulationIndex(const tt::math::Point2& p_position, s32 p_waveQuadIndex,
	                       const tt::math::Point2& p_startPosition) const;
	u32 convertQuadIndexToSimulationIndex(u32 p_waveQuadIndex) const;
	
	
	static const tt::math::Point2 ms_invalidPos;
	
	SurfaceHeights   m_surfaceHeights;
	tt::math::Point2 m_startPos;
	tt::math::Point2 m_prevPos;
	real             m_accumulator;
	real             m_angle;
	
	u32              m_simulationPointsPerTile;
	PersistentWaves  m_waveSettings[FluidType_Count][PoolSurfaceType_Count];
	DissipatingWaves m_dissipatingWaves;
	
	real m_velocitydiffusion[FluidType_Count];
	real m_diffusion        [FluidType_Count];
	
	const real m_maxHeight;
	const real m_minHeight;
	
	// No copying
	WaveGenerator(const WaveGenerator&);
	WaveGenerator& operator=(const WaveGenerator&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_WAVEGENERATOR_H)
