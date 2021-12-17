#if !defined(INC_TOKI_SHAREDGRAPHICS_H)
#define INC_TOKI_SHAREDGRAPHICS_H


#include <tt/audio/player/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/thread/Mutex.h>

#include <toki/cfg.h>
#include <toki/constants.h>


namespace toki {

/*! \brief Manages graphics shared between application states (such as loading screen graphics). */
class SharedGraphics
{
public:
	SharedGraphics();
	
	void createGraphics();
	void destroyGraphics();
	void destroyLoadingBgAndLogo();
	bool hasLoadingBgAndLogo() const;
	
	// The following level load texture functions are threadsafe.
	void setLevelLoadTexture(bool p_firstStartup, const std::string& p_levelName);
	tt::engine::renderer::TexturePtr getLevelLoadTexture();
	bool hasLoadedlevelLoadTexture();
	// No longer threadsafe below.
	void createLevelLoadQuad();
	bool hasLevelLoadQuad() const;
	bool isLevelFirstStartup() const;
	
	void startBackgroundMusic();
	void stopBackgroundMusic();
	void setBackgroundMusicVolume(real p_normalizedVolume);
	void updateBackgroundMusic();
	
	void updateAnimTime(real p_deltaTime);
	void updateFadeQuad();
	void updateActivityIndicators();
	void updateLoadingBgAndLogo();
	
	void updateAll();
	
	void renderFadeQuad()                         const;
	void renderActivityIndicator(Screen p_screen) const;
	void renderLoadingBackground(Screen p_screen) const;
	void renderLogo             (Screen p_screen) const;
	void renderLevelLoadQuad    (Screen p_screen) const;
	
	inline const tt::engine::renderer::QuadSpritePtr& getFadeQuad() { return m_fadeQuad; }
	
	const tt::engine::renderer::QuadSpritePtr& getActivityIndicator(Screen p_screen) const;
	const tt::engine::renderer::QuadSpritePtr& getLoadingBackground(Screen p_screen) const;
	void startLogoFadeIn(real p_time);
	
	// In internal builds, app loading can complete before the logo is faded in,
	// which makes the logo (or controls image) invisible during level loading.
	// This function ensures that the logo is visible during level load.
	void ensureLogoVisible();
	
	bool willShowControlsImage();
	bool isControlsImageShownLongEnough() const;
	
private:
	// No copying
	SharedGraphics(const SharedGraphics&);
	SharedGraphics& operator=(const SharedGraphics&);
	
	
	tt::engine::renderer::QuadSpritePtr m_fadeQuad;
	tt::engine::renderer::QuadSpritePtr m_activityIndicator[Screen_Count];
	tt::engine::renderer::QuadSpritePtr m_loadingBackground[Screen_Count];
	tt::engine::renderer::QuadSpritePtr m_logo             [Screen_Count];
	tt::engine::renderer::QuadSpritePtr m_levelLoadQuad    [Screen_Count];
	tt::engine::renderer::QuadSpritePtr m_buttonControls   [Screen_Count];
	
	tt::thread::Mutex                m_levelLoadTextureMutex;
	tt::engine::renderer::TexturePtr m_levelLoadTexture;       // Needs to be thread safe! (Use: m_levelLoadTextureMutex)
	bool                             m_levelLoadTextureWasSet; // Needs to be thread safe! (Use: m_levelLoadTextureMutex)
	bool                             m_levelLoadFirstStartup;  // Needs to be thread safe! (Use: m_levelLoadTextureMutex)
	
	real m_sharedAnimTime;
	real m_aspectActivityIndicator;
	real m_aspectBackground;
	real m_aspectLogo;
	real m_aspectLevelLoad;
	real m_aspectControls;
	
	tt::cfg::HandleReal m_controlsHeight;
	real                m_controlsRemainingDisplayTime;
	
	tt::cfg::HandleReal m_levelLoadTime;
	tt::cfg::HandleReal m_levelLoadScaleMin;
	tt::cfg::HandleReal m_levelLoadScaleMax;
	
	inline tt::math::Vector2 getRandomLevelLoadPos(real p_scale) const
	{
		const real height     = p_scale - 1.0f;
		const real halfHeight = height * 0.5f;
		const real halfWidth  = halfHeight;
		
		tt::math::Vector2 min(-halfWidth, -halfHeight);
		tt::math::Vector2 max( halfWidth,  halfHeight);
		
		return tt::math::Vector2(tt::math::Random::getEffects().getNextReal(min.x, max.x),
		                         tt::math::Random::getEffects().getNextReal(min.y, max.y));
	}
	inline real getRandomLevelLoadScale() const
	{
		return tt::math::Random::getEffects().getNextReal(cfg()->get(m_levelLoadScaleMin),
		                                                  cfg()->get(m_levelLoadScaleMax));
	}
	
	tt::math::Vector2 m_levelLoadStartPos;
	tt::math::Vector2 m_levelLoadEndPos;
	real              m_levelLoadStartScale;
	real              m_levelLoadEndScale;
	real              m_levelLoadAnimTime;
	
	tt::audio::player::MusicPlayerPtr m_backgroundMusic;
};

// Namespace end
}


#endif  // !defined(INC_TOKI_SHAREDGRAPHICS_H)
