#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Category.h>
#include <tt/audio/xact/CueInstance.h>
#include <tt/audio/xact/RuntimeParameterControl.h>
#include <tt/audio/xact/Sound.h>
#include <tt/audio/xact/SoundInstance.h>
#include <tt/audio/xact/Track.h>
#include <tt/code/ErrorStatus.h>
#include <tt/code/helpers.h>
#include <tt/fs/File.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/parse.h>
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

Sound::Sound()
:
m_volumeInDB(0.0f),
m_pitch(0.0f),
m_priority(0),
m_category(0),
m_tracks(),
m_soundIndex(-1),
m_soundBankIndex(-1)
{
}


Sound::~Sound()
{
	code::helpers::freePointerContainer(m_tracks);
}


void Sound::addTrack(Track* p_track)
{
	if (p_track == 0)
	{
		Sound_Warn("SoundInstance::addTrack: track must not be 0\n");
		return;
	}
	
	m_tracks.push_back(p_track);
}


void Sound::setVolume(real p_volumeInDB)
{
	if (p_volumeInDB > 6.0f)
	{
		Sound_Warn("Sound::setVolume: volume %f larger than 6.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = 6.0f;
	}
	
	if (p_volumeInDB < -96.0f)
	{
		Sound_Warn("Sound::setVolume: volume %f less than -96.0dB; clamping\n", realToFloat(p_volumeInDB));
		p_volumeInDB = -96.0f;
	}
	
	m_volumeInDB = p_volumeInDB;
}


void Sound::setPitch(real p_pitch)
{
	if (p_pitch > 12.0f)
	{
		Sound_Warn("Sound::setPitch: pitch %f larger than 12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = 12.0f;
	}
	
	if (p_pitch < -12.0f)
	{
		Sound_Warn("Sound::setPitch: pitch %f less than -12.0; clamping\n", realToFloat(p_pitch));
		p_pitch = -12.0f;
	}
	
	m_pitch = p_pitch;
}


void Sound::setPriority(int p_priority)
{
	if (p_priority > 255)
	{
		Sound_Warn("Sound::setPriority: priority %d larger than 255; clamping\n", p_priority);
		p_priority = 255;
	}
	
	if (p_priority < 0)
	{
		Sound_Warn("Sound::setPriority: priority %d less than 0; clamping\n", p_priority);
		p_priority = 0;
	}
	m_priority = p_priority;
}


Sound* Sound::createSound(const xml::XmlNode* p_node)
{
	TT_ERR_CREATE("createSound");
	
	if (p_node == 0)
	{
		Sound_Warn("Sound::createSound: xml node must not be 0\n");
		return 0;
	}
	
	if (p_node->getName() != "Sound")
	{
		Sound_Warn("Sound::createSound: xml node '%s' is not a sound node\n", p_node->getName().c_str());
		return 0;
	}
	
	Sound* sound = new Sound;
	
	const std::string& volstr(p_node->getAttribute("Volume"));
	if (volstr.empty())
	{
		Sound_Warn("Sound::createSound: no volume specified\n");
	}
	else
	{
		sound->setVolume(xml::fast_atof(volstr.c_str()) / 100.0f);
	}
	
	const std::string& pitchstr(p_node->getAttribute("Pitch"));
	if (pitchstr.empty())
	{
		Sound_Warn("Sound::createSound: no pitch specified\n");
	}
	else
	{
		sound->setPitch(xml::fast_atof(pitchstr.c_str()) / 100.0f);
	}
	
	const std::string& priostr(p_node->getAttribute("Priority"));
	if (priostr.empty())
	{
		Sound_Warn("Sound::createSound: no priority specified\n");
	}
	else
	{
		const s32 priority = str::parseS32(priostr, &errStatus);
		if (errStatus.hasError())
		{
			Sound_Warn("Sound::createSound: invalid priority value '%s'\n", priostr.c_str());
		}
		else
		{
			sound->setPriority(priority);
		}
	}
	
	for (const xml::XmlNode* child = p_node->getChild(); child != 0; child = child->getSibling())
	{
		if (child->getName() == "Category Entry")
		{
			const std::string& name(child->getAttribute("Name"));
			if (name.empty())
			{
				Sound_Warn("Sound::createSound: missing category name\n");
			}
			else
			{
				Category* category = AudioTT::getCategory(name);
				
				if (category == 0)
				{
					Sound_Warn("Sound::createSound: could not retrieve category '%s'\n", name.c_str());
				}
				else
				{
					sound->setCategory(category);
				}
			}
		}
		else if (child->getName() == "RPC Entry")
		{
			std::string rpcName = child->getAttribute("RPC Name");
			
			addRPC(sound, rpcName);
		}
		else if (child->getName() == "Effect Entry")
		{
			// Ignore "Effect Entry" nodes: they're for Microsoft Reverb
		}
		else if (child->getName() == "Track")
		{
			Track* track = Track::createTrack(child);
			
			if (track == 0)
			{
				Sound_Warn("Sound::createSound: Error creating track\n");
			}
			else
			{
				sound->addTrack(track);
			}
		}
		else
		{
			Sound_Warn("Sound::createSound: encountered unknown child node '%s'\n", child->getName().c_str());
		}
	}
	
	return sound;
}


SoundInstance* Sound::instantiate(CueInstance* p_cue)
{
	SoundInstance* instance = new SoundInstance(this, p_cue);
	
	for (Tracks::iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		instance->addTrack((*it)->instantiate(instance));
	}
	
	return instance;
}


bool Sound::load(const fs::FilePtr& p_file)
{
	// read members
	fs::readReal(p_file, &m_volumeInDB);
	fs::readReal(p_file, &m_pitch);
	fs::readInteger(p_file, &m_priority);
	
	// read category name
	std::string name;
	fs::readNarrowString(p_file, &name);
	
	// assign correct category
	m_category = AudioTT::getCategory(name);
	TT_NULL_ASSERT(m_category);
	
	// read tracks
	int size;
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		Track* track = new Track;
		track->load(p_file);
		
		m_tracks.push_back(track);
	}
	
	// load RPCs
	fs::readInteger(p_file, &size);
	for (int i = 0; i < size; ++i)
	{
		std::string RPCName;
		fs::readNarrowString(p_file, &RPCName);
		addRPC(this, RPCName);
	}
	
	return true;
}


bool Sound::save(const fs::FilePtr& p_file) const
{
	// write members
	fs::writeReal(p_file, m_volumeInDB);
	fs::writeReal(p_file, m_pitch);
	fs::writeInteger(p_file, m_priority);
	
	// save category name (so load function can get correct category)
	TT_NULL_ASSERT(m_category);
	fs::writeNarrowString(p_file, m_category->getName());
	
	// write tracks
	fs::writeInteger(p_file, m_tracks.size());
	for (Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end(); ++it)
	{
		(*it)->save(p_file);
	}
	
	// write RPC names
	fs::writeInteger(p_file, m_RPCs.size());
	for (RPCs::const_iterator it = m_RPCs.begin(); it != m_RPCs.end(); ++it)
	{
		fs::writeNarrowString(p_file, (*it)->getName());
	}
	
	return true;
}


void Sound::addRPC(Sound* p_sound, const std::string& p_RPCName)
{
	if(p_sound != 0 && p_RPCName.empty() == false)
	{
		// Link to RPC Preset
		RuntimeParameterControl* rpc = AudioTT::getRPCPreset(p_RPCName);
		if(rpc != 0)
		{
			p_sound->m_RPCs.push_back(rpc);
		}
		else
		{
			TT_PANIC("RPC '%s' not found in presets", p_RPCName.c_str());
		}
	}
}

// Namespace end
}
}
}
