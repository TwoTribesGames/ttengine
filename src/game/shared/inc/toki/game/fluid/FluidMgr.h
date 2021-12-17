#if !defined(INC_TOKI_GAME_FLUID_FLUIDMGR_H)
#define INC_TOKI_GAME_FLUID_FLUIDMGR_H


#include <map>
#include <vector>

#include <tt/audio/player/SoundCue.h>
#include <tt/cfg/Handle.h>
#include <tt/math/Point2.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fluid/FluidGraphicsMgr.h>
#include <toki/game/fluid/FluidParticlesMgr.h>
#include <toki/game/fluid/FluidSettings.h>
#include <toki/game/fluid/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/level/fwd.h>
#include <toki/level/TileChangedObserver.h>
#include <toki/level/TileRegistrationMgr.h>
#include <toki/serialization/fwd.h>
#include <toki/utils/SectionProfiler.h>


namespace toki {
namespace game {
namespace fluid {


/*! \brief Manages Fluids in a level. */
class FluidMgr : public level::TileChangedObserver
{
public:
	static FluidMgrPtr create(const level::AttributeLayerPtr& p_levelLayer);
	
	virtual ~FluidMgr() { }
	
	void update(real p_deltaTime);
	void updateForRender(const tt::math::VectorRect& p_visibilityRect);
	void render() const;
	inline void renderBack()            { m_graphicsMgr.renderBack();  }
	inline void renderFront()           { m_graphicsMgr.renderFront(); }
	inline void renderBackStillWater()  { m_graphicsMgr.renderBackStillWater(); }
	inline void renderFrontStillWater() { m_graphicsMgr.renderFrontStillWater(); }
	inline void renderLavaGlow()        { m_graphicsMgr.renderLavaGlow(); }
	
	virtual void onTileChange(const tt::math::Point2& p_position);
	void handleLevelResized();
	
	void setFluidLayerVisible(bool p_visible);
	
	void fillSurvey(movement::SurroundingsSurvey& p_survey,
	                const tt::math::PointRect&    p_entityRegisteredTileRect,
	                const entity::Entity&         p_entity) const;
	
	void updateEntityEffects();
	
	void onEntityEntersFluid(const entity::Entity& p_entity, FluidType p_type);
	void onEntityExitsFluid( const entity::Entity& p_entity, FluidType p_type);
	void onEntityEntersFall( const entity::Entity& p_entity, FluidType p_type);
	void onEntityExitsFall(  const entity::Entity& p_entity, FluidType p_type);
	void scheduleEntityEffectsRectsUpdate(const entity::Entity& p_entity,
	                                     EntityFluidEffectType p_entityFluidEffectType,
	                                     FluidType             p_fluidType);
	
	inline void startWave(const tt::math::Point2& p_position, real p_tilePosition,
	                      real p_width, real p_strength, real p_duration)
	{
		m_graphicsMgr.getWaveGenerator().startWave(p_position, p_tilePosition, p_width, p_strength, p_duration);
	}
	
	void addFluidWarp(const tt::math::VectorRect& p_rect, const std::string& p_name);
	void deleteFluidWarpPair(const std::string& p_name);
	
	void resetLevel();
	inline void resetSimulation(bool p_forcePregeneration) { resetFluidSimulation(p_forcePregeneration); }
	
	tt::math::Point2 getWarpedPosition(const tt::math::Point2& p_startPos);
	bool             isSameWarp(       const tt::math::Point2& p_pos1, const tt::math::Point2& p_pos2);
	
	void notifyTileChange(const tt::math::Point2& p_position);
	
	// DEBUG: Allow toggling the fluid graphics rendering
	inline void toggleGraphicsEnabled() { m_graphicsMgr.setEnabled(m_graphicsMgr.isEnabled() == false); }
	inline void toggleDebugRenderEnabled() { m_graphicsMgr.setDebugRenderEnabled(m_graphicsMgr.isDebugRenderEnabled() == false); }
	
	inline s32 renderProfiler(s32 p_x, s32 p_y)
	{
		return m_sectionProfiler.render(p_x, p_y, false);
	}
	
	inline const level::AttributeLayerPtr& getLayer() { return m_activeLayer; }
	
	void getFluidTypes(const tt::math::Point2& p_minPos,
	                   const tt::math::Point2& p_maxPos,
	                   FluidTypes&             p_notFalling_OUT,
	                   FluidType&              p_notFallingAllTiles_OUT,
	                   FluidTypes&             p_fall_OUT,
	                   FluidType&              p_fallAllTiles_OUT) const;
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr,
	                 const toki::game::entity::EntityMgr&         p_entityMgr);
	
private:
	typedef std::vector<tt::math::Point2> Point2s;
	typedef std::map<tt::math::Point2, tt::audio::player::SoundCuePtr, tt::math::Point2Less> SoundCues;
	typedef std::map<entity::EntityHandle, tt::audio::player::SoundCuePtr> EntitySoundCues;
	
	struct FluidFlowTypeData
	{
		FluidFlowTypeData()
		:
		timeTillNextUpdate(-1)
		{}
		
		real    timeTillNextUpdate;
		Point2s toFlowList;
	};
	
	
	FluidMgr(const level::AttributeLayerPtr& p_levelLayer);
	
	void collectSourceTiles();
	void simulate();
	void updateGrowth(real p_deltaTime);
	void tickGrowth(NodeType p_fluidFlowType);
	
	real getFlowTime(FluidType p_fluidType, NodeType p_nodeType) const;
	
	FluidFlowType getFlowType(const tt::math::Point2& p_position) const;
	void placeFluidTile(const tt::math::Point2& p_position, FluidType p_type, FluidFlowType p_flowType, bool p_playSound = false);
	void placeFluidTileLvl2(const tt::math::Point2& p_position, FluidFlowType p_flowType);
	void addToToflowList(const tt::math::Point2& p_position, NodeType p_nodeType);
	void setTileBelowFall(const tt::math::Point2& p_position, FluidType p_type);
	
	bool canStartLvl2(const tt::math::Point2& p_lvl1Pos) const;
	bool fillLvl2(const tt::math::Point2& p_lvl1Pos);
	void expandLvl2(FluidFlowType p_lvl2DirType, const tt::math::Point2& p_incomingPos,
	                s32 p_edgePosX, bool* p_hasExpandedOut, bool* p_reachedSimulationEndOut);
	
	void placeFlow(FluidFlowType p_newFluidFlowType, const tt::math::Point2& p_newPosition);
	void placeAdjacentFlow(const tt::math::Point2& p_position, NodeType p_nodeType);
	
	void             addWarpedNode(    const tt::math::Point2& p_sourcePosition, NodeType p_nodeType);
	
	void playAudio(const tt::math::Point2& p_position);
	
	void addTileToChangedList(    const tt::math::Point2&         p_pos);
	void addTileRectToChangedList(const tt::math::PointRect&      p_rect);
	void setAllWarpTiles(         const level::AttributeLayerPtr& p_layer);
	void setWarpTiles(            const tt::math::PointRect&      p_rect,
	                              const level::AttributeLayerPtr& p_layer, bool p_isWarpTile);
	
	movement::SurveyResult  checkForSurveyOnWater(const movement::SurroundingsSurvey& p_survey,
	                                              const tt::math::PointRect&    p_entityRegisteredTileRect,
	                                              const entity::Entity&         p_entity,
	                                              movement::SurveyResult*       p_flowDirection_OUT,
	                                              FluidType p_fluidType) const;
	
	inline bool isBassinInputType(FluidFlowType p_flowType) const
	{
		return p_flowType == FluidFlowType_Fall ||
		       p_flowType == FluidFlowType_StillUnderFall;
	}
	inline bool isIgnorableFlowType(FluidFlowType p_flowType) const
	{
		return p_flowType == FluidFlowType_None;
	}
	inline bool isIgnorableFlowTypeOrOverflow(FluidFlowType p_flowType) const
	{
		return p_flowType == FluidFlowType_None ||
		       p_flowType == FluidFlowType_LeftOverFlow ||
		       p_flowType == FluidFlowType_RightOverFlow;
	}
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
	
	void updateFlowTimes();
	
	void startEntityWave(const tt::math::PointRect& p_newRect,
	                     const tt::math::PointRect* p_oldRect,
	                     const entity::Entity&      p_entity,
	                     FluidType                  p_fluidType);
	
	level::AttributeLayerPtr m_levelLayer;
	level::AttributeLayerPtr m_activeLayer;
	level::AttributeLayerPtr m_simulationLayer;
#if defined(TT_PLATFORM_WIN)
	AttributeDebugViewPtr    m_activeView;
	AttributeDebugViewPtr    m_simulationView;
#endif
	
	FluidFlowTypeData   m_fluidFlowTypeData[FluidType_Count][NodeType_Count];
	tt::cfg::HandleReal m_flowTypeTimings  [FluidType_Count][NodeType_Count];
	real                m_flowTypeTimes    [FluidType_Count][NodeType_Count];
	real                m_drainFallSpeed[FluidType_Count];
	real                m_drainFlowSpeed[FluidType_Count];
	
	bool      m_dirty;
	bool      m_preGenerate;
	bool      m_checkActiveLayerType;
	bool      m_useSimulationLayer;
	FluidType m_activeFluidType;
	
	NodeType         m_currentNodeType;
	tt::math::Point2 m_currentPosition;
	
	Point2Set m_changedTiles;
	Point2Set m_sourceTiles;
	Point2Set m_stillTiles;
	Point2Set m_stillTilesSurface;
	Point2Set m_toFlowScratch;
	Point2s   m_toFlowActiveLayerScratch[FluidType_Count][NodeType_Count];
	
	SoundCues m_soundCues;
	SoundCues m_soundCuesForFalls;
	
	typedef std::vector<tt::engine::particles::ParticleEffectPtr> ParticleEffects;
	typedef std::vector<tt::math::PointRect>                      PointRects;
	
	struct EntityFluidResource
	{
		typedef std::vector<real> Reals;
		
		// FIXME: Merge these bools into a single BitMask.
		bool                           rectsDirty;
		bool                           needsParticleUpdate;
		bool                           needsWaveUpdate;
		PointRects                     tileRects;
		PointRects                     prevTileRectsWaves;
		ParticleEffects                particleEffects;
		Reals                          originalParticleCount;
		tt::audio::player::SoundCuePtr soundCue;
		
		EntityFluidResource()
		:
		rectsDirty(true),
		needsParticleUpdate(false),
		needsWaveUpdate(false),
		tileRects(),
		prevTileRectsWaves(),
		particleEffects(),
		originalParticleCount(),
		soundCue()
		{
		}
	};
	
	typedef std::map<entity::EntityHandle, EntityFluidResource> EntityToFluidResource;
	// We store the entity effect resources here.
	// Per effect and fluid type (So we can easy map it to the onEntity(Enter|Exit)(Fluid|Fall) callbacks)
	// Entities are added when they enter surface/fall tiles and have settings which need to do stuff.
	// Added entities are checked to update the particle rect.
	EntityToFluidResource m_entityFluidResources[EntityFluidEffectType_Count][FluidType_Count];
	
	EntityToFluidResource& modifyEntityFluidResource(EntityFluidEffectType p_EntityFluidEffectType, FluidType p_fluidType)
	{
		if (isValidEntityFluidEffectType(p_EntityFluidEffectType) == false ||
		    isValidFluidType(p_fluidType)             == false)
		{
			TT_PANIC("Incorrect EntityFluidEffectType: %d or FluidType: %d",
			         p_EntityFluidEffectType, p_fluidType);
			return m_entityFluidResources[0][0]; // Need to return something.
		}
		return m_entityFluidResources[p_EntityFluidEffectType][p_fluidType];
	}
	void clearAllEntityFluidResources();
	
	void onEntityEntersImpl( const entity::Entity& p_entity, EntityFluidEffectType p_effectType, FluidType p_fluidType, bool p_breakingSurface);
	void onEntityExitsImpl(  const entity::Entity& p_entity, EntityFluidEffectType p_effectType, FluidType p_fluidType);
	PointRects updateEntityEffectsRects(const entity::Entity& p_entity,
	                                    EntityFluidEffectType p_effectType,
	                                    FluidType             p_fluidType);
	
	// warp pairs
	struct WarpPair
	{
		WarpPair()
		:
		warpRect1(),
		warpRect2(),
		isValid(false)
		{}
		
		tt::math::PointRect warpRect1;
		tt::math::PointRect warpRect2;
		bool                isValid;
	};
	typedef std::map<WarpID, WarpPair> WarpPairs;
	
	WarpPairs m_warpPairs;
	
	// notified entities
	entity::EntityHandleSet m_notifiedEntities;
	
	FluidGraphicsMgr  m_graphicsMgr;
	FluidParticlesMgr m_particlesMgr;

	level::TileRegistrationMgr& m_tileRegMgr;
	
	utils::SectionProfiler<utils::FluidMgrSection, utils::FluidMgrSection_Count> m_sectionProfiler;
	
	// No copying
	FluidMgr(const FluidMgr&);
	FluidMgr& operator=(const FluidMgr&);

	//----------------------------------------------------------

	// Source Management
	bool hasFluidSource(const tt::math::Point2& p_position, FluidType p_type) const;
	bool hasFluidKill(const tt::math::Point2& p_position) const;

	// Fall Management
	void addFall(const tt::math::Point2& p_position, FluidType p_type, FallType p_fallType, WarpID p_warpID = WarpID::invalid);
	void removeFallsFromFlow(Flow& p_flow);
	bool expandFall(Fall& p_fall, real p_elapsedTime);
	void drainFall(Fall& p_fall);
	void handleFallObstructed(const tt::math::Point2& p_position, bool p_waterOnly);
	void addFallFromWarp(const tt::math::Point2& p_sourcePosition, FluidType p_type);

	// Flow Management
	void addFlow(const tt::math::Point2& p_position, FluidType p_type, WarpID p_warpID = WarpID::invalid);
	bool expandFlow(Flow& p_flow, Flow* p_adjacentFlow, real p_elapsedTime); 
	void expandFlow(Flow& p_flow, Flow* p_adjacentFlow, real p_elapsedTime, const tt::math::Point2& p_direction);
	Flow splitFlow(const FluidFlows::iterator& p_originalFlow, const tt::math::Point2 p_splitPoint);
	void splitWaterFlowByLava(const tt::math::Point2 p_splitPoint);
	void drainFlow(Flow& p_flow);
	void shrinkFlowFromLeft (Flow& p_flow, real p_newLeftEdge);
	void shrinkFlowFromRight(Flow& p_flow, real p_newRightEdge);
	void handleFlowObstructed(const tt::math::Point2& p_position);

	// Tile Data
	FlowState setFallTile(const tt::math::Point2& p_position, FluidType p_type, bool p_forceUpdate);
	void      removeFallTiles(s32 p_column, s32 p_startY, s32 p_endY);
	void      removeFallTile(const tt::math::Point2& p_position);
	void      restoreFallTile(const tt::math::Point2& p_position);
	void      setFlowTile(const tt::math::Point2& p_position, FluidType p_type, FluidFlowType p_flowType);
	void      resetFlowTiles(const Flow& p_flow);
	void      removeFlowTiles(const Flow& p_flow);
	void      removeFlowTile      (const tt::math::Point2& p_position, FluidType p_type);
	void      computeFlowUnderFall(s32 p_start, s32 p_end, bool p_fallToLeft, bool p_fallToRight, tt::math::Point2& p_tile);
	void      computeFlowDirection(Flow& p_flow);
	void      setFlowType(const tt::math::Point2& p_position, FluidFlowType p_flowType);

	// Dynamic update
	void handleTileChanged      (const tt::math::Point2& p_position);
	void handleSolidPlaced     (const tt::math::Point2& p_position);
	void handleSolidRemoved    (const tt::math::Point2& p_position);
	void handleFeedPointAdded  (const tt::math::Point2& p_position, FluidType p_type);
	void handleFeedPointRemoved(const tt::math::Point2& p_position, FluidType p_type);
	void insertFeedPoint       (const tt::math::Point2& p_position);
	s32  countFeedPoints(s32 p_row, s32 p_startX, s32 p_endX);
	void handleWarpPlaced (const tt::math::Point2& p_position);
	void handleWarpRemoved(const tt::math::Point2& p_position);
	void removeFallFromPairedWarp(const tt::math::Point2& p_position);
	bool allowedToEnterWarp(const WarpID& p_id, const tt::math::Point2& p_position);

	void updateFalls(real p_elapsedTime);
	void updateFlows(real p_elapsedTime);
	void updateFluids(real p_elapsedTime);
	void renderFluids() const;
	void resetFluidSimulation(bool p_forcePregeneration);
	void addStillWater();
	void updateAudio(const tt::math::VectorRect& p_visibilityRect);
	
	// All falls and flows that currently exist
	LevelFalls              m_falls;
	LevelFlows              m_flows;
	std::vector<FeedPoints> m_feedPoints;
	Point2Set               m_warpTiles;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_FLUIDMGR_H)
