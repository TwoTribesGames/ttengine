#include <tt/audio/xact/Category.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/RuntimeParameterControl.h>
#include <tt/audio/xact/Sound.h>
#include <tt/audio/xact/SoundInstance.h>
#include <tt/audio/xact/TrackInstance.h>
#include <tt/code/helpers.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


//#define SOUND_DEBUG
#ifdef SOUND_DEBUG
	#define Sound_Printf TT_Printf
#else
	#define Sound_Printf(...)
#endif


//#define SOUND_TRACE
#ifdef SOUND_TRACE
	#define Sound_Trace TT_Printf
#else
	#define Sound_Trace(...)
#endif


#define SOUND_WARN
#ifdef SOUND_WARN
	#define Sound_Warn TT_Printf
#else
	#define Sound_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

//--------------------------------------------------------------------------------------------------
// Public member functions

SoundInstance::SoundInstance(Sound* p_sound, CueInstance* p_cue)
:
m_sound(p_sound),
m_cue(p_cue),
m_tracks(),
m_volumeInDB(0.0f),
m_reverbVolumeInDB(-96.0f),
m_rpcVolumeInDB(0.0f)
{
	Sound_Trace("SoundInstance::SoundInstance: %p, %p\n", p_sound, p_cue);
	
	TT_ASSERTMSG(m_sound != 0, "SoundInstance::SoundInstance: sound must not be 0");
	TT_ASSERTMSG(m_cue   != 0, "SoundInstance::SoundInstance: cue must not be 0");
	
	// Get RPC values
	const RPCs& rpcs = m_sound->getRPCs();
	for(RPCs::const_iterator it = rpcs.begin(); it != rpcs.end(); ++it)
	{
		// Get all curves
		const RPCCurves& curves = (*it)->getCurves();
		for(RPCCurves::const_iterator iter = curves.begin(); iter != curves.end(); ++iter)
		{
			m_paramValues.push_back(RPCValue((*iter), (*iter)->getFirstPoint().x));
		}
	}
	
	m_volumeInDB       = m_sound->getVolume();
	m_reverbVolumeInDB = m_sound->getCategory()->getReverbVolume();
}


SoundInstance::~SoundInstance()
{
	Sound_Trace("SoundInstance::~SoundInstance\n");
	
	code::helpers::freePointerContainer(m_tracks);
}


void SoundInstance::addTrack(TrackInstance* p_track)
{
	Sound_Trace("SoundInstance::addTrack: %p\n", p_track);
	
	if (p_track == 0)
	{
		Sound_Warn("SoundInstance::addTrack: track must not be 0\n");
		return;
	}
	
	m_tracks.push_back(p_track);
}


real SoundInstance::getVolume() const
{
	Sound_Trace("SoundInstance::getVolume\n");
	
	return m_volumeInDB;
}


real SoundInstance::getCategoryVolume() const
{
	return m_sound->getCategory()->getVolume();
}


bool SoundInstance::belongsToCategory(const Category* p_category) const
{
	return p_category == m_sound->getCategory();
}


bool SoundInstance::play()
{
	Sound_Trace("SoundInstance::play\n");
	
	bool success = true;
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ( (*it)->play() == false )
		{
			success = false;
		}
		else
		{
			(*it)->setPitch(m_sound->getPitch());
		}
	}
	
	return success;
}


bool SoundInstance::stop()
{
	Sound_Trace("SoundInstance::stop\n");
	
	bool success = true;
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ( (*it)->stop() == false )
		{
			success = false;
		}
	}
	
	return success;
}


bool SoundInstance::pause()
{
	Sound_Trace("SoundInstance::pause\n");
	
	bool success = true;
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ( (*it)->pause() == false )
		{
			success = false;
		}
	}
	
	return success;
}


bool SoundInstance::resume()
{
	Sound_Trace("SoundInstance::resume\n");
	
	bool success = true;
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ( (*it)->resume() == false )
		{
			success = false;
		}
	}
	
	return success;
}


bool SoundInstance::stopCue()
{
	Sound_Trace("SoundInstance::stopCue\n");
	
	return m_cue->stop();
}


void SoundInstance::update(real p_time)
{
	Sound_Trace("SoundInstance::update: %f\n", realToFloat(p_time));
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		(*it)->update(p_time);
	}
}


void SoundInstance::updateVolume()
{
	Sound_Trace("SoundInstance::updateVolume:\n");
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		(*it)->updateVolume();
	}
}


bool SoundInstance::setPosition(const math::Vector3& p_position)
{
	Sound_Trace("SoundInstance::setPosition: <%f, %f, %f>\n",
		realToFloat(p_position.x), realToFloat(p_position.y), realToFloat(p_position.z));
	
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if((*it)->setPosition(p_position) == false)
		{
			return false;
		}
	}
	return true;
}


bool SoundInstance::setEmitterRadius(real p_inner, real p_outer)
{
	Sound_Trace("SoundInstance::setEmitterRadius: Inner: %f . Outer: %f\n",
	            realToFloat(p_inner), realToFloat(p_outer));
	
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ((*it)->setEmitterRadius(p_inner, p_outer) == false)
		{
			return false;
		}
	}
	
	return true;
}


void SoundInstance::setReverbVolume(real p_volumeInDB)
{
	if (p_volumeInDB > 6.0f)
	{
		//Sound_Warn("SoundInstance::setReverbVolume: Volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		//Sound_Warn("SoundInstance::setReverbVolume: Volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	m_reverbVolumeInDB = p_volumeInDB;
	
	// Flush the new volume to any active audio
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		(*it)->updateReverbVolume();
	}
}


bool SoundInstance::setVariable(const std::string& p_name, real p_value)
{
	// Find and set variable in the Sound RPCs
	bool foundVariable(false);
	
	for(ParameterValues::iterator it = m_paramValues.begin(); it != m_paramValues.end(); ++it)
	{
		if(it->first->getVariable() == getSoundVariableFromName(p_name))
		{
			Sound_Trace("Setting variable '%s' to value %g\n", p_name.c_str(), p_value);
			foundVariable = true;
			
			it->second = p_value;
			
			if(it->first->getVariable() == SoundVariable_Volume)
			{
				// RPC Curve has values in millibells (-9600 - 600)
				m_rpcVolumeInDB = it->first->getParameterValue(it->second) / 100.0f;
				updateTrackVolume();
			}
			break;
		}
	}

	return foundVariable;
}


bool SoundInstance::getVariable(const std::string& p_name, real* p_value_OUT)
{
	for(ParameterValues::iterator it = m_paramValues.begin(); it != m_paramValues.end(); ++it)
	{
		if(it->first->getVariable() == getSoundVariableFromName(p_name))
		{
			Sound_Trace("Getting variable '%s' has value %g\n", p_name.c_str(), it->second);
			*p_value_OUT = it->second;
			return true;
		}
	}
	
	return false;
}


bool SoundInstance::isDone() const
{
	Sound_Trace("SoundInstance::isDone\n");
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		if ((*it)->isDone() == false)
		{
			return false;
		}
	}
	return true;
}


snd::size_type SoundInstance::getPriority() const
{
	if(m_sound != 0)
	{
		return m_sound->getPriority();
	}
	return 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SoundInstance::updateTrackVolume()
{
	real soundVolume = m_volumeInDB + m_rpcVolumeInDB;
	
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		(*it)->setVolume(soundVolume);
	}
}

// Namespace end
}
}
}
