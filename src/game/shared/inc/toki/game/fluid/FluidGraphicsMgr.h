#if !defined(INC_TOKI_GAME_FLUID_FLUIDGRAPHICSMGR_H)
#define INC_TOKI_GAME_FLUID_FLUIDGRAPHICSMGR_H


#include <map>
#include <set>
#include <vector>

#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/QuadBuffer.h>
#include <tt/engine/renderer/TrianglestripBuffer.h>
#include <tt/math/Rect.h>
#include <tt/platform/tt_types.h>

#include <toki/game/fluid/fwd.h>
#include <toki/game/fluid/graphics_helpers.h>
#include <toki/game/fluid/WaveGenerator.h>
#include <toki/level/fwd.h>
#include <toki/utils/SectionProfiler.h>


namespace tt {
namespace engine {
namespace renderer {

struct BatchQuad;
typedef std::vector<BatchQuad> BatchQuadCollection;

}
}
}

namespace toki {
namespace game {
namespace fluid {

/*! \brief Manages the visualization of fluids in a level. */
class FluidGraphicsMgr
{
public:
	FluidGraphicsMgr();

	void update(real p_delta, utils::FluidMgrSectionProfiler& p_sectionProfiler);
	
	void updateForRender(const level::AttributeLayerPtr& p_layer,
	                     FluidParticlesMgr&              p_particleMgr,
	                     utils::FluidMgrSectionProfiler& p_sectionProfiler,
	                     const tt::math::VectorRect&     p_visibilityRect,
	                     const LevelFalls&               p_falls,
	                     const LevelFlows&               p_flows);
	
	void renderBack();
	void renderFront();
	void renderBackStillWater();
	void renderFrontStillWater();
	void renderLavaGlow();
	
	void handleLevelResized();
	
	inline WaveGenerator& getWaveGenerator() { return m_waveGenerator; }
	
	// DEBUG: Allow toggling the fluid graphics rendering while it is still unfinished
	inline bool isEnabled() const          { return m_enabled;      }
	inline void setEnabled(bool p_enabled) { m_enabled = p_enabled; }
	
	inline bool isDebugRenderEnabled() const          { return m_debugRenderEnabled;      }
	inline void setDebugRenderEnabled(bool p_enabled) { m_debugRenderEnabled = p_enabled; }
	
	void createStillFluidsQuads(const Point2Set& p_tiles, const level::AttributeLayerPtr& p_layer,
	                            Point2Set* p_surfaceTiles_OUT);
	
private:	
	struct WaveTrigger
	{
		real timeTillNextWave;
		bool active;
		s32 waveQuadIndex;
	};
	typedef std::map<tt::math::Point2, WaveTrigger, tt::math::Point2Less> WaveTimers;
	
	// StillFluid Typedefs
	struct TileQuad
	{
		s32 startY;
		s32 endY;
		s32 xPos;
		bool flow;
		
		TileQuad()
		: startY(-1), endY(-1), xPos(-1), flow(false) { }
		
		TileQuad(const tt::math::Point2& p_tile)
		: startY(p_tile.y), endY(p_tile.y), xPos(p_tile.x), flow(false) { }
		
		bool isValid() const { return startY != -1; }
		
	};
	struct VecQuad
	{
		tt::math::Vector2 min;
		tt::math::Vector2 max;
		
		VecQuad(const tt::math::Vector2& p_min, const tt::math::Vector2& p_max)
		: min(p_min), max(p_max) { }
	};
	typedef std::vector<TileQuad> TileQuads;
	typedef std::vector<VecQuad>  VecQuads;
	typedef std::vector<s32>      S32s;
	
	void addStillFluidEdges(const TileQuads& p_tileQuads, S32s& p_prevTiles, S32s& p_curTiles, s32 p_blobHeight,
	                        VecQuads& p_vecQuads, S32s& p_prevTileRightQuadScratch, S32s& p_currentTileLeftQuadScratch,
	                        s32 p_xPos, s32 p_blobYPos);
	
	FluidGraphicsMgr(const FluidGraphicsMgr&);            // No copying
	FluidGraphicsMgr& operator=(const FluidGraphicsMgr&); // No assigment
	
	
	bool m_enabled; // DEBUG: whether the fluid graphics rendering is enabled (off by default)
	bool m_debugRenderEnabled; // DEBUG: Should the debug rects be rendered as well?
	
	WaveGenerator m_waveGenerator;
	
	//-----------------------------------
	
	void initFluidSettings();
	void updateWaves(utils::FluidMgrSectionProfiler& p_sectionProfiler, real p_delta);
	
	typedef std::list<tt::math::Vector2> Borders;
	
	void createQuadsForFall(const Fall& p_fall);
	void createQuadsForFallBorders(
		const Borders& p_borders, s32 p_column, tt::engine::renderer::BatchQuadCollection& p_batch);
	void createFallGeometry(const LevelFalls& p_falls, const tt::math::VectorRect& p_visibilityRect,
		const level::AttributeLayerPtr& p_layer, FluidParticlesMgr& p_particleMgr);
	void fillQuad(tt::engine::renderer::BatchQuad& p_quad, 
		real p_left, real p_right, real p_top, real p_bottom, u8 p_topAlpha, u8 p_bottomAlpha, 
		const tt::math::Vector2& p_uvOffset);
	void generateFallParticles(const Fall&                    p_fall,
		                       const level::AttributeLayerPtr& p_layer,
	                           FluidParticlesMgr&              p_particleMgr);

	void addFlowCorner(tt::engine::renderer::TrianglestripVertices& p_vertices,
	                   real p_pivotX, real p_pivotY, real p_startAngle, real p_width, real p_height, real p_speed);

	void getCornerSettings(FlowState p_state, real& p_offsetOUT, real& p_widthOUT, real p_percentage);
	void createWaveQuads(FluidType p_type, bool p_isStill, real p_x, real p_y, real p_heightLeft, real p_heightRight, u8 leftAlpha, u8 rightAlpha);
	void createMiddlePart(const Flow& p_flow, bool p_hasLeftPart, bool p_hasRightPart, bool p_isStill);
	void createLeftPart  (const Flow& p_flow, real p_cornerOffset, real p_cornerWidth);
	void createRightPart (const Flow& p_flow, real p_cornerOffset, real p_cornerWidth);
	void createFlowGeometry(const LevelFlows& p_flows, const tt::math::VectorRect& p_visibilityRect, FluidParticlesMgr& p_particleMgr);
	void addWavePosition(tt::engine::renderer::TrianglestripVertices& p_vertices,
		real p_x, real p_y, real p_height, u8 p_alpha, bool p_upper, real p_speed, bool p_fadeBottom = false);
	void fillFlowQuad(tt::engine::renderer::BatchQuad& p_quad, 
		real p_left, real p_right, real p_top, real p_bottom, u8 p_topAlpha, u8 p_bottomAlpha, 
		const tt::math::Vector2& p_uvOffset, real p_topV, real p_bottomV);

	tt::engine::renderer::BatchQuad& getNextFallQuad(FluidType p_type);
	tt::engine::renderer::BatchQuad& getNextFlowQuad(FluidType p_type, bool p_isStill);

	tt::engine::renderer::TrianglestripBufferPtr getEdgeBuffer();

	void createFlowForStill(const tt::math::Point2& p_startTile, const tt::math::Point2& p_endTile, FluidType p_type);

	void handleIgnoreFog (FluidType p_type);
	void restoreIgnoreFog();

	tt::engine::renderer::TexturePtr          m_fluidTextures[QuadType_Count];

	tt::engine::renderer::QuadBufferPtr       m_fallBodyBuffer[FluidType_Count];
	tt::engine::renderer::BatchQuadCollection m_fallBodyBatch [FluidType_Count];
	u32                                       m_fallBodyQuadCount[FluidType_Count];

	tt::engine::renderer::QuadBufferPtr       m_fallLeftBorderBuffer [FluidType_Count];
	tt::engine::renderer::QuadBufferPtr       m_fallRightBorderBuffer[FluidType_Count];
	tt::engine::renderer::BatchQuadCollection m_fallLeftBorderBatch  [FluidType_Count];
	tt::engine::renderer::BatchQuadCollection m_fallRightBorderBatch [FluidType_Count];
	
	tt::engine::renderer::QuadBufferPtr       m_flowMiddleBuffer[FluidType_Count];
	tt::engine::renderer::BatchQuadCollection m_flowMiddleBatch [FluidType_Count];
	u32                                       m_flowQuadCount   [FluidType_Count];
	
	tt::engine::renderer::QuadBufferPtr       m_flowStillMiddleBuffer[FluidType_Count];
	tt::engine::renderer::BatchQuadCollection m_flowStillMiddleBatch [FluidType_Count];
	u32                                       m_flowStillQuadCount   [FluidType_Count];

	tt::engine::renderer::QuadBufferPtr       m_lavaGlowBuffer;
	tt::engine::renderer::BatchQuadCollection m_lavaGlowBatch;
	tt::engine::renderer::BatchQuad           m_lavaGlowQuad;

	tt::engine::renderer::QuadBufferPtr       m_stillFluidsBuffer[FluidType_Count];
	
	typedef std::vector<tt::engine::renderer::TrianglestripBufferPtr> FlowEdgeBuffers;
	tt::engine::renderer::TrianglestripVertices  m_flowBodyVertices[FluidType_Count];
	tt::engine::renderer::TrianglestripVertices  m_flowLowerVertices[FluidType_Count];
	FlowEdgeBuffers m_flowEdges [FluidType_Count];
	FlowEdgeBuffers m_edgeBuffers;
	u32             m_edgeBuffersUsed;

	tt::math::Vector2 m_fallUVOffset[FluidType_Count];
	tt::math::Vector2 m_fallBorderUVOffset[FluidType_Count];
	tt::math::Vector3 m_fallUVSpeedFront[FluidType_Count];
	tt::math::Vector3 m_fallUVSpeedBack [FluidType_Count];
	
	tt::math::Vector2 m_flowUVOffset[FluidType_Count];
	tt::math::Vector3 m_flowUVSpeed[FluidType_Count];
	
	tt::math::Vector2 m_stillUVOffset[FluidType_Count];
	
	real m_waveInterval[FluidType_Count];
	real m_waveHeight[FluidType_Count];
	real m_drainLevel;
	real m_animationTime;

	bool m_originalFogSetting;
	bool m_graphicsNeedUpdate;
	bool m_stillBufferNeedsUpdate;

	FluidFlows m_stillFlows;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_FLUID_FLUIDGRAPHICSMGR_H)
