#include <tt/audio/player/SoundCue.h>
#include <tt/audio/player/SoundCueSettings.h>


namespace tt {
namespace audio {
namespace player {

// -----------------------------------------------------------------------------
// Public functions

void SoundCueSettings::applyToSoundCue(SoundCue& p_soundCue)
{
	if (m_updateBitmask.checkFlag(UpdateFlag_ReverbVolume)) p_soundCue.setReverbVolume(m_reverbVolume);
	if (m_updateBitmask.checkFlag(UpdateFlag_Position))     p_soundCue.setPosition(m_position);
	if (m_updateBitmask.checkFlag(UpdateFlag_Radius))       p_soundCue.setRadius(m_innerRadius, m_outerRadius);
	
	for (Variables::const_iterator it = m_variables.begin();
	     it != m_variables.end(); ++it)
	{
		p_soundCue.setVariable((*it).first, (*it).second);
	}
}

// namespace end
}
}
}
