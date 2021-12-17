#if !defined(INC_TT_AUDIO_PLAYER_XACT3PLAYER_H)
#define INC_TT_AUDIO_PLAYER_XACT3PLAYER_H


#define NOMINMAX
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <XAudio2.h>
#else
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\XAudio2.h>
#endif
#include <xact3.h>
#include <xact3d3.h>

#include <map>

#include <tt/audio/player/SoundPlayer.h>
#include <tt/code/Buffer.h>
#include <tt/fs/types.h>
#include <tt/math/Vector3.h>


namespace tt {
namespace audio {
namespace player {

class Xact3Player : public SoundPlayer
{
public:
	Xact3Player(bool p_mustInitCom = true);
	virtual ~Xact3Player();
	
	
	// Initialization functions
	
	/*! \brief Initialize the Xact engine.
	    \param p_globalSettings Global settings file (.xsg) (optional).
	    \param p_debug Whether debugging should be enabled.
	    \return true on success, false on failure or when already initialized.*/
	bool init(const std::string& p_globalSettings = "", bool p_debug = false);
	
	/*! \brief Uninitialize the Xact engine.
	    \return true on success, false on failure or when not initialized.*/
	bool uninit();
	
	/*! \brief Returns whether the player is initialized or not.
	    \return Whether the player is initialized or not.*/
	inline bool isInitialized() const { return m_engine != 0; }
	
	
	// Soundbank functions
	
	/*! \brief Loads a soundbank.
	    \param p_path Path of the soundbank file (.xsb) to load.
	    \return true on success, false on failure or when this soundbank was already loaded.*/
	bool loadSoundBank(const std::string& p_path);
	
	/*! \brief Unloads a soundbank.
	    \param p_path Path of the soundbank file (.xsb) to unload.
	    \return true on success, false on failure or when this soundbank was not loaded.*/
	bool unloadSoundBank(const std::string& p_path);
	
	/*! \brief Unloads all soundbanks.
	    \return true on success, false on failure.*/
	bool unloadAllSoundBanks();
	
	
	// Wavebank functions
	
	/*! \brief Loads a wavebank.
	    \param p_path Path of the wavebank file (.xsw) to load.
	    \param p_memoryMap Whether the file should be memory mapped (saves RAM). Not supported on Xbox 360!
	    \return true on success, false on failure or when this wavebank was already loaded.*/
	bool loadWaveBank(const std::string& p_path, bool p_memoryMap);
	
	/*! \brief Unloads a wavebank.
	    \param p_path Path of the wavebank file (.xsw) to unload.
	    \return true on success, false on failure or when this wavebank was not loaded.*/
	bool unloadWaveBank(const std::string& p_path);
	
	/*! \brief Unloads all wavebanks.
	    \return true on success, false on failure.*/
	bool unloadAllWaveBanks();
	
	
	virtual void update(real p_deltaTime);
	
	
	// Sound functions
	
	virtual SoundCuePtr createCue(const std::string& p_soundbank, const std::string& p_name, bool p_positional = false);
	virtual bool        play     (const std::string& p_soundbank, const std::string& p_name);
	virtual SoundCuePtr playCue  (const std::string& p_soundbank, const std::string& p_name, bool p_positional = false);
	virtual bool        stop     (const std::string& p_soundbank, const std::string& p_name);
	
	// Category functions
	
	virtual bool hasCategory      (const std::string& p_category);
	virtual bool stopCategory     (const std::string& p_category);
	virtual bool pauseCategory    (const std::string& p_category);
	virtual bool resumeCategory   (const std::string& p_category);
	virtual bool setCategoryVolume(const std::string& p_category, real p_normalizedVolume);
	
	
	// Positional Audio
	
	virtual bool set3DAudioEnabled(bool p_enabled);
	virtual bool setListenerPosition(const math::Vector3& p_position);
	
	// Global RPC variables
	virtual bool setGlobalVariable(const std::string& p_name, real  p_value);
	virtual bool getGlobalVariable(const std::string& p_name, real* p_value_OUT) const;
	
	bool init3DAudio();
	
private:
	struct SoundBank
	{
		IXACT3SoundBank* bank;
		code::BufferPtr  data;
		
		
		inline SoundBank()
		:
		bank(0),
		data()
		{ }
	};
	
	struct WaveBank
	{
		IXACT3WaveBank* bank;
		code::BufferPtr data;
		HANDLE          mappingHandle;
		HANDLE          fileHandle;
		void*           buffer;
		
		WaveBank()
		:
		bank(0),
		data(),
		mappingHandle(0),
		fileHandle(0),
		buffer(0)
		{ }
	};
	
	typedef std::map<std::string, SoundBank> SoundBanks;
	typedef std::map<std::string, WaveBank>  WaveBanks;
	
	IXACT3SoundBank* getBank(const std::string& p_soundbank) const;
	IXACT3Cue* prepareCue(IXACT3SoundBank* p_soundbank, XACTINDEX p_cueIndex);
	bool setEmitterPosition(IXACT3Cue*           p_cue,
	                        X3DAUDIO_EMITTER*    p_emitter_OUT,
	                        const math::Vector3& p_position);
	bool setEmitterRadius(IXACT3Cue*        p_cue,
	                      X3DAUDIO_EMITTER* p_emitter_OUT,
	                      real              p_inner,
	                      real              p_outer);
	bool applyEmitterSettings(IXACT3Cue* p_cue, X3DAUDIO_EMITTER* p_emitter);
	
	
	// No copying
	Xact3Player(const Xact3Player&);
	const Xact3Player& operator=(const Xact3Player&);
	
	
	IXACT3Engine*         m_engine;
	X3DAUDIO_HANDLE       m_3DHandle;
	X3DAUDIO_LISTENER     m_3DListener;
	X3DAUDIO_EMITTER      m_default3DEmitter;
	X3DAUDIO_DSP_SETTINGS m_3DSettings;
	
	SoundBanks        m_soundBanks;
	WaveBanks         m_waveBanks;
	math::Vector3     m_listenerPosition;
	bool              m_3DAudioEnabled;
	bool              m_3DInitialized;
	bool              m_mustReleaseCom;
	
	// To access prepareCue
	friend class Xact3Cue;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_PLAYER_XACT3PLAYER_H)
