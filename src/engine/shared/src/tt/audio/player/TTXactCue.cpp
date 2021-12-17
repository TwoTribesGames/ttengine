#include <tt/audio/player/TTXactCue.h>
#include <tt/audio/xact/Cue.h>
#include <tt/audio/xact/SoundBank.h>
#include <tt/audio/helpers.h>
#include <tt/platform/tt_error.h>
#include <tt/snd/snd.h> // for gs_xactWavesVolPanPitchManagedByCode
#include <tt/math/Vector3.h>


namespace tt {
namespace audio {
namespace player {


//--------------------------------------------------------------------------------------------------
// Public member functions

TTXactCue::~TTXactCue()
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr != 0)
	{
		ptr->stop();
	}
}


bool TTXactCue::play()
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr != 0)
	{
		// Still having a cueinstance. Unlike Xact3Cue::play(), we cannot destroy this cueinstance
		// since we don't have its ownership. Therefore we cannot play this sample.
		return false;
	}
	
	// create cueInstance and TTXactCue
	xact::CueInstancePtr cue;
	xact::Cue::ErrorStatus result = m_soundBank->createCue(m_cueIndex, cue);
	if (result != xact::Cue::ErrorStatus_OK)
	{
		if (result != xact::Cue::ErrorStatus_InstanceLimited)
		{
#ifndef TT_BUILD_FINAL
			TT_PANIC("Creating XACT cue %u ('%s') failed.", m_cueIndex, m_cueName.c_str());
#endif
		}
		return true;
	}
	
	m_cue = cue;
	
	applySettings();
	
	m_state = State_Playing;
	
	return cue->play();
}


bool TTXactCue::stop(bool p_immediately)
{
	(void)p_immediately;
	
	m_state = State_Stopped;
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr != 0)
	{
		return ptr->stop();
	}
	return true;
}


bool TTXactCue::pause()
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		TT_PANIC("TTXactCue::pause: no internal cue set; call play() first");
		return false;
	}
	m_state = State_Paused;
	return ptr->pause();
}


bool TTXactCue::resume()
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		TT_PANIC("TTXactCue::resume: no internal cue set; call play() first");
		return false;
	}
	m_state = State_Playing;
	return ptr->resume();
}


bool TTXactCue::setVariable(const std::string& p_name, real p_value)
{
	SoundCue::setVariable(p_name, p_value);
	
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return false;
	}
	return ptr->setVariable(p_name, p_value);
}


bool TTXactCue::getVariable(const std::string& p_name, real* p_value_OUT) const
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return false;
	}
	return ptr->getVariable(p_name, p_value_OUT);
}


bool TTXactCue::setPosition(const math::Vector3& p_position)
{
	if (isPositional() == false)
	{
		TT_PANIC("Cannot set position on non-positional sound cue.");
		return false;
	}
	
	SoundCue::setPosition(p_position);
	
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return false;
	}
	return ptr->setPosition(p_position);
}


bool TTXactCue::setRadius(real p_inner, real p_outer)
{
	if (isPositional() == false)
	{
		TT_PANIC("Cannot set radius on non-positional sound cue.");
		return false;
	}
	
	SoundCue::setRadius(p_inner, p_outer);
	
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return false;
	}
	return ptr->setEmitterRadius(p_inner, p_outer);
}


TTXactCue::State TTXactCue::getState() const
{
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return (m_state == State_Created) ? State_Created : State_None;
	}
	
	// FIXME: Need a way to set the state from 'State_Playing' when the cue is done
	//        Cannot do it here because this function is const
	return ptr->isDone() ? State_Stopped : m_state;
}


bool TTXactCue::setReverbVolume(real p_normalizedVolume)
{
	SoundCue::setReverbVolume(p_normalizedVolume);
	
	xact::CueInstancePtr ptr(m_cue.lock());
	if (ptr == 0)
	{
		return false;
	}
	
	ptr->setReverbVolume(helpers::ratioTodB(p_normalizedVolume));
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

TTXactCue::TTXactCue(xact::SoundBank* p_soundBank, s32 p_cueIndex,
                     bool p_positional, const std::string& p_cueName)
:
SoundCue(p_positional),
m_soundBank(p_soundBank),
m_cueIndex(p_cueIndex),
m_state(State_Created),
m_cue()
#if !defined(TT_BUILD_FINAL)
,
m_cueName(p_cueName)
#endif
{
	(void)p_cueName;
	
	TT_NULL_ASSERT(p_soundBank);
}

// Namespace end
}
}
}
