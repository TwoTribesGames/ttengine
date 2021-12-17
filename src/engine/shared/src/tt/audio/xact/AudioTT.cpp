#include <cstring>

#include <tt/audio/xact/AudioTT.h>
#include <tt/audio/xact/Category.h>
#include <tt/audio/xact/RuntimeParameterControl.h>
#include <tt/audio/xact/SoundBank.h>
#include <tt/audio/xact/WaveBank.h>
#include <tt/code/helpers.h>
#include <tt/doc/xap/XapReader.h>
#include <tt/fs/File.h>
#include <tt/fs/utils/utils.h>
#include <tt/platform/tt_printf.h>
#include <tt/snd/snd.h>
#include <tt/str/common.h>
#include <tt/xml/XmlNode.h>

#if !defined(TT_BUILD_FINAL)
#include <tt/system/Time.h>
#endif


static const char* gs_binarySignature     = "BXAP";
static const s32   gs_binarySignatureSize = 5;
static const s32   gs_binaryVersion       = 4;


//#define AUDIO_DEBUG
#ifdef AUDIO_DEBUG
	#define Audio_Printf TT_Printf
#else
	#define Audio_Printf(...)
#endif


//#define AUDIO_TRACE
#ifdef AUDIO_TRACE
	#define Audio_Trace TT_Printf
#else
	#define Audio_Trace(...)
#endif


#define AUDIO_WARN
#ifdef AUDIO_WARN
	#define Audio_Warn TT_Printf
#else
	#define Audio_Warn(...)
#endif


namespace tt {
namespace audio {
namespace xact {

bool                    AudioTT::ms_stereo        = true;
bool                    AudioTT::ms_fromConverter = false;
AudioTT::SoundBanks     AudioTT::ms_soundBanks;
AudioTT::WaveBanks      AudioTT::ms_waveBanks;
AudioTT::Categories     AudioTT::ms_categories;
AudioTT::RPCPresets     AudioTT::ms_RPCPresets;
real                    AudioTT::ms_masterVolume     = 1.0f;
std::string             AudioTT::ms_root;
const std::string       AudioTT::ms_emptyString;


fs::identifier  AudioTT::ms_fs = 0;
snd::identifier AudioTT::ms_ss = 0;


bool AudioTT::load(const std::string& p_fileName, bool p_autoload, bool p_fromConverter)
{
	Audio_Trace("AudioTT::load: '%s' (type %d)\n", p_fileName.c_str(), ms_fs);
	
	ms_fromConverter = p_fromConverter;
	
	if (p_fromConverter == false)
	{
		ms_stereo = snd::supportsStereo(ms_ss);
	}
	
	// find root
	std::string::size_type pos = p_fileName.rfind('/');
	if (pos == std::string::npos)
	{
		ms_root = "/";
	}
	else
	{
		ms_root = p_fileName.substr(0, pos + 1);
	}
	
	// load XAP/BXAP based on extension
	std::string ext(str::toLower(tt::fs::utils::getExtension(p_fileName)));
	
	if (ext == "bxap")
	{
		return loadBXAP(p_fileName, p_autoload);
	}
	else if (ext == "xap")
	{
		return loadXAP(p_fileName, p_autoload);
	}
	
	TT_PANIC("Invalid XAP extension '%s'. Valid filetypes are 'xap' and 'bxap'", ext.c_str());
	return false;
}


bool AudioTT::saveBXAP(const std::string& p_fileName)
{
	fs::FilePtr file(fs::open(p_fileName, fs::OpenMode_Write));
	TT_NULL_ASSERT(file);
	
	// write file signature and version
	fs::write(file, gs_binarySignature, gs_binarySignatureSize);
	fs::writeInteger(file, gs_binaryVersion);
	
	// write members
	fs::writeReal(file, ms_masterVolume);
	
	// save all category info (do this first, as sounds depend on it!)
	fs::writeInteger(file, ms_categories.size());
	
	for (Categories::const_iterator it = ms_categories.begin(); it != ms_categories.end(); ++it)
	{
		// write name
		fs::writeNarrowString(file, (*it).first);
		
		// write category
		(*it).second->save(file);
	}
	
	// save all RPCPresets
	fs::writeInteger(file, ms_RPCPresets.size());
	
	for (RPCPresets::const_iterator it = ms_RPCPresets.begin();
	     it != ms_RPCPresets.end(); ++it)
	{
		// write name
		fs::writeNarrowString(file, (*it).first);
		
		// write RPCPreset
		(*it).second->save(file);
	}
	
	// save all wave bank info (sounds depend on it!)
	fs::writeInteger(file, ms_waveBanks.size());
	
	for (WaveBanks::const_iterator it = ms_waveBanks.begin();
	     it != ms_waveBanks.end(); ++it)
	{
		// write name
		fs::writeNarrowString(file, (*it).first);
		
		// write wavebank
		(*it).second->save(file);
	}
	
	// save all sound bank info
	fs::writeInteger(file, ms_soundBanks.size());
	
	for (SoundBanks::const_iterator it = ms_soundBanks.begin();
	     it != ms_soundBanks.end(); ++it)
	{
		// write name
		fs::writeNarrowString(file, (*it).first);
		
		// write soundbank
		(*it).second->save(file);
	}
	
	return true;
}


bool AudioTT::unload()
{
	Audio_Trace("AudioTT::unload\n");
	
	code::helpers::freePairSecondContainer(ms_soundBanks);
	code::helpers::freePairSecondContainer(ms_waveBanks);
	code::helpers::freePairSecondContainer(ms_categories);
	code::helpers::freePairSecondContainer(ms_RPCPresets);
	
	// unload
	return true;
}


bool AudioTT::hasCategory(const std::string& p_name)
{
	Audio_Trace("AudioTT::hasCategory: '%s'\n", p_name.c_str());
	return ms_categories.find(p_name) != ms_categories.end();
}


Category* AudioTT::getCategory(const std::string& p_name)
{
	Audio_Trace("AudioTT::getCategory: '%s'\n", p_name.c_str());
	
	Categories::iterator it = ms_categories.find(p_name);
	
	if (it == ms_categories.end())
	{
		Audio_Warn("AudioTT::getCategory: unable to find category '%s'\n", p_name.c_str());
		return 0;
	}
	
	return (*it).second;
}


WaveBank* AudioTT::getWaveBank(int p_index)
{
	Audio_Trace("AudioTT::getWaveBank: %d\n", p_index);
	if (p_index < 0 || p_index >= static_cast<int>(ms_waveBanks.size()))
	{
		Audio_Warn("AudioTT::getWaveBank: unable to find wavebank %d\n", p_index);
		return 0;
	}
	
	return ms_waveBanks[p_index].second;
}


SoundBank* AudioTT::getSoundBank(const std::string& p_soundbank)
{
	Audio_Trace("AudioTT::getSoundBank: '%s'\n", p_soundbank.c_str());
	
	for (s32 i = 0; i < static_cast<s32>(ms_soundBanks.size()); ++i)
	{
		if (ms_soundBanks[i].first == p_soundbank)
		{
			return ms_soundBanks[i].second;
		}
	}
	
	TT_PANIC("Unable to find soundbank '%s'\n", p_soundbank.c_str());
	return 0;
}


SoundBank* AudioTT::getSoundBank(int p_index)
{
	Audio_Trace("AudioTT::getSoundBank: %d\n", p_index);
	if (p_index < 0 || p_index >= static_cast<int>(ms_soundBanks.size()))
	{
		TT_PANIC("Unable to find soundbank %d\n", p_index);
		return 0;
	}
	
	return ms_soundBanks[p_index].second;
}


RuntimeParameterControl* AudioTT::getRPCPreset(const std::string& p_name)
{
	Audio_Trace("AudioTT::getRPCPreset: '%s'\n", p_name.c_str());
	
	RPCPresets::iterator it = ms_RPCPresets.find(p_name);
	
	if (it == ms_RPCPresets.end())
	{
		Audio_Warn("AudioTT::getRPCPreset: unable to find RPC preset '%s'\n", p_name.c_str());
		return 0;
	}
	
	return (*it).second;
}


bool AudioTT::pauseCategory(const std::string& p_category)
{
	Category* cat = getCategory(p_category);
	if (cat == 0)
	{
		return false;
	}
	
	bool success = true;
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		if ((*it).second->pauseCategory(cat) == false)
		{
			success = false;
		}
	}
	return success;
}


bool AudioTT::resumeCategory(const std::string& p_category)
{
	Category* cat = getCategory(p_category);
	if (cat == 0)
	{
		return false;
	}
	
	bool success = true;
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		if ((*it).second->resumeCategory(cat) == false)
		{
			success = false;
		}
	}
	return success;
}


bool AudioTT::stopCategory(const std::string& p_category)
{
	Category* cat = getCategory(p_category);
	if (cat == 0)
	{
		return false;
	}
	
	bool success = true;
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		if ((*it).second->stopCategory(cat) == false)
		{
			success = false;
		}
	}
	return success;
}


void AudioTT::setReverbVolumeForCategory(const std::string& p_categoryName,
                                         real               p_volumeInDB)
{
	Category* cat = getCategory(p_categoryName);
	if (cat == 0) return;
	
	// Set the new volume for the category (used when playing new sounds)
	cat->setReverbVolume(p_volumeInDB);
	
	// Update the volume for already playing sounds
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		(*it).second->setReverbVolumeForCategory(cat, p_volumeInDB);
	}
}


void AudioTT::update(real p_delta)
{
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		(*it).second->update(p_delta);
	}
}
	
	
void AudioTT::updateVolume()
{
	for (SoundBanks::iterator it = ms_soundBanks.begin(); it != ms_soundBanks.end(); ++it)
	{
		(*it).second->updateVolume();
	}
}


void AudioTT::setMasterVolume(real p_masterVolume)
{
	if (p_masterVolume < 0.0f)
	{
		Audio_Warn("AudioTT::setMasterVolume: volume %f is less than 0.\n", realToFloat(p_masterVolume));
		p_masterVolume = 0.0f;
	}
	if (p_masterVolume > 1.0f)
	{
		Audio_Warn("AudioTT::setMasterVolume: volume %f is larger than 1.\n", realToFloat(p_masterVolume));
		p_masterVolume = 1.0f;
	}
	ms_masterVolume = p_masterVolume;
	
	updateVolume();
}


bool AudioTT::loadXAP(const std::string& p_fileName, bool p_autoload)
{
#if !defined TT_BUILD_FINAL
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	doc::xap::XapReader xap;
	xml::XmlNode* node = xap.readFile(p_fileName.c_str(), ms_fs);
	
	if (node == 0)
	{
		Audio_Warn("AudioTT::load: error parsing '%s'\n", p_fileName.c_str());
		return false;
	}
	
	if (node->getName() != "xap")
	{
		Audio_Warn("AudioTT::load: root node ('%s') is not a xap node\n", node->getName().c_str());
		return false;
	}
	
	xml::XmlNode* child = node->getChild();
	
	if (child == 0)
	{
		Audio_Warn("AudioTT::load: root node does not have any children\n");
		return false;
	}
	
	while (child != 0)
	{
		if (child->getName() == "Sound Bank")
		{
			const std::string name = child->getAttribute("Name");
			Audio_Printf("AudioTT::load: found sound bank '%s'\n", name.c_str());
			
			SoundBank* bank = SoundBank::createSoundBank(static_cast<int>(ms_soundBanks.size()), child);
			
			if (bank == 0)
			{
				TT_PANIC("AudioTT::load: error creating sound bank\n");
			}
			else
			{
				ms_soundBanks.push_back(SoundBanks::value_type(name, bank));
			}
		}
		else if (child->getName() == "Wave Bank")
		{
			const std::string name = child->getAttribute("Name");
			Audio_Printf("AudioTT::load: found wave bank '%s'\n", name.c_str());
			
			WaveBank* bank = WaveBank::createWaveBank(static_cast<int>(ms_waveBanks.size()), child, ms_fromConverter);
			
			if (bank == 0)
			{
				TT_PANIC("AudioTT::load: error creating wave bank\n");
			}
			else
			{
				ms_waveBanks.push_back(WaveBanks::value_type(name, bank));
				if (p_autoload)
				{
					bank->loadWaves();
				}
			}
		}
		else if (child->getName() == "Global Settings")
		{
			Audio_Printf("AudioTT::load: found global settings\n");
			
			xml::XmlNode* settingNode = child->getChild();
			while (settingNode != 0)
			{
				if (settingNode->getName() == "Category")
				{
					Audio_Printf("AudioTT::load: found category\n");
					
					std::string catName = settingNode->getAttribute("Name");
					
					if (catName.empty())
					{
						TT_PANIC("AudioTT::load: missing category name attribute\n");
					}
					else
					{
						Category* category = Category::createCategory(catName, settingNode);
						
						if (category == 0)
						{
							TT_PANIC("AudioTT::load: error creating category\n");
						}
						else
						{
							ms_categories.insert(Categories::value_type(catName, category));
						}
					}
				}
				// Runtime Parameter Controls
				else if (settingNode->getName() == "RPC")
				{
					Audio_Printf("AudioTT::load: found RPC\n");
					
					std::string rpcName = settingNode->getAttribute("Name");
					
					if(rpcName.empty())
					{
						TT_PANIC("Missing RPC name attribute\n");
					}
					// Ignore the "Microsoft Reverb" RPC (it is a special RPC for reverb effects)
					else if (rpcName != "Microsoft Reverb")
					{
						RuntimeParameterControl* rpc = RuntimeParameterControl::createRPC(rpcName, settingNode);
						
						if(rpc)
						{
							ms_RPCPresets.insert(RPCPresets::value_type(rpcName, rpc));
						}
						else
						{
							TT_PANIC("Error creating RPC\n");
						}
					}
				}
				
				settingNode = settingNode->getSibling();
			}
		}
		else
		{
			// ignore
		}
		child = child->getSibling();
	}
	delete node;
	
#if !defined(TT_BUILD_FINAL)
	if (ms_fromConverter == false)
	{
		const u64 endTime = tt::system::Time::getInstance()->getMilliSeconds();
		// Show how much memory was used by the wave bank data
		s32 totalMemSize = 0;
		TT_Printf("AudioTT::loadXAP: Wave banks loaded from XAP file '%s':\n", p_fileName.c_str());
		for (WaveBanks::iterator it = ms_waveBanks.begin(); it != ms_waveBanks.end(); ++it)
		{
			const s32 waveBankSize = (*it).second->getMemorySize();
			TT_Printf("AudioTT::loadXAP: - '%s': %d bytes (%.2f MB)\n",
			          (*it).first.c_str(), waveBankSize, waveBankSize / (1024.0f * 1024.0f));
			totalMemSize += waveBankSize;
		}
		TT_Printf("AudioTT::loadXAP: Loaded in %llu ms. Total memory used by all wave banks: %d bytes (%.2f MB)\n",
		          (endTime - startTime),
		          totalMemSize, totalMemSize / (1024.0f * 1024.0f));
	}
#endif
	
	return true;
}


bool AudioTT::loadBXAP(const std::string& p_fileName, bool p_autoload)
{
#if !defined TT_BUILD_FINAL
	const u64 startTime = tt::system::Time::getInstance()->getMilliSeconds();
#endif
	
	fs::FilePtr file(fs::open(p_fileName, fs::OpenMode_Read));
	TT_NULL_ASSERT(file);
	
	// verify signature
	char signature[gs_binarySignatureSize];
	fs::read(file, signature, gs_binarySignatureSize);
	if (std::memcmp(signature, gs_binarySignature, gs_binarySignatureSize) != 0)
	{
		TT_PANIC("Invalid BXAP signature");
		return false;
	}
	
	// verify version
	int version = 0;
	fs::readInteger(file, &version);
	if (version != gs_binaryVersion)
	{
		TT_PANIC("Invalid BXAP version. Code version is '%d', asset version is '%d'",
		         version, gs_binaryVersion);
		return false;
	}
	
	// read members
	fs::readReal(file, &ms_masterVolume);
	
	int size = 0;
	std::string name;
	
	// read all category info (do this first, as sounds depend on it!)
	fs::readInteger(file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read name
		fs::readNarrowString(file, &name);
		
		// create and load category
		Category *category = new Category(name);
		
		// load category
		category->load(file);
		
		ms_categories.insert(Categories::value_type(name, category));
	}
	
	// read all RPCPresets (sounds depend on it!)
	fs::readInteger(file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read name
		fs::readNarrowString(file, &name);
		
		// read RPC data
		RuntimeParameterControl* rpc = new RuntimeParameterControl(name);
		
		// add 
		ms_RPCPresets.insert(RPCPresets::value_type(name, rpc));
		
		rpc->load(file);
	}
	
	// read all wave bank info (sounds depend on it!)
	fs::readInteger(file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read name
		fs::readNarrowString(file, &name);
		
		// read wavebank data
		WaveBank* bank = new WaveBank(i, name);
		bank->load(file);
		
		if (p_autoload)
		{
			bank->loadWaves();
		}
		
		ms_waveBanks.push_back(WaveBanks::value_type(name, bank));
	}
	
	// read all sound bank info
	fs::readInteger(file, &size);
	for (int i = 0; i < size; ++i)
	{
		// read name
		fs::readNarrowString(file, &name);
		
		// read soundbank data
		SoundBank* bank = new SoundBank(i);
		
		// add 
		ms_soundBanks.push_back(SoundBanks::value_type(name, bank));
		
		bank->load(file);
	}
	
#if !defined(TT_BUILD_FINAL)
	if (ms_fromConverter == false)
	{
		const u64 endTime = tt::system::Time::getInstance()->getMilliSeconds();
		// Show how much memory was used by the wave bank data
		s32 totalMemSize = 0;
		TT_Printf("AudioTT::loadBXAP: Wave banks loaded from BXAP file '%s':\n", p_fileName.c_str());
		for (WaveBanks::iterator it = ms_waveBanks.begin(); it != ms_waveBanks.end(); ++it)
		{
			const s32 waveBankSize = (*it).second->getMemorySize();
			TT_Printf("AudioTT::loadBXAP: - '%s': %d bytes (%.2f MB)\n",
			          (*it).first.c_str(), waveBankSize, waveBankSize / (1024.0f * 1024.0f));
			totalMemSize += waveBankSize;
		}
		TT_Printf("AudioTT::loadBXAP: Loaded in %llu ms. Total memory used by all wave banks: %d bytes (%.2f MB)\n",
		          (endTime - startTime),
		          totalMemSize, totalMemSize / (1024.0f * 1024.0f));
	}
#endif
	
	return true;
}

// Namespace end
}
}
}
