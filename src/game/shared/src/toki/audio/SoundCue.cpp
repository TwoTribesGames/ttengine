#include <tt/code/bufferutils.h>
#include <tt/code/helpers.h>
#include <tt/math/math.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/audio/SoundCue.h>


namespace toki {
namespace audio {

//--------------------------------------------------------------------------------------------------
// Public member functions

SoundCuePtr SoundCue::create()
{
	SoundCuePtr instance(new SoundCue);
	instance->m_this = instance;
	return instance;
}


SoundCuePtr SoundCue::create(const PlayInfo& p_playInfo)
{
	SoundCuePtr instance(new SoundCue(p_playInfo));
	instance->m_this = instance;
	return instance;
}


void SoundCue::play()
{
	if (m_cue == 0)
	{
		if (m_playInfo.isPositional)
		{
			if (m_playInfo.entityForPositionalSound.isEmpty() == false)
			{
				m_cue = AudioPlayer::getInstance()->playPositionalEffectCue(
						m_playInfo.soundbank,
						m_playInfo.name,
						m_playInfo.entityForPositionalSound);
			}
		}
		else
		{
			m_cue = AudioPlayer::getInstance()->playEffectCue(m_playInfo.soundbank, m_playInfo.name);
		}
	}
}


void SoundCue::stop()
{
	if (m_cue != 0)
	{
		m_cue->stop();
		reset();
	}
}


void SoundCue::pause()
{
	if (m_cue != 0)
	{
		m_cue->pause();
	}
}


void SoundCue::resume()
{
	if (m_cue != 0)
	{
		m_cue->resume();
	}
}


bool SoundCue::isPlaying() const
{
	return (m_cue != 0) ? m_cue->isPlaying() : false;
}


bool SoundCue::isPaused() const
{
	return (m_cue != 0) ? m_cue->isPaused() : false;
}


void SoundCue::setFadingVariable(const std::string& p_name,
                                 real               p_value,
                                 real               p_duration,
                                 bool               p_stopCueAfterFade)
{
	if (m_cue == 0)
	{
		return;
	}
	
	// Find this variable
	FadingVariables::iterator varIt = m_variables.end();
	for (FadingVariables::iterator it = m_variables.begin(); it != m_variables.end(); ++it)
	{
		if ((*it).name == p_name)
		{
			varIt = it;
		}
	}
	
	if (p_duration <= 0.0f)
	{
		// Instantly set variable
		m_cue->setVariable(p_name, p_value);
		
		m_nonFadingVariables[p_name] = p_value;
		
		// Variable instantly set, remove from variables list if present
		if (varIt != m_variables.end())
		{
			m_variables.erase(varIt);
		}
		
		if (p_stopCueAfterFade)
		{
			stop();
		}
		return;
	}
	else
	{
		// Remove from non-fading variables registration
		NonFadingVariables::iterator nonFadingVarIt = m_nonFadingVariables.find(p_name);
		if (nonFadingVarIt != m_nonFadingVariables.end())
		{
			m_nonFadingVariables.erase(nonFadingVarIt);
		}
	}
	
	// Has this variable already been set?
	if (varIt != m_variables.end())
	{
		(*varIt).stopCueAfterFade = p_stopCueAfterFade;
		(*varIt).interpolation.startNewInterpolation(p_value, p_duration);
	}
	else
	{
		real startValue = 0.0f;
		if (m_cue->getVariable(p_name, &startValue))
		{
			FadingVariable fadingVar;
			fadingVar.name             = p_name;
			fadingVar.interpolation    = TLI(startValue, p_value, p_duration);
			fadingVar.stopCueAfterFade = p_stopCueAfterFade;
			m_variables.push_back(fadingVar);
			AudioPlayer::getInstance()->registerSoundCueForUpdate(m_this);
		}
	}
}


bool SoundCue::setRadius(real p_inner, real p_outer)
{
	if (m_cue == 0)
	{
		return false;
	}
	
	m_emitterConeInnerRadius = p_inner;
	m_emitterConeOuterRadius = p_outer;
	
	return m_cue->setRadius(p_inner, p_outer);
}


void SoundCue::setReverbVolume(real p_normalizedVolume)
{
	if (m_cue == 0)
	{
		return;
	}
	
	tt::math::clamp(p_normalizedVolume, 0.0f, 1.0f);
	m_reverbNormalizedVolume = p_normalizedVolume;
	m_cue->setReverbVolume(m_reverbNormalizedVolume);
}


void SoundCue::update(real p_deltaTime)
{
	if (m_cue != 0 && m_cue->isPlaying())
	{
		for (FadingVariables::iterator it = m_variables.begin(); it != m_variables.end();)
		{
			TLI& interpolation = (*it).interpolation;
			if (interpolation.isDone() == false)
			{
				interpolation.update(p_deltaTime);
				m_cue->setVariable((*it).name, interpolation.getValue());
				++it;
			}
			else
			{
				if ((*it).stopCueAfterFade)
				{
					stop();
					break;
				}
				
				// Fading variable finished, remove from update cue
				it = m_variables.erase(it);
			}
		}
	}
}


void SoundCue::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	
	bool cueValid = (m_cue != 0);
	bu::put(cueValid, p_context);
	
	if (cueValid == false)
	{
		// Nothing more to save
		return;
	}
	
	std::string cueStateName;
	if (m_cue != 0)
	{
		using tt::audio::player::SoundCue;
		const SoundCue::State cueState = m_cue->getState();
		switch (cueState)
		{
		case SoundCue::State_Playing:
			cueStateName = "playing";
			break;
			
		case SoundCue::State_Created:
			cueStateName = "created";
			break;
			
		case SoundCue::State_None:
			TT_WARN("SoundCue has state 'none'. Assuming 'stopped'.");
		case SoundCue::State_Stopped:
			cueStateName = "stopped";
			break;
		
		case SoundCue::State_Paused:
			cueStateName = "paused";
			break;
			
		default:
			TT_PANIC("SoundCue is in unsupported state %d.", cueState);
			cueStateName = "playing";
			break;
		}
	}
	bu::put(cueStateName, p_context);
	
	bu::put      (m_playInfo.soundbank,                p_context);
	bu::put      (m_playInfo.name,                     p_context);
	bu::put      (m_playInfo.isPositional,             p_context);
	bu::putHandle(m_playInfo.entityForPositionalSound, p_context);
	bu::put      (m_emitterConeInnerRadius,            p_context);
	bu::put      (m_emitterConeOuterRadius,            p_context);
	bu::put      (m_reverbNormalizedVolume,            p_context);
	
	bu::put(static_cast<u32>(m_variables.size()), p_context);
	for (FadingVariables::const_iterator it = m_variables.begin();
	     it != m_variables.end(); ++it)
	{
		bu::put   ((*it).name,             p_context);
		bu::putTLI((*it).interpolation,    p_context);
		bu::put   ((*it).stopCueAfterFade, p_context);
	}
	
	bu::put(static_cast<u32>(m_nonFadingVariables.size()), p_context);
	for (NonFadingVariables::const_iterator it = m_nonFadingVariables.begin();
	     it != m_nonFadingVariables.end(); ++it)
	{
		bu::put((*it).first,  p_context);
		bu::put((*it).second, p_context);
	}
}


void SoundCue::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	// Start with defaults
	reset();
	
	const bool cueValid = bu::get<bool>(p_context);
	
	if (cueValid == false)
	{
		// Nothing more to load
		return;
	}
	
	const std::string cueStateName = bu::get<std::string>(p_context);
	
	m_playInfo.soundbank                = bu::get      <std::string         >(p_context);
	m_playInfo.name                     = bu::get      <std::string         >(p_context);
	m_playInfo.isPositional             = bu::get      <bool                >(p_context);
	m_playInfo.entityForPositionalSound = bu::getHandle<game::entity::Entity>(p_context);
	m_emitterConeInnerRadius            = bu::get      <real                >(p_context);
	m_emitterConeOuterRadius            = bu::get      <real                >(p_context);
	m_reverbNormalizedVolume            = bu::get      <real                >(p_context);
	
	const u32 variableCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < variableCount; ++i)
	{
		FadingVariable fadingVar;
		fadingVar.name             = bu::get   <std::string>(p_context);
		fadingVar.interpolation    = bu::getTLI<real       >(p_context);
		fadingVar.stopCueAfterFade = bu::get   <bool       >(p_context);
		m_variables.push_back(fadingVar);
	}
	
	const u32 nonFadingVariableCount = bu::get<u32>(p_context);
	for (u32 i = 0; i < nonFadingVariableCount; ++i)
	{
		const std::string varName  = bu::get<std::string>(p_context);
		const real        varValue = bu::get<real       >(p_context);
		m_nonFadingVariables[varName] = varValue;
	}
	
	if (m_variables.empty() == false)
	{
		AudioPlayer::getInstance()->registerSoundCueForUpdate(m_this);
	}
	
	// Restart the sound effect based on the information loaded
	play();
	
	// Restore the cue to the state it was in when saved
	if (m_cue != 0)
	{
		for (NonFadingVariables::iterator it = m_nonFadingVariables.begin();
		     it != m_nonFadingVariables.end(); ++it)
		{
			m_cue->setVariable((*it).first, (*it).second);
		}
		
		for (FadingVariables::iterator it = m_variables.begin();
		     it != m_variables.end(); ++it)
		{
			m_cue->setVariable((*it).name, (*it).interpolation.getValue());
		}
		
		if (tt::math::realGreaterEqual(m_emitterConeInnerRadius, 0.0f) &&
		    tt::math::realGreaterEqual(m_emitterConeOuterRadius, 0.0f))
		{
			setRadius(m_emitterConeInnerRadius, m_emitterConeOuterRadius);
		}
		
		if (tt::math::realGreaterEqual(m_reverbNormalizedVolume, 0.0f))
		{
			setReverbVolume(m_reverbNormalizedVolume);
		}
		
		if (cueStateName == "playing" || cueStateName == "created")
		{
			// Cue should already be playing at this point, so nothing to do here
		}
		else if (cueStateName == "stopped")
		{
			m_cue->stop();
		}
		else if (cueStateName == "paused")
		{
			m_cue->pause();
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

SoundCue::SoundCue()
:
m_cue(),
m_variables(),
m_nonFadingVariables(),
m_playInfo(),
m_emitterConeInnerRadius(-1.0f),
m_emitterConeOuterRadius(-1.0f),
m_reverbNormalizedVolume(-1.0f),
m_this()
{
	m_variables.reserve(reserveCount);
}


SoundCue::SoundCue(const PlayInfo& p_playInfo)
:
m_cue(),
m_variables(),
m_nonFadingVariables(),
m_playInfo(p_playInfo),
m_emitterConeInnerRadius(-1.0f),
m_emitterConeOuterRadius(-1.0f),
m_reverbNormalizedVolume(-1.0f),
m_this()
{
	m_variables.reserve(reserveCount);
	play();
}


void SoundCue::reset()
{
	m_playInfo = PlayInfo();
	m_cue.reset();
	tt::code::helpers::freeContainer(m_variables);
	tt::code::helpers::freeContainer(m_nonFadingVariables);
	
	m_emitterConeInnerRadius = -1.0f;
	m_emitterConeOuterRadius = -1.0f;
	m_reverbNormalizedVolume = -1.0f;
	
	m_variables.reserve(reserveCount);
}

// Namespace end
}
}
