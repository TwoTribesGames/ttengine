#ifndef INC_TT_AUDIO_XACT_AUDIOTT_H
#define INC_TT_AUDIO_XACT_AUDIOTT_H


#include <map>
#include <string>
#include <vector>

#include <tt/audio/xact/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>
#include <tt/snd/types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace audio {
namespace xact {

class AudioTT
{
public:
	// FIXME: This function is not implemented. Should it be, or should it be removed?
	//static bool init(fs::identifier p_fs, snd::identifier p_ss);
	static bool load(const std::string& p_fileName, bool p_autoload = true, bool p_fromConverter = false);
	
	// save loaded AudioTT state to a binary XAP (BXAP) file
	static bool saveBXAP(const std::string& p_fileName);
	
	static bool unload();
	
	static bool       hasCategory(const std::string& p_name);
	static Category*  getCategory(const std::string& p_name);
	
	static WaveBank*  getWaveBank(int p_index);
	static SoundBank* getSoundBank(const std::string& p_soundbank);
	static SoundBank* getSoundBank(int p_index);
	
	static RuntimeParameterControl* getRPCPreset(const std::string& p_name);
	
	static bool pauseCategory (const std::string& p_name);
	static bool resumeCategory(const std::string& p_name);
	static bool stopCategory  (const std::string& p_name);
	
	static void setReverbVolumeForCategory(const std::string& p_categoryName,
	                                       real               p_volumeInDB);
	
	static void update(real p_delta);
	
	static void updateVolume();
	static void setMasterVolume(real p_masterVolume);
	inline static real getMasterVolume() { return ms_masterVolume; }
	
	inline static void setStereo(bool p_stereo) { ms_stereo = p_stereo; }
	inline static bool getStereo()              { return ms_stereo;     }
	
	inline static int getWaveBankCount()  { return static_cast<int>(ms_waveBanks.size());  }
	inline static int getSoundBankCount() { return static_cast<int>(ms_soundBanks.size()); }
	
	inline static const std::string& getRoot() { return ms_fromConverter ? ms_emptyString : ms_root; }
	
	inline static fs::identifier  getFileSystem()  { return ms_fs; }
	inline static snd::identifier getSoundSystem() { return ms_ss; }
	
private:
	// non instantiable
	AudioTT();
	~AudioTT();
	AudioTT(const AudioTT&);
	AudioTT& operator=(const AudioTT&);
	
	// textual load
	static bool loadXAP(const std::string& p_fileName, bool p_autoload);
	
	// binary load
	static bool loadBXAP(const std::string& p_fileName, bool p_autoload);
	
	static bool load(const fs::FilePtr& p_file, bool p_autoload = true);
	static bool save(const fs::FilePtr& p_file);
	
	
	static bool              ms_stereo;
	static bool              ms_fromConverter;
	static std::string       ms_root;
	static const std::string ms_emptyString;
	
	// Members that should be saved
	static real           ms_masterVolume;     // Normalized
	
	typedef std::map<std::string, Category*>                 Categories;
	typedef std::vector<std::pair<std::string, SoundBank*> > SoundBanks;
	typedef std::vector<std::pair<std::string, WaveBank*> >  WaveBanks;
	typedef std::map<std::string, RuntimeParameterControl*>  RPCPresets;
	
	static Categories     ms_categories;
	static WaveBanks      ms_waveBanks;
	static SoundBanks     ms_soundBanks;
	static RPCPresets     ms_RPCPresets;
	
	static fs::identifier  ms_fs;
	static snd::identifier ms_ss;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_XACT_AUDIOTT_H
