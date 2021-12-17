#include <algorithm>

#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Cue.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/Sound.h>
#include <tt/audio/xact/SoundBank.h>
#include <tt/audio/xact/utils.h>
#include <tt/code/ErrorStatus.h>
#include <tt/fs/File.h>
#include <tt/math/Random.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>
//#include <tt/system/Time.h>
#include <tt/xml/XmlNode.h>


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

Cue::Cue()
:
m_playList(),
m_playOrder(Variation_Ordered),
m_prevSound(0),
m_instances(),
m_variation(Variation_Ordered),
m_limitBehavior(LimitBehavior_None),
m_limit(0)
{
	Cue_Trace("Cue::Cue\n");
}


Cue::~Cue()
{
	Cue_Trace("Cue::~Cue\n");
}


void Cue::addSound(Sound* p_sound, int p_weight)
{
	Cue_Trace("Cue::addSound: %p %d\n", p_sound, p_weight);
	
	if (p_sound == 0)
	{
		Cue_Warn("Cue::addSound: sound must not be 0\n");
		return;
	}
	
	if (p_weight > 255)
	{
		Cue_Warn("Cue::addSound: weight %d is larger than 255; clamping\n",
		         p_weight);
		p_weight = 255;
	}
	
	if (p_weight < 0)
	{
		Cue_Warn("Cue::addSound: weight %d is less than 0; clamping\n",
		         p_weight);
		p_weight = 0;
	}
	
	m_playList.push_back(PlayListEntry(p_sound, p_weight));
}


void Cue::setVariation(VariationType p_type)
{
	Cue_Trace("Cue::setVariation: %d\n", p_type);
	
	m_variation = p_type;
}


void Cue::setLimitBehavior(LimitBehavior p_behavior)
{
	Cue_Trace("Cue::setLimitBehavior: %d\n", p_behavior);
	
	if (p_behavior == LimitBehavior_Queue)
	{
		Cue_Warn("Cue::setLimitBehavior: Queue not supported.\n");
		p_behavior = LimitBehavior_FailToPlay;
	}
	
	m_limitBehavior = p_behavior;
}


void Cue::setInstanceLimit(int p_limit)
{
	if (p_limit <= 0)
	{
		Cue_Warn("Cue::setInstanceLimit: %d is too small (must be 1 or larger).\n",
		         p_limit);
		p_limit = 1;
	}
	
	m_limit = p_limit;
}


Cue* Cue::createCue(const xml::XmlNode* p_node, SoundBank* p_soundBank)
{
	Cue_Trace("Cue::createCue: %p\n", p_node);
	TT_ERR_CREATE("createCue");
	
	if (p_node == 0)
	{
		Cue_Warn("Cue::createCue: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Cue")
	{
		Cue_Warn("Cue::createCue: xml node '%s' is not a cue node\n",
		         p_node->getName().c_str());
		return 0;
	}
	
	if (p_soundBank == 0)
	{
		Cue_Warn("Cue::createCue: sound bank must not be 0\n");
		return 0;
	}
	
	Cue* cue = new Cue;
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Variation")
		{
			const std::string& vartypestr(child->getAttribute("Variation Type"));
			
			if (vartypestr.empty())
			{
				Cue_Warn("Cue::createCue: no variation type specified\n");
				continue;
			}
			
			const s32 type = str::parseS32(vartypestr, &errStatus);
			if (errStatus.hasError())
			{
				Cue_Warn("Cue::createCue: "
				         "invalid variation type specified: '%s'\n",
				         vartypestr.c_str());
				continue;
			}
			
			switch (type)
			{
			case 0:
				cue->setVariation(Variation_Ordered);
				break;
				
			case 1:
				cue->setVariation(Variation_OrderedFromRandom);
				break;
				
			case 2:
				cue->setVariation(Variation_Random);
				break;
				
			case 3:
				cue->setVariation(Variation_RandomNoRepeat);
				break;
				
			case 4:
			{
				const std::string& tabletypestr(child->getAttribute("Variation Table Type"));
				
				if (tabletypestr.empty())
				{
					Cue_Warn("Cue::createCue: "
					         "variation type 4 specified but "
					         "table type is missing\n");
					break;
				}
				
				const s32 tabletype = str::parseS32(tabletypestr, &errStatus);
				if (errStatus.hasError())
				{
					Cue_Warn("Cue::createCue: "
					         "invalid variation table type "
					         "specified: '%s'\n",
					         tabletypestr.c_str());
					break;
				}
				
				if (tabletype == 1)
				{
					cue->setVariation(Variation_Shuffle);
				}
				else if (tabletype == 3)
				{
					cue->setVariation(Variation_Interactive);
				}
				else
				{
					Cue_Warn("Cue::createCue: "
					         "unknown table type (%d) "
					         "for variation type 4\n",
					         tabletype);
				}
				break;
			}
			
			default:
				Cue_Warn("Cue::creatCue: "
				         "unknown variation type %d\n", type);
				break;
			}
		}
		else if (child->getName() == "Sound Entry")
		{
			const std::string& indexstr(child->getAttribute("Index"));
			
			if (indexstr.empty())
			{
				Cue_Warn("Cue::createCue: no sound entry index specified\n");
				continue;
			}
			
			const s32 index = str::parseS32(indexstr, &errStatus);
			if (errStatus.hasError())
			{
				Cue_Warn("Cue::createCue: "
				         "invalid sound entry index specified: '%s'\n",
				         indexstr.c_str());
				continue;
			}
			
			// Retrieve Sound from soundbank
			Sound* sound = p_soundBank->getSound(index);
			if (sound == 0)
			{
				Cue_Warn("Cue::createCue: unable to retreive "
				         "sound %d from sound bank\n",
				         index);
				continue;
			}
			
			const std::string& weightstr(child->getAttribute("Weight Max"));
			
			if (weightstr.empty())
			{
				Cue_Warn("Cue::createCue: "
				         "no sound entry weight specified\n");
				continue;
			}
			
			int weight = str::parseS32(weightstr, &errStatus);
			if (errStatus.hasError())
			{
				Cue_Warn("Cue::createCue: invalid sound entry "
				         "weight specified '%s'\n",
				         weightstr.c_str());
				continue;
			}
			
			cue->addSound(sound, weight);
		}
		else if (child->getName() == "Instance Limit")
		{
			const std::string& instanceStr(child->getAttribute("Max Instances"));
			if (instanceStr.empty())
			{
				Cue_Warn("Cue::createCue: no max instances specified.\n");
				continue;
			}
			
			const s32 instances = str::parseS32(instanceStr, &errStatus);
			if (errStatus.hasError())
			{
				Cue_Warn("Cue::createCue: "
				         "invalid Max Instances specified: '%s'\n",
				         instanceStr.c_str());
				continue;
			}
			
			const std::string& behaviorStr(child->getAttribute("Behavior"));
			
			if (behaviorStr.empty())
			{
				Cue_Warn("Cue::createCue: no instance limit behavior specified\n");
				continue;
			}
			
			const s32 behavior = str::parseS32(behaviorStr, &errStatus);
			if (errStatus.hasError())
			{
				Cue_Warn("Cue::createCue: "
				         "invalid instance limit behavior specified: '%s'\n",
				         behaviorStr.c_str());
				continue;
			}
			
			switch (behavior)
			{
			case 0:
				cue->setLimitBehavior(LimitBehavior_FailToPlay);
				cue->setInstanceLimit(instances);
				break;
				
			case 1:
				cue->setLimitBehavior(LimitBehavior_Queue);
				cue->setInstanceLimit(instances);
				break;
				
			case 2:
				cue->setLimitBehavior(LimitBehavior_Replace);
				cue->setInstanceLimit(instances);
				break;
				
			default:
				Cue_Warn("Cue::creatCue: "
				         "unknown instance limit behavior %d\n", behavior);
				break;
			}
		}
		else
		{
			Cue_Warn("Cue::createCue: encountered unknown child node '%s'\n",
			         child->getName().c_str());
		}
	}
	
	return cue;
}


Cue::ErrorStatus Cue::instantiate(CueInstancePtr& p_instance)
{
	Cue_Trace("Cue::instantiate\n");
	p_instance.reset();
	
	if (m_limitBehavior != LimitBehavior_None && instanceLimitReached())
	{
		switch (m_limitBehavior)
		{
		case LimitBehavior_FailToPlay:
			return ErrorStatus_InstanceLimited;
			
		case LimitBehavior_Queue:
			Cue_Warn("Cue::instantiate: Queue behavior not supported.\n");
			return ErrorStatus_InstanceLimited;
			
		case LimitBehavior_Replace:
			if (m_instances.size() > 0)
			{
				CueInstancePtr cue(m_instances.front());
				if (cue != 0)
				{
					cue->stop();
				}
				m_instances.pop_front();
				TT_ASSERT(instanceLimitReached() == false);
			}
			break;
			
		default:
			Cue_Warn("Cue::instantiate: Unknown behavior %d.\n",
			         m_limitBehavior);
			return ErrorStatus_InstanceLimited;
		}
	}
	
	if (m_prevSound == 0)
	{
		// initialize play list
		// prepare playlist
		switch (m_variation)
		{
		case Variation_Ordered:
			{
				orderPlayList();
				break;
			}
			
		case Variation_OrderedFromRandom:
			{
				orderPlayList();
				if (m_playList.empty() == false)
				{
					u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
					m_prevSound = m_playOrder[index];
				}
				else
				{
					m_prevSound = 0;
				}
				break;
			}
			
		case Variation_Random:
		case Variation_RandomNoRepeat:
			{
				shufflePlayList();
				if (m_playList.empty() == false)
				{
					u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
					m_prevSound = m_playOrder[index];
				}
				else
				{
					m_prevSound = 0;
				}
				break;
			}
			
		case Variation_Shuffle:
			{
				shufflePlayList();
				break;
			}
			
		default: // Warning: Variation_Interactive not handled in switch.
			break;
		}
	}
	m_prevSound = getNextSound();
	
	if (m_prevSound == 0)
	{
		return ErrorStatus_NoPreviousSound;
	}
	
	p_instance.reset(new CueInstance(this, m_prevSound));
	
	m_instances.push_back(p_instance);
	
	return ErrorStatus_OK;
}


bool Cue::play()
{
	Cue_Trace("Cue::play\n");
	
	CueInstancePtr instance;
	instantiate(instance);
	
	if (instance == 0)
	{
		return true;
	}
	return instance->play();
}


CueInstancePtr Cue::playCue()
{
	Cue_Trace("Cue::playCue\n");
	
	CueInstancePtr instance;
	instantiate(instance);
	
	if (instance == 0)
	{
		return CueInstancePtr();
	}
	
	if (instance->play() == false)
	{
		return CueInstancePtr();
	}
	
	return instance;
}


bool Cue::stop()
{
	Cue_Trace("Cue::stop\n");
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->stop() == false)
		{
			success = false;
		}
	}
	
	return success;
}


bool Cue::pause()
{
	Cue_Trace("Cue::pause\n");
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->pause() == false)
		{
			success = false;
		}
	}
	
	return success;
}


bool Cue::resume()
{
	Cue_Trace("Cue::resume\n");
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->resume() == false)
		{
			success = false;
		}
	}
	
	return success;
}


bool Cue::pauseCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	
	bool found = false;
	for (PlayList::const_iterator it = m_playList.begin(); it != m_playList.end(); ++it)
	{
		if ((*it).first->getCategory() == p_cat)
		{
			found = true;
			break;
		}
	}
	if (found == false)
	{
		return true;
	}
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->pause() == false)
		{
			success = false;
		}
	}
	
	return success;
}


bool Cue::resumeCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	
	bool found = false;
	for (PlayList::const_iterator it = m_playList.begin(); it != m_playList.end(); ++it)
	{
		if ((*it).first->getCategory() == p_cat)
		{
			found = true;
			break;
		}
	}
	if (found == false)
	{
		return true;
	}
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->resume() == false)
		{
			success = false;
		}
	}
	
	return success;
}


bool Cue::stopCategory(Category* p_cat)
{
	if (p_cat == 0)
	{
		return false;
	}
	
	bool found = false;
	for (PlayList::const_iterator it = m_playList.begin(); it != m_playList.end(); ++it)
	{
		if ((*it).first->getCategory() == p_cat)
		{
			found = true;
			break;
		}
	}
	if (found == false)
	{
		return true;
	}
	
	bool success = true;
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->stop() == false)
		{
			success = false;
		}
	}
	
	return success;
}


void Cue::setReverbVolumeForCategory(Category* p_category, real p_volumeInDB)
{
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		if ((*it)->belongsToCategory(p_category))
		{
			(*it)->setReverbVolume(p_volumeInDB);
		}
	}
}


void Cue::update(real p_delta)
{
	Cue_Trace("Cue::update\n");
	
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); )
	{
		(*it)->update(p_delta);
		if ((*it)->isDone())
		{
			it = m_instances.erase(it);
		}
		else
		{
			++it;
		}
	}
}


void Cue::updateVolume()
{
	Cue_Trace("Cue::updateVolume\n");
	
	for (Instances::iterator it = m_instances.begin(); it != m_instances.end(); ++it)
	{
		(*it)->updateVolume();
	}
}


//--------------------------------------------------------------------------------------------------
// Private Functions

bool Cue::load(const fs::FilePtr& p_file)
{
	// read playlist
	int size;
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read soundbank index (should be >= 0)
		int soundBankIndex;
		fs::readInteger(p_file, &soundBankIndex);
		TT_ASSERT(soundBankIndex >= 0);
		
		// read sound index (should be >= 0)
		int soundIndex;
		fs::readInteger(p_file, &soundIndex);
		TT_ASSERT(soundIndex >= 0);
		
		// read weight
		int weight;
		fs::readInteger(p_file, &weight);
		
		// now fetch the correct sound from the soundbank
		SoundBank* bank = AudioTT::getSoundBank(soundBankIndex);
		TT_NULL_ASSERT(bank);
		
		Sound* sound = bank->getSound(soundIndex);
		TT_NULL_ASSERT(sound);
		
		addSound(sound, weight);
	}
	
	// read members
	fs::readEnum<u8>(p_file, &m_variation);
	fs::readEnum<u8>(p_file, &m_limitBehavior);
	fs::readInteger (p_file, &m_limit);
	
	return true;
}


bool Cue::save(const fs::FilePtr& p_file) const
{
	// save playlist
	fs::writeInteger(p_file, m_playList.size());
	for (PlayList::const_iterator it = m_playList.begin(); it != m_playList.end(); ++it)
	{
		// save soundbank index (should be >= 0)
		int soundBankIndex = (*it).first->getSoundBankIndex();
		TT_ASSERT(soundBankIndex >= 0);
		fs::writeInteger(p_file, soundBankIndex);
		
		// save soundindex (should be >= 0)
		int soundIndex = (*it).first->getSoundIndex();
		TT_ASSERT(soundIndex >= 0);
		fs::writeInteger(p_file, soundIndex);
		
		// save weight
		fs::writeInteger(p_file, (*it).second);
	}
	
	// write members
	fs::writeEnum<u8>(p_file, m_variation);
	fs::writeEnum<u8>(p_file, m_limitBehavior);
	fs::writeInteger (p_file, m_limit);
	
	return true;
}


void Cue::orderPlayList()
{
	Cue_Trace("Cue::orderPlayList\n");
	
	m_playOrder.clear();
	
	int maxweight = 255;
	
	while (m_playOrder.size() != m_playList.size())
	{
		int secondbest = 0;
		
		for (PlayList::iterator it = m_playList.begin(); it != m_playList.end(); ++it)
		{
			if ((*it).second == maxweight)
			{
				m_playOrder.push_back((*it).first);
			}
			else if ((*it).second > secondbest && (*it).second < maxweight)
			{
				secondbest = (*it).second;
			}
		}
		maxweight = secondbest;
	}
}


void Cue::shufflePlayList()
{
	Cue_Trace("Cue::shufflePlayList\n");
	
	Cue_Printf("Cue::shufflePlayList: cue contains %d sounds\n", m_playList.size());
	
	m_playOrder.clear();
	
	while (m_playOrder.size() != m_playList.size())
	{
		Cue_Printf("Cue::shufflePlayList: %d left\n",  m_playList.size() - m_playOrder.size());
		
		const u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
		Cue_Printf("Cue::shufflePlayList: attempting %d\n", index);
		
		Sound* snd = m_playList[index].first;
		
		if (std::find(m_playOrder.begin(), m_playOrder.end(), snd) == m_playOrder.end())
		{
			m_playOrder.push_back(snd);
		}
	}
	Cue_Printf("Cue::shufflePlayList: done\n");
}


Sound* Cue::getNextSound()
{
	Cue_Trace("Cue::getNextSound\n");
	if (m_playList.empty())
	{
		return 0;
	}
	
	switch (m_variation)
	{
	case Variation_Ordered:
	case Variation_OrderedFromRandom:
		{
			Cue_Printf("Cue::getNextSound: get ordered sound\n");
			// get next item in playlist
			// loop through playlist
			OrderedList::iterator it = std::find(m_playOrder.begin(), m_playOrder.end(), m_prevSound);
			
			if (it == m_playOrder.end())
			{
				return *(m_playOrder.begin());
			}
			
			++it;
			
			if (it == m_playOrder.end())
			{
				return *(m_playOrder.begin());
			}
			
			return *it;
		}
		
	case Variation_Random:
		{
			Cue_Printf("Cue::getNextSound: get random sound\n");
			// get random item
			const u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
			return m_playList[index].first;
		}
		
	case Variation_RandomNoRepeat:
		{
			Cue_Printf("Cue::getNextSound: get random sound no repeat\n");
			if (m_playList.size() == 1)
			{
				// prevent eternal looping
				return (*(m_playList.begin())).first;
			}
			
			// get random item
			u32 index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
			while (m_playList[index].first == m_prevSound)
			{
				index = getXactRandom().getNext(static_cast<u32>(m_playList.size()));
			}
			
			return m_playList[index].first;
		}
		
	case Variation_Shuffle:
		{
			Cue_Printf("Cue::getNextSound: get shuffled sound\n");
			// get next item in playlist
			// if there are no more items
			// shuffle the playlist and get first item
			
			OrderedList::iterator it = std::find(m_playOrder.begin(), m_playOrder.end(), m_prevSound);
			
			if (it == m_playOrder.end())
			{
				shufflePlayList();
				return *(m_playOrder.begin());
			}
			
			++it;
			
			if (it == m_playOrder.end())
			{
				shufflePlayList();
				return *(m_playOrder.begin());
			}
			
			return *it;
		}
		
	case Variation_Interactive:
		{
			// unsupported
			return 0;
		}
	}
	
	return 0;
}

// Namespace end
}
}
}
