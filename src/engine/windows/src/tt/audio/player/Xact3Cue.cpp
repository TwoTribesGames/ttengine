#include <tt/audio/player/Xact3Cue.h>
#include <tt/audio/player/Xact3Helpers.h>
#include <tt/audio/player/Xact3Player.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace player {

//--------------------------------------------------------------------------------------------------
// Public member functions

Xact3Cue::~Xact3Cue()
{
	if (m_cue != 0)
	{
		m_cue->Destroy();
	}
}


bool Xact3Cue::play()
{
	if (m_cue != 0)
	{
		// Cue already playing, destroy immediately
		m_cue->Destroy();
		m_cue = 0;
	}
	
	m_cue = m_player->prepareCue(m_soundBank, m_cueIndex);
	if (m_cue == 0)
	{
		return false;
	}
	
	applySettings();
	
	HRESULT hr = m_cue->Play();
	if (FAILED(hr))
	{
		// Ignore instance limit warnings
		if (hr != XACTENGINE_E_INSTANCELIMITFAILTOPLAY)
		{
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
			TT_PANIC("Playing XACT cue %u ('%s') failed.\nError: %s (%s)",
			         m_cueIndex, m_cueName.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
#endif
			return false;
		}
	}
	
	return true;
}


bool Xact3Cue::stop(bool p_immediately)
{
	if (m_cue == 0)
	{
		return true;
	}
	
	HRESULT hr = m_cue->Stop(p_immediately ? XACT_FLAG_CUE_STOP_IMMEDIATE : 0);
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
	TT_ASSERTMSG(hr == S_OK,
	             "Stopping XACT cue %u ('%s') failed.\nError: %s (%s)",
	             m_cueIndex, m_cueName.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
#endif
	return hr == S_OK;
}


bool Xact3Cue::pause()
{
	if (m_cue == 0)
	{
		TT_PANIC("Xact3Cue::pause: no internal cue set; call play() first");
		return false;
	}
	
	HRESULT hr = m_cue->Pause(TRUE);
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
	TT_ASSERTMSG(hr == S_OK,
	             "Pausing XACT cue %u ('%s') failed.\nError: %s (%s)",
	             m_cueIndex, m_cueName.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
#endif
	return hr == S_OK;
}


bool Xact3Cue::resume()
{
	if (m_cue == 0)
	{
		TT_PANIC("Xact3Cue::resume: no internal cue set; call play() first");
		return false;
	}
	
	TT_ASSERTMSG(getState() == State_Paused, "Cannot resume cue that is not paused.");
	HRESULT hr = m_cue->Pause(FALSE);
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
	TT_ASSERTMSG(hr == S_OK,
	             "Resuming XACT cue %u ('%s') failed.\nError: %s (%s)",
	             m_cueIndex, m_cueName.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
#endif
	return hr == S_OK;
}


Xact3Cue::State Xact3Cue::getState() const
{
	if (m_cue == 0)
	{
		// Xact3Cue exists, but internal cue doesn't exist; i.e., the cue is created but isn't playing
		return State_Created;
	}
	
	DWORD   cueState = 0;
	HRESULT hr       = m_cue->GetState(&cueState);
	if (hr != S_OK)
	{
		// Retrieving state failed; state is unknown
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
		TT_PANIC("Retrieving state of XACT cue %u ('%s') failed.\nError: %s (%s)",
		         m_cueIndex, m_cueName.c_str(), getXactErrorName(hr), getXactErrorDesc(hr));
#endif
		return State_None;
	}
	
	// Group some XACT cue state bits together into one TT cue state
	const DWORD playingMask = XACT_CUESTATE_PREPARING | XACT_CUESTATE_PREPARED | XACT_CUESTATE_PLAYING;
	const DWORD stoppedMask = XACT_CUESTATE_STOPPING | XACT_CUESTATE_STOPPED;
	
	if ((cueState & XACT_CUESTATE_PAUSED) == XACT_CUESTATE_PAUSED)
	{
		return State_Paused;
	}
	else if ((cueState & XACT_CUESTATE_PREPARED) == XACT_CUESTATE_PREPARED)
	{
		return State_Created;
	}
	else if ((cueState & playingMask) != 0) // Checking for != 0, because any of the bits in the mask may be set
	{
		return State_Playing;
	}
	else if ((cueState & stoppedMask) != 0) // Checking for != 0, because any of the bits in the mask may be set
	{
		return State_Stopped;
	}
	
	// Not in any particular state we're interested in
	return State_None;
}


bool Xact3Cue::setVariable(const std::string& p_name, real p_value)
{
	SoundCue::setVariable(p_name, p_value);
	
	if (m_cue == 0)
	{
		return false;
	}
	
	// NOTE: Could potentially speed this up by caching the "variable name to index" translation
	XACTVARIABLEINDEX variableIndex = m_cue->GetVariableIndex(p_name.c_str());
	if (variableIndex == XACTVARIABLEINDEX_INVALID)
	{
		/* NOTE: Do not complain about non-existent variables: assume calling code knows what it's doing
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
		TT_PANIC("XACT cue %u ('%s') does not have a variable named '%s'.",
		         m_cueIndex, m_cueName.c_str(), p_name.c_str());
#endif
		*/
		return false;
	}
	
	HRESULT hr = m_cue->SetVariable(variableIndex, static_cast<XACTVARIABLEVALUE>(p_value));
	
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
	TT_ASSERTMSG(hr == S_OK,
	             "Setting XACT cue %u ('%s') variable '%s' to value %f failed.\nError: %s (%s)",
	             m_cueIndex, m_cueName.c_str(), p_name.c_str(), p_value,
	             getXactErrorName(hr), getXactErrorDesc(hr));
#endif
	
	return (hr == S_OK);
}


bool Xact3Cue::getVariable(const std::string& p_name, real* p_value_OUT) const
{
	if (m_cue == 0)
	{
		TT_PANIC("Xact3Cue::setVariable: no internal cue set; call play() first");
		return false;
	}
	
	TT_NULL_ASSERT(p_value_OUT);
	
	// NOTE: Could potentially speed this up by caching the "variable name to index" translation
	XACTVARIABLEINDEX variableIndex = m_cue->GetVariableIndex(p_name.c_str());
	if (variableIndex == XACTVARIABLEINDEX_INVALID)
	{
		/* NOTE: Do not complain about non-existent variables: assume calling code knows what it's doing
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
		TT_PANIC("XACT cue %u ('%s') does not have a variable named '%s'.",
		         m_cueIndex, m_cueName.c_str(), p_name.c_str());
#endif
		*/
		return false;
	}
	
	XACTVARIABLEVALUE value;
	HRESULT hr = m_cue->GetVariable(variableIndex, &value);
	
#if !defined(TT_BUILD_FINAL) // cue index and name are not available in final builds
	TT_ASSERTMSG(hr == S_OK,
	             "Getting value of XACT cue %u ('%s') variable '%s' failed.\nError: %s (%s)",
	             m_cueIndex, m_cueName.c_str(), p_name.c_str(),
	             getXactErrorName(hr), getXactErrorDesc(hr));
#endif
	
	if (hr == S_OK)
	{
		*p_value_OUT = static_cast<real>(value);
	}
	
	return (hr == S_OK);
}


bool Xact3Cue::setPosition(const math::Vector3& p_position)
{
	if (isPositional() == false)
	{
		TT_PANIC("Cannot set position on non-positional sound cue.");
		return false;
	}
	
	SoundCue::setPosition(p_position);
	
	if (m_cue == 0)
	{
		return false;
	}
	
	return m_player->setEmitterPosition(m_cue, &m_emitter, p_position);
}


bool Xact3Cue::setRadius(real p_inner, real p_outer)
{
	if (isPositional() == false)
	{
		TT_PANIC("Cannot set radius on non-positional sound cue.");
		return false;
	}
	
	SoundCue::setRadius(p_inner, p_outer);
	
	if (m_cue == 0)
	{
		return false;
	}
	
	return m_player->setEmitterRadius(m_cue, &m_emitter, p_inner, p_outer);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

Xact3Cue::Xact3Cue(Xact3Player* p_player, IXACT3SoundBank* p_soundBank, XACTINDEX p_cueIndex,
                   bool p_positional, const X3DAUDIO_EMITTER& p_defaultEmitter, const std::string& p_cueName)
:
SoundCue(p_positional),
m_player(p_player),
m_soundBank(p_soundBank),
m_cueIndex(p_cueIndex),
m_emitter(p_defaultEmitter),
m_cue(0)
#if !defined(TT_BUILD_FINAL)
,
m_cueName(p_cueName)
#endif
{
	(void)p_cueName;
	TT_NULL_ASSERT(m_player);
	TT_NULL_ASSERT(p_soundBank);
}

// Namespace end
}
}
}
