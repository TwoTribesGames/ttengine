#include <tt/pres/PresentationObject.h>

#include <toki/audio/AudioPlayer.h>
#include <toki/pres/AudioTrigger.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

AudioTrigger::AudioTrigger(const tt::pres::TriggerInfo&           p_triggerInfo,
                           const tt::pres::PresentationObjectPtr& p_object)
:
TriggerBase(p_triggerInfo),
m_soundName(p_triggerInfo.data),
m_positional(p_triggerInfo.type != "global_audio"),
m_soundCue(),
m_presentationObject(p_object)
{
	TT_ASSERT(p_triggerInfo.type == "audio" || p_triggerInfo.type == "global_audio");
}


void AudioTrigger::trigger()
{
	const std::string soundbank("Effects");
	
	if (audio::AudioPlayer::hasInstance() == false)
	{
		return;
	}
	
	tt::pres::PresentationObjectPtr presObj = m_presentationObject.lock();
	if (presObj == 0)
	{
		return;
	}
	
	audio::AudioPlayer* player = audio::AudioPlayer::getInstance();
	const bool looping(player->isCueLooping(soundbank, m_soundName));
	
	// Start the sound effect
	if (looping)
	{
		if (m_soundCue == 0)
		{
			// FIXME: This may not be functionally identical to the way it worked in Swap
			//        (which has a custom "looping sound" playback implementation)
			if (m_positional)
			{
				m_soundCue = player->playPositionalEffectCue(soundbank, m_soundName, presObj);
			}
			else
			{
				m_soundCue = player->playEffectCue(soundbank, m_soundName);
			}
		}
	}
	else
	{
		if (m_positional)
		{
			player->playPositionalEffectCue(soundbank, m_soundName, presObj);
		}
		else
		{
			player->playEffect(soundbank, m_soundName);
		}
	}
}


AudioTrigger* AudioTrigger::clone() const
{
	return new AudioTrigger(*this);
}


void AudioTrigger::presentationEnded()
{
	if (m_soundCue != 0)
	{
		m_soundCue->stop();
		m_soundCue.reset();
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

AudioTrigger::AudioTrigger(const AudioTrigger& p_rhs)
:
tt::pres::TriggerBase(p_rhs),
m_soundName(p_rhs.m_soundName),
m_positional(p_rhs.m_positional),
m_soundCue(p_rhs.m_soundCue),
m_presentationObject() // NOTE: m_presentationObject is not copied. The copy needs the reference of the newly copied presentationObject.
{
}

// Namespace end
}
}
