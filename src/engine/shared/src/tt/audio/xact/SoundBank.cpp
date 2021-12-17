#include <tt/audio/xact/Cue.h>
#include <tt/audio/xact/Sound.h>
#include <tt/audio/xact/SoundBank.h>
#include <tt/code/helpers.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>
#include <tt/xml/XmlNode.h>


//#define SOUND_DEBUG
#ifdef SOUND_DEBUG
	#define Sound_Printf TT_Printf
#else
	#define Sound_Printf(...)
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

SoundBank::SoundBank(int p_soundBankIndex)
:
m_sounds(),
m_cueIndices(),
m_cues(),
m_soundBankIndex(p_soundBankIndex)
{
}


SoundBank::~SoundBank()
{
	code::helpers::freePairSecondContainer(m_cues);
	code::helpers::freePairSecondContainer(m_sounds);
}


void SoundBank::addCue(Cue* p_cue, const std::string& p_name)
{
	if (p_cue == 0)
	{
		Sound_Warn("SoundBank::addCue: cue must not be 0\n");
		return;
	}
	
	int cueindex = static_cast<int>(m_cues.size());
	m_cues.insert(Cues::value_type(cueindex, p_cue));
	m_cueIndices.insert(CueIndices::value_type(p_name, cueindex));
}


void SoundBank::addSound(Sound* p_sound)
{
	if (p_sound == 0)
	{
		Sound_Warn("SoundBank::addSound: sound must not be 0\n");
		return;
	}
	
	int soundindex = static_cast<int>(m_sounds.size());
	m_sounds.insert(Sounds::value_type(soundindex, p_sound));
	
	// set the soundbankindex/soundindex in the wave itself as well, used for loading and saving
	p_sound->setSoundIndex(soundindex);
	
	TT_ASSERT(m_soundBankIndex >= 0);
	p_sound->setSoundBankIndex(m_soundBankIndex);
}


int SoundBank::getCueIndex(const std::string& p_name) const
{
	CueIndices::const_iterator it = m_cueIndices.find(p_name);
	if (it == m_cueIndices.end())
	{
		return -1;
	}
	return (*it).second;
}


Sound* SoundBank::getSound(int p_index)
{
	Sounds::iterator it = m_sounds.find(p_index);
	return (it == m_sounds.end()) ? 0 : (*it).second;
}


Cue::ErrorStatus SoundBank::createCue(int p_cueIndex, CueInstancePtr& p_result_OUT)
{
	Cues::iterator it = m_cues.find(p_cueIndex);
	if (it == m_cues.end())
	{
		p_result_OUT.reset();
		return Cue::ErrorStatus_InvalidCue;
	}
	return (*it).second->instantiate(p_result_OUT);
}


bool SoundBank::play(const std::string& p_name)
{
	CueIndices::iterator it = m_cueIndices.find(p_name);
	if (it == m_cueIndices.end())
	{
		TT_Printf("Cannot find '%s'\n", p_name.c_str());
		return false;
	}
	
	return play((*it).second);
}


bool SoundBank::play(int p_cueIndex)
{
	Cues::iterator it = m_cues.find(p_cueIndex);
	if (it == m_cues.end())
	{
		return false;
	}
	
	return (*it).second->play();
}


bool SoundBank::stop(const std::string& p_name)
{
	CueIndices::iterator it = m_cueIndices.find(p_name);
	if (it == m_cueIndices.end())
	{
		return false;
	}
	
	return stop((*it).second);
}


bool SoundBank::stop(int p_cueIndex)
{
	Cues::iterator it = m_cues.find(p_cueIndex);
	if (it == m_cues.end())
	{
		return false;
	}
	
	return (*it).second->stop();
}


bool SoundBank::stop()
{
	bool success = true;
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		if ((*it).second->stop() == false)
		{
			success = false;
		}
	}
	return success;
}


bool SoundBank::pause(int p_cueIndex)
{
	Cues::iterator it = m_cues.find(p_cueIndex);
	if (it == m_cues.end())
	{
		return false;
	}
	
	return (*it).second->pause();
}


bool SoundBank::pause(const std::string& p_name)
{
	CueIndices::iterator it = m_cueIndices.find(p_name);
	if (it == m_cueIndices.end())
	{
		TT_Printf("Cannot find '%s'\n", p_name.c_str());
		return false;
	}
	
	return pause((*it).second);
}


bool SoundBank::resume(int p_cueIndex)
{
	Cues::iterator it = m_cues.find(p_cueIndex);
	if (it == m_cues.end())
	{
		return false;
	}
	
	return (*it).second->resume();
}


bool SoundBank::resume(const std::string& p_name)
{
	CueIndices::iterator it = m_cueIndices.find(p_name);
	if (it == m_cueIndices.end())
	{
		TT_Printf("Cannot find '%s'\n", p_name.c_str());
		return false;
	}
	
	return resume((*it).second);
}


bool SoundBank::pauseCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	bool success = true;
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		if ((*it).second->pauseCategory(p_cat) == false)
		{
			success = false;
		}
	}
	return success;
}


bool SoundBank::resumeCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	bool success = true;
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		if ((*it).second->resumeCategory(p_cat) == false)
		{
			success = false;
		}
	}
	return success;
}


bool SoundBank::stopCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	bool success = true;
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		if ((*it).second->stopCategory(p_cat) == false)
		{
			success = false;
		}
	}
	return success;
}


void SoundBank::setReverbVolumeForCategory(Category* p_category, real p_volumeInDB)
{
	TT_NULL_ASSERT(p_category);
	if (p_category == 0) return;
	
	// Set the new volume for any active audio
	// (any audio started after the Category volume change will simply inherit that volume)
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		(*it).second->setReverbVolumeForCategory(p_category, p_volumeInDB);
	}
}


SoundBank* SoundBank::createSoundBank(int p_soundBankIndex, const xml::XmlNode* p_node)
{
	if (p_node == 0)
	{
		Sound_Warn("SoundBank::createSoundBank: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Sound Bank")
	{
		Sound_Warn("SoundBank::createSoundBank: xml node '%s' is not a sound bank node\n", p_node->getName().c_str());
		return 0;
	}
	
	SoundBank* bank = new SoundBank(p_soundBankIndex);
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Sound")
		{
			Sound* sound = Sound::createSound(child);
			
			if (sound == 0)
			{
				Sound_Warn("SoundBank::createSoundBank: Error creating sound\n");
			}
			else
			{
				bank->addSound(sound);
			}
		}
		else if (child->getName() == "Cue")
		{
			Cue* cue = Cue::createCue(child, bank);
			
			if (cue == 0)
			{
				Sound_Warn("SoundBank::createSoundBank: Error creating cue\n");
			}
			else
			{
				bank->addCue(cue, child->getAttribute("Name"));
			}
		}
		else
		{
			Sound_Warn("SoundBank::createSoundBank: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return bank;
}


void SoundBank::update(real p_delta)
{
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		(*it).second->update(p_delta);
	}
}


void SoundBank::updateVolume()
{
	for (Cues::iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		(*it).second->updateVolume();
	}
}


// Private

bool SoundBank::load(const fs::FilePtr& p_file)
{
	int size;
	int index;
	std::string name;
	
	// read all sounds (do this first, as cues depend on it!)
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		Sound* sound = new Sound;
		sound->load(p_file);
		
		addSound(sound);
	}
	
	// read all cue indices
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read name
		fs::readNarrowString(p_file, &name);
		
		// read index
		fs::readInteger(p_file, &index);
		
		m_cueIndices.insert(CueIndices::value_type(name, index));
	}
	
	// read all cues
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read index
		fs::readInteger(p_file, &index);
		
		// read cue
		Cue* cue = new Cue;
		cue->load(p_file);

#ifndef TT_BUILD_FINAL
		// Store name for debugging
		for(CueIndices::iterator it = m_cueIndices.begin(); it != m_cueIndices.end(); ++it)
		{
			if(it->second == index)
			{
				cue->m_name = it->first;
			}
		}
#endif
		
		m_cues.insert(Cues::value_type(index, cue));
	}
	
	return true;
}


bool SoundBank::save(const fs::FilePtr& p_file) const
{
	// save all sounds (do this first, as cues depend on it!)
	fs::writeInteger(p_file, m_sounds.size());
	for (Sounds::const_iterator it = m_sounds.begin(); it != m_sounds.end(); ++it)
	{
		// verify that all sounds in this bank have the correct sound bank set
		TT_ASSERT((*it).second->getSoundIndex() == (*it).first);
		
		// save sound
		(*it).second->save(p_file);
	}
	
	// save all cue indices
	fs::writeInteger(p_file, m_cueIndices.size());
	for (CueIndices::const_iterator it = m_cueIndices.begin(); it != m_cueIndices.end(); ++it)
	{
		// write name
		fs::writeNarrowString(p_file, (*it).first);
		
		// write index
		fs::writeInteger(p_file, (*it).second);
	}
	
	// save all cues
	fs::writeInteger(p_file, m_cues.size());
	for (Cues::const_iterator it = m_cues.begin(); it != m_cues.end(); ++it)
	{
		// save index
		fs::writeInteger(p_file, (*it).first);
		
		// save cue
		(*it).second->save(p_file);
	}
	
	return true;
}

// Namespace end
}
}
}
