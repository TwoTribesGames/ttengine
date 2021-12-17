#include <tt/app/Platform.h>
#include <tt/audio/player/TTIMPlayer.h>
#include <tt/engine/renderer/QuadSprite.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/math/interpolation.h>
#include <tt/thread/CriticalSection.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/utils/utils.h>
#include <toki/AppGlobal.h>
#include <toki/SharedGraphics.h>


namespace toki {

//--------------------------------------------------------------------------------------------------
// Public member functions

SharedGraphics::SharedGraphics()
:
m_fadeQuad(),
m_levelLoadTextureMutex(),
m_levelLoadTexture(),
m_levelLoadTextureWasSet(false),
m_levelLoadFirstStartup(false),
m_sharedAnimTime(0.0f),
m_aspectActivityIndicator(1.0f),
m_aspectBackground(1.0f),
m_aspectLogo(1.0f),
m_aspectLevelLoad(1.0f),
m_aspectControls(1.0f),
m_controlsHeight(),
m_controlsRemainingDisplayTime(0.0f),
m_levelLoadTime(),
m_levelLoadScaleMin(),
m_levelLoadScaleMax(),
m_levelLoadStartPos(tt::math::Vector2::zero),
m_levelLoadEndPos(tt::math::Vector2::zero),
m_levelLoadStartScale(0.0f),
m_levelLoadEndScale(0.0f),
m_levelLoadAnimTime(0.0f),
m_backgroundMusic()
{
}


void SharedGraphics::createGraphics()
{
	using tt::engine::renderer::QuadSprite;
	using tt::engine::renderer::TexturePtr;
	using tt::engine::renderer::TextureCache;
	
	m_fadeQuad = QuadSprite::createQuad(10000.0f, 10000.0f, tt::engine::renderer::ColorRGBA(80, 80, 80, 80));
	m_fadeQuad->resetFlag(QuadSprite::Flag_Visible);
	m_fadeQuad->update();
	
	//TexturePtr texBg       (TextureCache::get("background",         "textures.loading", true));
	//TexturePtr texLogo     (TextureCache::get("logo",               "textures.loading", true));
	TexturePtr texIndicator(TextureCache::get("activity_indicator", "textures.loading", true));
	//TexturePtr texControls (TextureCache::get("controls",           "textures.loading", true));
	
	m_aspectActivityIndicator = static_cast<real>(texIndicator->getWidth()) / texIndicator->getHeight();
	//m_aspectBackground        = static_cast<real>(texBg       ->getWidth()) / texBg       ->getHeight();
	//m_aspectLogo              = static_cast<real>(texLogo     ->getWidth()) / texLogo     ->getHeight();
	//m_aspectControls          = static_cast<real>(texControls ->getWidth()) / texControls ->getHeight();
	
	//const bool cat = (tt::app::getPlatform() == tt::app::Platform_CAT) || (tt::app::getPlatform() == tt::app::Platform_PS4);
	//m_controlsHeight     = cfg()->getHandleReal(std::string("toki.loading.controls.height_") + (cat ? "cat" : "desktop"));
	m_levelLoadTime      = cfg()->getHandleReal("toki.loading.level_preview.time");
	m_levelLoadScaleMin  = cfg()->getHandleReal("toki.loading.level_preview.scale_min");
	m_levelLoadScaleMax  = cfg()->getHandleReal("toki.loading.level_preview.scale_max");
	
	//m_controlsRemainingDisplayTime = cfg()->getRealDirect("toki.loading.controls.min_display_duration");
	
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		m_activityIndicator[i] = QuadSprite::createQuad(texIndicator, tt::engine::renderer::ColorRGB::white);
		//m_loadingBackground[i] = QuadSprite::createQuad(texBg);
		//m_logo             [i] = QuadSprite::createQuad(texLogo,     tt::engine::renderer::ColorRGB::white);
		//m_buttonControls   [i] = QuadSprite::createQuad(texControls, tt::engine::renderer::ColorRGB::white);
		
		// Hide the logo and controls image by default
		//m_logo             [i]->resetFlag(QuadSprite::Flag_Visible);
		//m_buttonControls   [i]->resetFlag(QuadSprite::Flag_Visible);
	}
}


void SharedGraphics::destroyGraphics()
{
	m_fadeQuad.reset();
	m_levelLoadTexture.reset();
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		m_activityIndicator[i].reset();
		m_loadingBackground[i].reset();
		m_logo             [i].reset();
		m_levelLoadQuad    [i].reset();
		//m_buttonControls   [i].reset();
	}
	m_sharedAnimTime = 0.0f;
}


void SharedGraphics::destroyLoadingBgAndLogo()
{
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		m_loadingBackground[i].reset();
		m_logo             [i].reset();
		m_levelLoadQuad    [i].reset();
		//m_buttonControls   [i].reset();
	}
	m_levelLoadTexture.reset();
}


bool SharedGraphics::hasLoadingBgAndLogo() const
{
	// NOTE: Lifetime of these graphics is tied together, so checking one is enough
	return m_loadingBackground[0] != 0;
}


void SharedGraphics::setLevelLoadTexture(bool p_firstStartup, const std::string& p_levelName)
{
	tt::thread::CriticalSection cs(&m_levelLoadTextureMutex);
	
	TT_ASSERTMSG(m_levelLoadTextureWasSet == false,
	             "Trying to set level load texture more than once.");
	
	// Check if we need a level specific load texture. (for load screen)
	using tt::engine::renderer::TextureCache;
	const std::string ns("textures.loading.level");
	
	m_levelLoadTexture.reset();
	m_levelLoadFirstStartup = p_firstStartup;
	
	if (AppGlobal::isInLevelEditorMode())
	{
		// Load an editor loading texture instead of a level texture
		m_levelLoadTexture = TextureCache::get("editor", "textures.loading");
		TT_NULL_ASSERT(m_levelLoadTexture);
	}
	else if (m_levelLoadFirstStartup == false)
	{
		// Try to load texture based on level name.
		if (TextureCache::exists(p_levelName, ns))
		{
			m_levelLoadTexture = TextureCache::get(p_levelName, ns);
			TT_NULL_ASSERT(m_levelLoadTexture);
		}
	}
	
	//if (m_levelLoadTexture == 0) // No level specific texture, use default texture.
	//{
	//	m_levelLoadTexture = TextureCache::get("default", ns);
	//	if (m_levelLoadTexture != 0)
	//	{
	//		m_aspectLevelLoad = static_cast<real>(m_levelLoadTexture->getWidth()) / m_levelLoadTexture->getHeight();
	//	}
	//}
	
	m_levelLoadTextureWasSet = true;
}


tt::engine::renderer::TexturePtr SharedGraphics::getLevelLoadTexture()
{
	tt::thread::CriticalSection cs(&m_levelLoadTextureMutex);
	tt::engine::renderer::TexturePtr result = m_levelLoadTexture;
	TT_ASSERTMSG(m_levelLoadTextureWasSet,
	             "Trying to get level load texture when it wasn't set yet.\n");
	return result;
}


bool SharedGraphics::hasLoadedlevelLoadTexture()
{
	tt::thread::CriticalSection cs(&m_levelLoadTextureMutex);
	bool copy = m_levelLoadTextureWasSet;
	return copy;
}


void SharedGraphics::createLevelLoadQuad()
{
	TT_ASSERT(m_levelLoadTextureWasSet);
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		// Start faded out. (alpha 0)
		const tt::engine::renderer::ColorRGBA color(tt::engine::renderer::ColorRGB::white, 0);
		m_levelLoadQuad[i] = tt::engine::renderer::QuadSprite::createQuad(m_levelLoadTexture, color);
		m_levelLoadQuad[i]->fadeIn(0.5f);
	}
	
	m_levelLoadStartScale = getRandomLevelLoadScale();
	m_levelLoadEndScale   = getRandomLevelLoadScale();
	m_levelLoadStartPos   = getRandomLevelLoadPos(m_levelLoadStartScale);
	m_levelLoadEndPos     = getRandomLevelLoadPos(m_levelLoadEndScale);
	m_levelLoadAnimTime   = 0.0f;
}


bool SharedGraphics::hasLevelLoadQuad() const
{
	return m_levelLoadQuad[0] != 0;
}


bool SharedGraphics::isLevelFirstStartup() const
{
	TT_ASSERT(m_levelLoadTextureWasSet);
	return m_levelLoadFirstStartup;
}


void SharedGraphics::startBackgroundMusic()
{
	if (AppGlobal::isAudioInSilentMode() == false &&
	    AppGlobal::isMusicEnabled())
	{
		m_backgroundMusic.reset(new tt::audio::player::TTIMPlayer(0));
		m_backgroundMusic->setFade(audio::AudioPlayer::getMusicVolumeFromSettings());
		m_backgroundMusic->play("audio/Music/loading_background.ttim", true);
	}
}


void SharedGraphics::stopBackgroundMusic()
{
	if (m_backgroundMusic != 0)
	{
		m_backgroundMusic->stop();
		m_backgroundMusic.reset();
	}
}


void SharedGraphics::setBackgroundMusicVolume(real p_normalizedVolume)
{
	if (m_backgroundMusic != 0)
	{
		m_backgroundMusic->setFade(audio::AudioPlayer::getMusicVolumeFromSettings() * p_normalizedVolume);
	}
}


void SharedGraphics::updateBackgroundMusic()
{
	if (m_backgroundMusic != 0)
	{
		m_backgroundMusic->update();
	}
}


void SharedGraphics::updateAnimTime(real p_deltaTime)
{
	m_sharedAnimTime    += p_deltaTime;
	m_levelLoadAnimTime += p_deltaTime;
	
	using tt::engine::renderer::QuadSprite;
	//if (m_controlsRemainingDisplayTime > 0.0f                         &&
	//    m_buttonControls[0] != 0                                 &&
	//    m_buttonControls[0]->checkFlag(QuadSprite::Flag_Visible) &&
	//    m_buttonControls[0]->checkFlag(QuadSprite::Flag_FadingIn) == false)
	//{
	//	m_controlsRemainingDisplayTime -= p_deltaTime;
	//}
}


void SharedGraphics::updateFadeQuad()
{
	tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	m_fadeQuad->setPosition(renderer->getScreenWidth()  * 0.5f, renderer->getScreenHeight() * 0.5f, 0.0f);
	m_fadeQuad->update();
}


void SharedGraphics::updateActivityIndicators()
{
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		const tt::math::Point2 scrSize(utils::getScreenSize(static_cast<Screen>(i)));
		
		m_activityIndicator[i]->setRotation(-m_sharedAnimTime * tt::math::pi);
		m_activityIndicator[i]->setHeight  (scrSize.y * 0.138f);
		m_activityIndicator[i]->setWidth   (m_activityIndicator[i]->getHeight() * m_aspectActivityIndicator);
		m_activityIndicator[i]->setPosition(scrSize.x * 0.5f, scrSize.y - (scrSize.y * 0.12f), 0.0f);
		m_activityIndicator[i]->update();
	}
}


void SharedGraphics::updateLoadingBgAndLogo()
{
	tt::engine::renderer::Renderer* r = tt::engine::renderer::Renderer::getInstance();
	const real aspectRatio = r->getScreenWidth() / static_cast<real>(r->getScreenHeight());
	
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		const tt::math::Point2  scrSize  (utils::getScreenSize(static_cast<Screen>(i)));
		const tt::math::Vector3 scrCenter(scrSize.x * 0.5f, scrSize.y * 0.5f, 0.0f);
		
		if (m_loadingBackground[i] != 0)
		{
			m_loadingBackground[i]->setPosition(scrCenter);
			m_loadingBackground[i]->setHeight  (static_cast<real>(scrSize.y));
			m_loadingBackground[i]->setWidth   (scrSize.y * m_aspectBackground);
			m_loadingBackground[i]->update();
		}
		
		if (m_logo[i] != 0)
		{
			//tt::math::Vector3 logoOffset(0.0f, -0.1f * scrSize.y, 0.0f);
			//logoOffset.y += tt::math::sin(m_sharedAnimTime * tt::math::halfPi) * 0.01f * scrSize.y;
			m_logo[i]->setPosition(scrCenter);
			m_logo[i]->setHeight  (scrSize.y * 1.0f);
			m_logo[i]->setWidth   (m_logo[i]->getHeight() * aspectRatio);
			m_logo[i]->update();
		}
		
		if (m_levelLoadQuad[i] != 0)
		{
			const real time = cfg()->get(m_levelLoadTime);
			
			if (m_levelLoadAnimTime >= time)
			{
				while (m_levelLoadAnimTime >= time)
				{
					m_levelLoadAnimTime -= time;
				}
				
				m_levelLoadStartScale = m_levelLoadEndScale;
				m_levelLoadEndScale   = getRandomLevelLoadScale();
				m_levelLoadStartPos   = m_levelLoadEndPos;
				m_levelLoadEndPos     = getRandomLevelLoadPos(m_levelLoadEndScale);
			}
			
			const real scale = tt::math::interpolation::Easing<real>::getValue(
					m_levelLoadStartScale,
					m_levelLoadEndScale - m_levelLoadStartScale,
					m_levelLoadAnimTime,
					time,
					tt::math::interpolation::EasingType_QuadraticInOut);
			
			const tt::math::Vector2 pos = tt::math::interpolation::Easing<tt::math::Vector2>::getValue(
					m_levelLoadStartPos,
					m_levelLoadEndPos - m_levelLoadStartPos,
					m_levelLoadAnimTime,
					time,
					tt::math::interpolation::EasingType_QuadraticInOut);
			
			const real size  = static_cast<real>(scrSize.y) * scale;
			tt::math::Vector3 offset(pos.x * scrSize.y * m_aspectLevelLoad,
			                         pos.y * scrSize.y);
			/*
			TT_Printf("pos %f, %f, offset %f, %f, scale: %f, size: %f, m_levelLoadAnimTime: %f\n",
			           pos.x, pos.y, offset.x, offset.y, scale, size, m_levelLoadAnimTime);
			// */
			
			m_levelLoadQuad[i]->setPosition(scrCenter + offset);
			m_levelLoadQuad[i]->setWidth   (size * m_aspectLevelLoad);
			m_levelLoadQuad[i]->setHeight  (size);
			m_levelLoadQuad[i]->update();
		}
		
		//if (m_buttonControls[i] != 0)
		//{
		//	const bool cat = ((tt::app::getPlatform() == tt::app::Platform_CAT) || (tt::app::getPlatform() == tt::app::Platform_PS4));
		//	const real imageHeight = scrSize.y * cfg()->get(m_controlsHeight);
		//	// Cat is bottom aligned and desktop is center aligned.
		//	const real yPos = (cat) ? scrSize.y - (imageHeight * 0.5f) : scrSize.y * 0.5f;
		//	tt::math::Vector3 pos(scrSize.x * 0.5f, yPos, 0.0f);
		//	m_buttonControls[i]->setPosition(pos);
		//	m_buttonControls[i]->setHeight  (imageHeight);
		//	m_buttonControls[i]->setWidth   (imageHeight * m_aspectControls);
		//	m_buttonControls[i]->update();
		//}
	}
}


void SharedGraphics::updateAll()
{
	updateFadeQuad();
	updateActivityIndicators();
	updateLoadingBgAndLogo();
}


void SharedGraphics::renderFadeQuad() const
{
	m_fadeQuad->render();
}


void SharedGraphics::renderActivityIndicator(Screen p_screen) const
{
	if (isValidScreen(p_screen) && m_activityIndicator[p_screen] != 0)
	{
		m_activityIndicator[p_screen]->render();
	}
}


void SharedGraphics::renderLoadingBackground(Screen p_screen) const
{
	if (isValidScreen(p_screen) && m_loadingBackground[p_screen] != 0)
	{
		m_loadingBackground[p_screen]->render();
	}
}


void SharedGraphics::renderLogo(Screen p_screen) const
{
	if (isValidScreen(p_screen) == false)
	{
		return;
	}
	
	//if (isLevelFirstStartup())
	//{
	//	if (m_buttonControls[p_screen] != 0)
	//	{
	//		m_buttonControls[p_screen]->render();
	//	}
	//}
	//else
	{
		if (m_logo[p_screen] != 0)
		{
			m_logo[p_screen]->render();
		}
	}
}


void SharedGraphics::renderLevelLoadQuad(Screen p_screen) const
{
	if (isValidScreen(p_screen) && m_levelLoadQuad[p_screen] != 0)
	{
		m_levelLoadQuad[p_screen]->render();
	}
}


const tt::engine::renderer::QuadSpritePtr& SharedGraphics::getActivityIndicator(Screen p_screen) const
{
	TT_ASSERT(isValidScreen(p_screen));
	return m_activityIndicator[p_screen];
}


const tt::engine::renderer::QuadSpritePtr& SharedGraphics::getLoadingBackground(Screen p_screen) const
{
	TT_ASSERT(isValidScreen(p_screen));
	return m_loadingBackground[p_screen];
}


void SharedGraphics::startLogoFadeIn(real p_time)
{
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		if (m_logo[i] != 0)
		{
			m_logo[i]->fadeIn(p_time);
		}
		//if (m_buttonControls[i] != 0)
		//{
		//	m_buttonControls[i]->fadeIn(p_time);
		//}
	}
}


void SharedGraphics::ensureLogoVisible()
{
	using tt::engine::renderer::QuadSprite;
	for (s32 i = 0; i < Screen_Count; ++i)
	{
		if (m_logo[i] != 0                                           &&
		    m_logo[i]->checkFlag(QuadSprite::Flag_Visible)  == false &&
		    m_logo[i]->checkFlag(QuadSprite::Flag_FadingIn) == false)
		{
			m_logo[i]->setOpacity(255);
		}
		//if (m_buttonControls[i] != 0                                           &&
		//    m_buttonControls[i]->checkFlag(QuadSprite::Flag_Visible)  == false &&
		//    m_buttonControls[i]->checkFlag(QuadSprite::Flag_FadingIn) == false)
		//{
		//	m_buttonControls[i]->setOpacity(255);
		//}
	}
}


bool SharedGraphics::willShowControlsImage()
{
	//tt::thread::CriticalSection cs(&m_levelLoadTextureMutex);  // needed for m_levelLoadTextureWasSet
	//// Assume we will be showing the controls image if no decision was made yet
	//return m_levelLoadTextureWasSet == false || isLevelFirstStartup();
	return false;
}


bool SharedGraphics::isControlsImageShownLongEnough() const
{
	// Don't force extra 'loading' times if users don't want to precache (which implies they want fast startup)
	return AppGlobal::shouldDoPrecache() == false ||
	       m_controlsRemainingDisplayTime <= 0.0f;
}

// Namespace end
}
