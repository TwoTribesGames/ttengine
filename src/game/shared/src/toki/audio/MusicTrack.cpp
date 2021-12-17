#include <tt/audio/player/MusicPlayerCallbackInterface.h>
#include <tt/audio/player/TTIMPlayer.h>
#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/math/math.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/audio/MusicTrack.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/script/wrappers/MusicTrackWrapper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/AppGlobal.h>
#include <toki/constants.h>


namespace toki {
namespace audio {

const std::string MusicTrack::ms_musicPath     ("audio/Music/");
const std::string MusicTrack::ms_musicExtension("ttim");


//--------------------------------------------------------------------------------------------------
// Helper class to handle callbacks from the music player

class MusicTrackCallbackForwarder : public tt::audio::player::MusicPlayerCallbackInterface
{
public:
	inline explicit MusicTrackCallbackForwarder(const MusicTrackHandle& p_musicTrack)
	:
	m_musicTrack(p_musicTrack)
	{
	}
	
	virtual void onMusicPlayerLooped()
	{
		MusicTrack* musicTrack = m_musicTrack.getPtr();
		if (musicTrack != 0)
		{
			game::entity::Entity* callbackEntity = musicTrack->getCallbackEntity().getPtr();
			if (callbackEntity != 0)
			{
				game::script::EntityBasePtr script = callbackEntity->getEntityScript();
				if (script != 0)
				{
					script->queueSqFun("onMusicTrackLooped", game::script::wrappers::MusicTrackWrapper(m_musicTrack));
				}
			}
		}
	}
	
private:
	MusicTrackHandle m_musicTrack;
};



//--------------------------------------------------------------------------------------------------
// Public member functions

MusicTrack::MusicTrack(const CreationParams& p_creationParams, const MusicTrackHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_player(),
m_playerCallbackInterface(),
m_creationParams(p_creationParams),
m_normalizedVolumeFade(1.0f),
m_stopTrackAfterVolumeFade(false),
m_callbackEntity(),
m_shouldStartPlaying(false)
{
	if (AppGlobal::isAudioInSilentMode() == false &&
	    AppGlobal::isMusicEnabled())
	{
		m_player.reset(new tt::audio::player::TTIMPlayer(0));
		applyPlayerVolume(m_normalizedVolumeFade.getValue());
		
		m_playerCallbackInterface.reset(new MusicTrackCallbackForwarder(m_ownHandle));
		m_player->setCallbackInterface(m_playerCallbackInterface);
		m_player->preload(getTrackFilename());
	}
}


MusicTrack::~MusicTrack()
{
}


void MusicTrack::play()
{
	if (m_player == 0)
	{
		return;
	}
	
	m_shouldStartPlaying = true;
}


void MusicTrack::stop()
{
	m_shouldStartPlaying = false;
	if (m_player != 0)
	{
		m_player->stop();
	}
	
	m_stopTrackAfterVolumeFade = false;
}


void MusicTrack::pause()
{
	if (m_player != 0)
	{
		m_player->pause();
	}
}


void MusicTrack::resume()
{
	if (m_player != 0)
	{
		m_player->resume();
	}
}


bool MusicTrack::isPlaying() const
{
	return (m_player != 0 && m_player->isPlaying());
}


real MusicTrack::getVolume() const
{
	return m_normalizedVolumeFade.getEndValue();
}


void MusicTrack::setVolume(real p_normalizedVolume, real p_fadeDuration, bool p_stopTrackAfterFade)
{
	real clampedVolume = p_normalizedVolume;
	if (tt::math::clamp(clampedVolume, 0.0f, 1.0f))
	{
		TT_NONFATAL_PANIC("Invalid track volume specified: %f . Must be a normalized volume (range 0 - 1). "
		                  "Volume has been clamped to %f.", p_normalizedVolume, clampedVolume);
	}
	
	m_stopTrackAfterVolumeFade = p_stopTrackAfterFade;
	
	if (p_fadeDuration <= 0.0f)
	{
		// Instantly set new volume
		m_normalizedVolumeFade = TLI(clampedVolume);
		applyPlayerVolume(clampedVolume);
		
		if (p_stopTrackAfterFade)
		{
			stop();
		}
	}
	else
	{
		// Fade to new volume
		m_normalizedVolumeFade.startNewInterpolation(clampedVolume, p_fadeDuration);
	}
}


void MusicTrack::handleMusicVolumeSettingChanged()
{
	// Simply re-apply the volume (will take the current volume setting into account)
	applyPlayerVolume(m_normalizedVolumeFade.getValue());
}


void MusicTrack::update(real p_deltaTime)
{
	if (m_player == 0)
	{
		return;
	}
	
	if (m_shouldStartPlaying && m_player->isPaused() == false)
	{
		if (m_player->play(getTrackFilename(), true) == false)
		{
			TT_PANIC("Could not play music track '%s'.", m_creationParams.musicName.c_str());
		}
		m_shouldStartPlaying = false;
	}
	
	if (m_player->isPlaying() && m_player->isPaused() == false)
	{
		if (m_normalizedVolumeFade.isDone() == false)
		{
			m_normalizedVolumeFade.update(p_deltaTime);
			applyPlayerVolume(m_normalizedVolumeFade.getValue());
		}
		else if (m_stopTrackAfterVolumeFade)
		{
			stop();
		}
	}
	
	m_player->update();
}


void MusicTrack::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_creationParams.musicName, p_context);
}


MusicTrack::CreationParams MusicTrack::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const std::string musicName = bu::get<std::string>(p_context);
	
	return CreationParams(musicName);
}


void MusicTrack::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const bool playerIsPaused = (m_player != 0 && m_player->isPaused());
	
	bu::putTLI   (m_normalizedVolumeFade,      p_context);
	bu::put      (m_stopTrackAfterVolumeFade,  p_context);
	bu::putHandle(m_callbackEntity,            p_context);
	bu::put      (isPlaying(),                 p_context);
	bu::put      (playerIsPaused,              p_context);
	bu::put      (m_shouldStartPlaying,        p_context);
	
	const s32 currentBlock = m_player != 0 ? m_player->getCurrentBlock() : -1;
	bu::put      (currentBlock,                p_context);
}


void MusicTrack::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_normalizedVolumeFade     = bu::getTLI   <real                >(p_context);
	m_stopTrackAfterVolumeFade = bu::get      <bool                >(p_context);
	m_callbackEntity           = bu::getHandle<game::entity::Entity>(p_context);
	const bool shouldPlay      = bu::get      <bool                >(p_context);
	const bool wasPaused       = bu::get      <bool                >(p_context);
	m_shouldStartPlaying       = bu::get      <bool                >(p_context);
	const s32 currentBlock     = bu::get      <s32                 >(p_context);
	
	if (m_player != 0)
	{
		m_player->preloadBlock(getTrackFilename(), currentBlock);
	}
	
	applyPlayerVolume(m_normalizedVolumeFade.getValue());
	
	if (shouldPlay)
	{
		play();
		
		if (wasPaused)
		{
			pause();
		}
	}
}


MusicTrack* MusicTrack::getPointerFromHandle(const MusicTrackHandle& p_handle)
{
	return (AudioPlayer::hasInstance()) ? AudioPlayer::getInstance()->getMusicTrackMgr().getTrack(p_handle) : 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

std::string MusicTrack::getTrackFilename() const
{
	return ms_musicPath + m_creationParams.musicName + "." + ms_musicExtension;
}


void MusicTrack::applyPlayerVolume(real p_normalizedVolume)
{
	if (m_player != 0)
	{
		const real musicVolume(AudioPlayer::hasInstance() ? AudioPlayer::getInstance()->getVolume(Category_Music) : 1.0f);
		m_player->setFade(musicVolume * AudioPlayer::getMusicVolumeFromSettings() * p_normalizedVolume);
	}
}

// Namespace end
}
}
