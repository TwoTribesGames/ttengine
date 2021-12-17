#if !defined(INC_TOKI_GAME_LIGHT_LIGHTMGR_H)
#define INC_TOKI_GAME_LIGHT_LIGHTMGR_H


#include <tt/code/HandleArrayMgr.h>
#include <tt/engine/renderer/pp/fwd.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/engine/scene2d/shoebox/fwd.h>
#include <tt/pres/fwd.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/light/Light.h>
#include <toki/game/light/LightRayTracer.h>
#include <toki/level/fwd.h>
#include <toki/level/TileChangedObserver.h>
#include <toki/serialization/fwd.h>
#include <toki/utils/SectionProfiler.h>


#if !defined(TT_BUILD_FINAL)
#	define DO_LIGHT_BLOB_QUAD_DEBUG_RENDER 0 // Change this to 1 to enable blob quad render code.
#else
#	define DO_LIGHT_BLOB_QUAD_DEBUG_RENDER 0 // DO NOT CHANGE! Keep this 0 for final builds
#endif


namespace toki {
namespace game {
namespace light {


class LightMgr : public level::TileChangedObserver
{
public:
	LightMgr(s32 p_reserveCount, const level::AttributeLayerPtr& p_levelLayer);
	~LightMgr();
	
	inline void setAttributeLayer(const level::AttributeLayerPtr& p_levelLayer)
	{
		m_levelLayer          = p_levelLayer;
		m_dirty = true;
	}
	
	LightHandle createLight(const entity::EntityHandle& p_source,
	                        const tt::math::Vector2&    p_offset,
	                        real                        p_radius,
	                        real                        p_strength);
	void destroyLight(LightHandle& p_handle);
	inline Light* getLight(const LightHandle& p_handle) { return m_lights.get(p_handle); }
	
	virtual void onTileChange(const tt::math::Point2& p_position);
	void handleLevelResized();
	void resetLevel();
	
	bool isPositionInLight(const tt::math::Vector2& p_position) const;
	bool isEntityInLight(const entity::Entity& p_entity) const;
	
	void update(real p_deltaTime);
	void updateForRender(const tt::math::VectorRect& p_visibilityRect, s32 p_ambientLight);
	
	inline void updatePostInit()
	{
		if (m_isDarkLevel == false)
		{
			updateDarkness(); // Get (fresh) darkness rects from DarknessMgr.
		}
	}
	
	void startLightRender(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shadowMaskShoebox,
	                      const tt::math::VectorRect& p_visibilityRect);
	void addLightsToRender();
	void endLightRender(const tt::engine::scene2d::shoebox::ShoeboxPtr& p_shadowMaskShoebox,
	                    const tt::math::VectorRect& p_visibilityRect);
	
	void renderLightGlows() const;
	
	void restoreAlphaChannel();
	
	void debugRender();
	
	inline void setDarkLevel(bool p_isDark) { m_isDarkLevel = p_isDark; }
	inline bool isDarkLevel() const         { return m_isDarkLevel;      }
	
	s32 getLevelLightAmbient();
	void setLevelLightAmbient(s32 p_ambient);
	
	inline s32 renderProfiler(s32 p_x, s32 p_y)
	{
		return m_sectionProfiler.render(p_x, p_y, false);
	}
	
	static inline void setLightGlowsFilter(const tt::engine::renderer::pp::FilterPtr& p_filter)
	{
		ms_lightGlowsFilter = p_filter;
	}

	static inline void setShadowFilter(const tt::engine::renderer::pp::FilterPtr& p_gaussH,
	                                   const tt::engine::renderer::pp::FilterPtr& p_gaussV)
	{
		ms_gaussBlurH = p_gaussH;
		ms_gaussBlurV = p_gaussV;
	}

	static void createStaticResources(bool p_renderLightGlows);
	static void destroyStaticResources();
	
	static void setShadowQuality(ShadowQuality p_quality);
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<Light>          Lights;
	typedef std::map<entity::EntityHandle, Polygon*> EntityPolygons;
	typedef std::vector<tt::math::VectorRect>        Rects;
	typedef std::vector<Light*>                      LightPtrs;
	
	void updateStaticOccluders();
	bool updateDynamicOccluders();
	void mergeStaticWithDynamicOccluders();
	
	void updateLightShape(size_t p_index, const Polygons& p_occluders);
	
	void updateDarkness(); // Get (fresh) darkness rects from DarknessMgr.
	void updateSensors();
	inline bool shouldDoRectChecks() const { return m_isDarkLevel == false && m_darknessRects.empty() == false; }
	inline bool shouldDoLight()      const { return m_isDarkLevel          || m_darknessRects.empty() == false; }
	bool isInDarknessRect(const tt::math::Vector2&    p_position, real p_radius = 0.0f) const;
	bool isInDarknessRect(const tt::math::VectorRect& p_rect) const;
	bool isEntityInLightImpl(const entity::Entity& p_entity) const;
	
	void setLevelLightAmbientImpl(s32 p_ambient);
	
	void renderFullScreenQuad2D(const tt::engine::renderer::Quad2DPtr& p_quad);
	
	Lights    m_lights;
	LightPtrs m_visibleLights;
	LightPtrs m_visibleGlows;
	
	//entity::EntityHandleSet m_litEntities;
	//entity::EntityHandleSet m_entitiesInShadowRect; // No darkness rect logic for rewind
	
	tt::engine::renderer::Quad2DPtr m_wholeLevelNoAlpha;
	tt::engine::renderer::Quad2DPtr m_wholeLevelAmbientAlpha;
	tt::engine::renderer::Quad2DPtr m_wholeLevelFullAlpha;
	tt::math::Matrix44              m_fullScreenMtx;
	s32                             m_lightAmbient; // The current alpha set in m_wholeLevelAmbientAlpha.
	
	Polygons       m_staticOccluders;
	EntityPolygons m_dynamicOccluders;
	Polygons       m_allOccluders;
	Rects          m_darknessRects;
	
	level::AttributeLayerPtr m_levelLayer;
	
	real m_currentTime; // A single time for all lights (used for the rotation animations.)
	bool m_dirty;
	bool m_isDarkLevel;
	bool m_shouldRenderDarkness;
	u8   m_defaultLightAmbient;
	
	static tt::engine::renderer::RenderTargetPtr ms_lightGlowsRenderTarget;
	static tt::engine::renderer::pp::FilterPtr   ms_lightGlowsFilter;

	static tt::engine::renderer::RenderTargetPtr ms_shadowPing;
	static tt::engine::renderer::RenderTargetPtr ms_shadowPong;
	static tt::engine::renderer::pp::FilterPtr   ms_gaussBlurH;
	static tt::engine::renderer::pp::FilterPtr   ms_gaussBlurV;
	static tt::engine::renderer::pp::FilterPtr   ms_clearScreen;
	static ShadowQuality                         ms_shadowQuality;
	static s32                                   ms_blurPassCount;
	
#if DO_LIGHT_BLOB_QUAD_DEBUG_RENDER
	// -------------------------------------------------------------
	// DEBUG Members
	class DebugQuad
	{
	public:
		tt::math::VectorRect           rect;
		tt::engine::renderer::ColorRGB color;
		
		inline DebugQuad(const tt::math::Vector2&              p_min,
		                 const tt::math::Vector2&              p_max,
		                 const tt::engine::renderer::ColorRGB& p_color)
		:
		rect(p_min, p_max),
		color(p_color)
		{
		}
	};
	
	typedef std::vector<DebugQuad> DebugQuads;
	DebugQuads m_debugQuads;
#endif
	
	utils::SectionProfiler<utils::LightMgrSection, utils::LightMgrSection_Count> m_sectionProfiler;
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_LIGHTMGR_H)
