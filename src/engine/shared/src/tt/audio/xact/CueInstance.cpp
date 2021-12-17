#include <algorithm>

#include <tt/audio/xact/Cue.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/Sound.h>
#include <tt/audio/xact/SoundInstance.h>
#include <tt/math/Random.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


//#define CUE_DEBUG
#ifdef CUE_DEBUG
	#define Cue_Printf TT_Printf
#else
	#define Cue_Printf(...)
#endif


//#define CUE_TRACE
#ifdef CUE_TRACE
	#define Cue_Trace TT_Printf
#else
	#define Cue_Trace(...)
#endif


#define CUE_WARN
#ifdef CUE_WARN
	#define Cue_Warn TT_Printf
#else
	#define Cue_Warn(...)
#endif

namespace tt {
namespace audio {
namespace xact {


CueInstance::CueInstance(Cue* p_cue, Sound* p_sound)
:
m_cue(p_cue),
m_sound(0),
m_time(0.0f),
m_paused(false)
{
	Cue_Trace("CueInstance::CueInstance: %p, %p\n", p_cue, p_sound);
	
	TT_ASSERTMSG(m_cue   != 0, "CueInstance::CueInstance: cue must not be 0");
	TT_ASSERTMSG(p_sound != 0, "CueInstance::CueInstance: sound must not be 0");
	
	m_sound = p_sound->instantiate(this);
	
	TT_ASSERTMSG(m_sound != 0, "CueInstance::CueInstance: unable to instantiate sound");
}


CueInstance::~CueInstance()
{
	Cue_Trace("CueInstance::~CueInstance\n");
	delete m_sound;
}


bool CueInstance::play()
{
	Cue_Trace("CueInstance::play\n");
	m_time = 0.0f;
	return m_sound->play();
}


bool CueInstance::stop()
{
	Cue_Trace("CueInstance::stop\n");
	return m_sound->stop();
}


bool CueInstance::pause()
{
	Cue_Trace("CueInstance::pause\n");
	m_paused = true;
	return m_sound->pause();
}


bool CueInstance::resume()
{
	Cue_Trace("CueInstance::resume\n");
	m_paused = false;
	return m_sound->resume();
}


void CueInstance::update(real p_time)
{
	Cue_Trace("CueInstance::update: %f\n", realToFloat(p_time));
	if (m_paused)
	{
		return;
	}
	m_time += p_time;
	m_sound->update(m_time);
}


void CueInstance::updateVolume()
{
	Cue_Trace("CueInstance::updateVolume\n");
	m_sound->updateVolume();
}


bool CueInstance::belongsToCategory(const Category* p_category) const
{
	return m_sound->belongsToCategory(p_category);
}


bool CueInstance::setPosition(const math::Vector3& p_position)
{
	return m_sound->setPosition(p_position);
}


bool CueInstance::setEmitterRadius(real p_inner, real p_outer)
{
	return m_sound->setEmitterRadius(p_inner, p_outer);
}


void CueInstance::setReverbVolume(real p_volumeInDB)
{
	m_sound->setReverbVolume(p_volumeInDB);
}


bool CueInstance::setVariable(const std::string& p_name, real p_value)
{
	return m_sound->setVariable(p_name, p_value);
}


bool CueInstance::getVariable(const std::string& p_name, real* p_value_OUT)
{
	return m_sound->getVariable(p_name, p_value_OUT);
}


bool CueInstance::isDone() const
{
	Cue_Trace("CueInstance::isDone\n");
	return m_sound->isDone();
}

// Namespace end
}
}
}
