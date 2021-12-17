#include <cmath>
#include <string.h>

#include <tt/audio/helpers.h>
#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Wave.h>
#include <tt/audio/xact/WaveInstance.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/snd.h>
#include <tt/snd/Voice.h>
#include <tt/system/Time.h>


//#define WAVE_DEBUG
#ifdef WAVE_DEBUG
	#define Wave_Printf TT_Printf
#else
	#define Wave_Printf(...)
#endif


//#define WAVE_TRACE
#ifdef WAVE_TRACE
	#define Wave_Trace TT_Printf
#else
	#define Wave_Trace(...)
#endif


//#define WAVE_WARN
#ifdef WAVE_WARN
	#define Wave_Warn TT_Printf
#else
	#define Wave_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

WaveInstance::WaveInstance(Wave* p_wave)
:
m_wave(p_wave),
m_volumeInDB(0.0f),
m_pitch(1.0f),
m_pan(0),
m_position(math::Vector3::zero),
m_emitterRadiusInner(-1.0f),
m_emitterRadiusOuter(-1.0f),
m_reverbVolumeInDB(-96.0f),
m_hwVolume(1.0f),
m_voice(),
m_paused(false),
m_isPositional(false),
m_silenceTime(0),
m_currentTime(0)
{
	Wave_Trace("WaveInstance::WaveInstance: %p\n", p_wave);
	
	TT_ASSERTMSG(m_wave != 0, "WaveInstance::WaveInstance: wave must not be 0");
}


WaveInstance::~WaveInstance()
{
	Wave_Trace("WaveInstance::~WaveInstance\n");
	
	if (isPlaying())
	{
		stop();
	}
}


void WaveInstance::setVolume(real p_volumeInDB, real p_normalizedCategoryVolume)
{
	Wave_Trace("WaveInstance::setVolume: %f\n", realToFloat(p_volumeInDB));

	if (m_wave->isSilent()) return;
	
	if (p_volumeInDB > 6.0f)
	{
		Wave_Warn("WaveInstance::setVolume: volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		Wave_Warn("WaveInstance::setVolume: volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	TT_ASSERTMSG(p_normalizedCategoryVolume >= 0.0f && p_normalizedCategoryVolume <= 1.0f,
	             "Category volume has to be a normalized number! found: %f",
	             realToFloat(p_normalizedCategoryVolume));
	
	m_volumeInDB = p_volumeInDB;
	
	// HACK BEUN: XACT somehow outputs global sfx too loud; louder than a positional sound with infinite radius.
	// To account for this for all platforms, we lowered the positional volume by 5 dB.
	if (m_isPositional)
	{
		m_hwVolume   = (helpers::dBToRatio(m_volumeInDB - 5.0f) * p_normalizedCategoryVolume * AudioTT::getMasterVolume());
	}
	else
	{
		m_hwVolume   = (helpers::dBToRatio(m_volumeInDB) * p_normalizedCategoryVolume * AudioTT::getMasterVolume());
	}
	
	// Check with a little range extra, just so we ignore small rounding errors.
	TT_ASSERTMSG(math::realGreaterEqual(m_hwVolume, -0.1f) && math::realLessEqual(m_hwVolume, 2.0f),
	             "Hardware volume isn't normalized! - "
	             "m_hwVolume: %f, m_volumeInDB: %f, p_normalizedCategoryVolume: %f, masterVolume: %f. "
	             "(wave filename: '%s')\n",
	             realToFloat(m_hwVolume), realToFloat(m_volumeInDB), realToFloat(p_normalizedCategoryVolume),
	             realToFloat(AudioTT::getMasterVolume()), m_wave->getFileName().c_str());
	
	if (m_voice != 0)
	{
		m_voice->setVolumeRatio(m_hwVolume);
	}
}


void WaveInstance::setReverbVolume(real p_volumeInDB)
{
	Wave_Trace("WaveInstance::setReverbVolume: %f\n", realToFloat(p_volumeInDB));
	
	if (m_wave->isSilent()) return;
	
	if (p_volumeInDB > 6.0f)
	{
		Wave_Warn("WaveInstance::setReverbVolume: volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		Wave_Warn("WaveInstance::setReverbVolume: volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	m_reverbVolumeInDB = p_volumeInDB;
	
	if (m_voice != 0)
	{
		m_voice->setReverbVolume(m_reverbVolumeInDB);
	}
}


void WaveInstance::setPitch(real p_pitch)
{
	Wave_Trace("WaveInstance::setPitch: %f\n", realToFloat(p_pitch));
	
	if (m_wave->isSilent()) return;

	// Total pitch is 12 for the track + an optional 12 through pitch variables. When this is out of bounds, something happened
	if (p_pitch > 24.0f)
	{
		TT_WARN("WaveInstance::setPitch: pitch %f larger than 24.0; clamping\n", realToFloat(p_pitch));
		p_pitch = 24.0f;
	}
	
	if (p_pitch < -24.0f)
	{
		TT_WARN("WaveInstance::setPitch: pitch %f less than -24.0; clamping\n", realToFloat(p_pitch));
		p_pitch = -24.0f;
	}
	
	m_pitch = p_pitch;
	
	if (m_voice != 0)
	{
		m_voice->setPlaybackRatio(helpers::semiTonesToRatio(m_pitch));
	}
}


void WaveInstance::setPan(int p_pan)
{
	Wave_Trace("WaveInstance::setPan: %d\n", p_pan);

	if (m_wave->isSilent()) return;

	if (p_pan > 359)
	{
		Wave_Warn("WaveInstance::setPan: panning %f larger than 359; clamping\n", p_pan);
		p_pan = 359;
	}
	
	if (p_pan < 0)
	{
		Wave_Warn("WaveInstance::setPan: panning %f less than 0; clamping\n", p_pan);
		p_pan = 0;
	}
	
	m_pan = p_pan;
	
	if (m_voice != 0)
	{
		m_voice->set360Panning(static_cast<real>(m_pan));
	}
}


bool WaveInstance::setPosition(const math::Vector3& p_position)
{
	m_isPositional = true;
	m_position     = p_position;
	
	if (m_voice != 0 && snd::is3DAudioEnabled(m_voice->getSoundSystem()))
	{
		return m_voice->setPosition(p_position);
	}
	
	return true;
}


bool WaveInstance::setEmitterRadius(real p_inner, real p_outer)
{
	m_isPositional       = true;
	m_emitterRadiusInner = p_inner;
	m_emitterRadiusOuter = p_outer;
	
	if (m_voice != 0 && snd::is3DAudioEnabled(m_voice->getSoundSystem()))
	{
		return m_voice->setRadius(p_inner, p_outer);
	}
	
	return true;
}


bool WaveInstance::play(bool p_loop, snd::size_type p_priority)
{
	Wave_Trace("WaveInstance::play \n");

	if (m_wave->isSilent())
	{
		Wave_Trace("Playing silence for %g seconds...\n", m_wave->getDuration());
		m_silenceTime = m_currentTime + m_wave->getDuration();
		return true;
	}
	
	
	if (m_wave->getBuffer() == 0)
	{
		Wave_Warn("WaveInstance::play cannot load wave data\n");
		return false;
	}
	
	if (m_voice == 0)
	{
		// Generate a source
		m_voice = snd::openVoice(p_priority, AudioTT::getSoundSystem());
		if (m_voice == 0)
		{
			Wave_Warn("WaveInstance::play cannot allocate voice\n");
			return false;
		}
		
		if (m_voice->setBuffer(m_wave->getBuffer()) == false)
		{
			Wave_Warn("WaveInstance::play cannot set buffer\n");
			m_voice.reset();
			return false;
		}
	}
	
	// not pretty with short wav loops
	if (isPlaying())
	{
		Wave_Warn("Already playing!\n");
		//already playing
		return false;
	}
	
	m_voice->setPlaybackRatio(helpers::semiTonesToRatio(m_pitch));
	m_voice->setVolumeRatio(m_hwVolume);
	m_voice->setReverbVolume(m_reverbVolumeInDB);
	if (snd::is3DAudioEnabled(m_voice->getSoundSystem()) && m_isPositional)
	{
		m_voice->setPosition(m_position);
		if (math::realGreaterEqual(m_emitterRadiusInner, 0.0f) &&
		    math::realGreaterEqual(m_emitterRadiusOuter, 0.0f))
		{
			m_voice->setRadius(m_emitterRadiusInner, m_emitterRadiusOuter);
		}
	}
	else
	{
		m_voice->set360Panning(static_cast<real>(m_pan));
	}
	
	if (m_voice->play(p_loop) == false)
	{
		Wave_Warn("WaveInstance::play cannot play the source\n");
		m_voice.reset();
		return false;
	}
	//TT_Printf("Playing wave: %s\n", m_wave->getFileName().c_str());
	
	return true;
}


bool WaveInstance::stop()
{
	Wave_Trace("WaveInstance::stop\n");
	
	if (m_voice != 0)
	{
		if (m_voice->isPlaying())
		{
			m_voice->stop();
		}
		m_voice.reset();
	}
	
	return true;
}


bool WaveInstance::pause()
{
	Wave_Trace("WaveInstance::pause\n");
	
	if (m_paused == false)
	{
		if (m_voice != 0)
		{
			if (m_voice->pause() == false)
			{
				TT_Printf("WaveInstance::pause failed\n");
				return false;
			}
			m_paused = true;
		}
		else if (m_wave->isSilent())
		{
			m_paused = true;
		}
	}
	
	return true;
}


bool WaveInstance::resume()
{
	Wave_Trace("WaveInstance::resume\n");
	
	if (m_paused)
	{
		if (m_voice != 0)
		{
			if (m_voice->resume() == false)
			{
				TT_Printf("WaveInstance::resume failed\n");
				return false;
			}
			m_paused = false;
		}
		else if (m_wave->isSilent())
		{
			m_paused = false;
		}
	}
	
	return true;
}


void WaveInstance::update(real p_time)
{
	Wave_Trace("WaveInstance::update\n");
	m_currentTime = p_time;
}


bool WaveInstance::isPlaying()
{
	Wave_Trace("WaveInstance::isPlaying\n");
	
	if (m_wave->isSilent())
	{
		return m_currentTime < m_silenceTime;
	}
	
	return m_voice != 0 && m_voice->isPlaying();
}


const std::string& WaveInstance::getWaveFilename() const
{
	return m_wave->getFileName();
}


// Namespace end
}
}
}
