#if !defined(INC_TOKI_GAME_EVENT_SOUNDGRAPHICSMGR_H)
#define INC_TOKI_GAME_EVENT_SOUNDGRAPHICSMGR_H

#include <vector>

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/QuadBuffer.h>

#include <toki/game/event/fwd.h>
#include <toki/game/event/helpers/SoundChecker.h>


namespace toki {
namespace game {
namespace event {

class SoundGraphicsMgr
{
public:
	SoundGraphicsMgr();
	
	void registerSound(real p_radius, const helpers::SoundChecker::Locations& p_visitedSoundLocations);
	
	void update(real p_deltatime);
	void updateForRender();
	void render(const tt::math::VectorRect& p_visibilityRect) const;
	void renderLightmask(const tt::math::VectorRect& p_visibilityRect) const;
	
private:
	static const s32 numberOfBatchSettings = 3;
	
	struct BatchSettings
	{
		BatchSettings()
		:
		texture(),
		scale(1.0f),
		lifetime(0.0f),
		uvAnim()
		{ }
		
		tt::engine::renderer::TexturePtr texture;
		real scale;
		real lifetime;
		real fadeInTime;
		real fadeOutTime;
		tt::math::Vector3 uvAnim;
	};
	
	class SoundGraphics
	{
	public:
		SoundGraphics(const tt::engine::renderer::BatchQuadCollection& p_quadBatch,
		              const BatchSettings& p_settings);
		
		void update(real p_deltatime);
		void updateForRender();
		void render() const;
		inline bool isAlive() const { return m_time < m_settings.lifetime; }
		const tt::math::VectorRect& getBoundingRect() const { return m_boundingRect; }
		
	private:
		// No copying
		SoundGraphics(const SoundGraphics&);
		SoundGraphics& operator=(const SoundGraphics&);
		
		BatchSettings                             m_settings;
		tt::engine::renderer::QuadBufferPtr       m_quadBuffer;
		tt::engine::renderer::BatchQuadCollection m_quadBatch;
		real                                      m_time;
		u8                                        m_opacity;
		tt::math::VectorRect                      m_boundingRect;
	};
	typedef tt_ptr<SoundGraphics>::shared SoundGraphicsPtr;
	typedef std::vector<SoundGraphicsPtr> SoundGraphicsCollection;
	
	// No copying
	SoundGraphicsMgr(const SoundGraphicsMgr&);
	SoundGraphicsMgr& operator=(const SoundGraphicsMgr&);
	
	void batchSoundGraphic(const tt::math::Point2& p_tilePosition, real p_angle,
	                       const BatchSettings& p_settings,
	                       tt::engine::renderer::BatchQuadCollection* p_quadBatch_OUT);
	
	BatchSettings           m_settings[numberOfBatchSettings];
	SoundGraphicsCollection m_graphics;
	bool                    m_graphicsNeedUpdate;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_GAME_EVENT_SOUNDGRAPHICSMGR_H)
