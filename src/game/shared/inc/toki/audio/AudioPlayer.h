#if !defined(INC_TOKI_AUDIO_AUDIOPLAYER_H)
#define INC_TOKI_AUDIO_AUDIOPLAYER_H


#include <map>
#include <string>
#include <vector>

#include <tt/audio/player/SoundPlayer.h>
#include <tt/code/fwd.h>
#include <tt/math/TimedLinearInterpolation.h>
#include <tt/math/Vector3.h>
#include <tt/pres/fwd.h>
#include <tt/snd/types.h>
#include <tt/str/str_types.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/Mutex.h>

#include <toki/audio/constants.h>
#include <toki/audio/MusicTrackMgr.h>
#include <toki/audio/SoundCue.h>
#include <toki/game/entity/fwd.h>


namespace toki {
namespace audio {

class AudioPlayer
{
public:
	/*! \brief Whether the AudioPlayer should be created on the main thread (as opposed to on the loading thread).
	           NOTE: This function is implemented by each platform (not shared). */
	static bool needsCreateOnMainThread();
	
	// FIXME: Need thread synchronization?
	inline static void triggerCreateOnMainThreadNow() { ms_createOnMainThreadNow = true; }
	inline static bool shouldCreateOnMainThreadNow()  { return ms_createOnMainThreadNow; }
	
	static void createInstance(); //!< NOTE: This function is implemented by each platform (not shared).
	static void destroyInstance();
	
	inline static AudioPlayer* getInstance() { return ms_instance;      }
	inline static bool         hasInstance() { return ms_instance != 0; }
	
	void playEffect(const std::string& p_soundbank, const std::string& p_name);
	tt::audio::player::SoundCuePtr playEffectCue(const std::string& p_soundbank, const std::string& p_name);
	
	// Positional sound effects:
	tt::audio::player::SoundCuePtr playPositionalEffectCue(
			const std::string&                     p_soundbank,
			const std::string&                     p_name,
			const tt::pres::PresentationObjectPtr& p_presentationToFollow);
	tt::audio::player::SoundCuePtr playPositionalEffectCue(
			const std::string&                     p_soundbank,
			const std::string&                     p_name,
			const game::entity::EntityHandle&      p_entityToFollow);
	tt::audio::player::SoundCuePtr playPositionalEffectCue(
			const std::string&                     p_soundbank,
			const std::string&                     p_name,
			const tt::math::Vector3&               p_staticPosition);
	tt::audio::player::SoundCuePtr playPositionalEffectCue(
			const std::string&                     p_soundbank,
			const std::string&                     p_name,
			const AudioPositionInterfacePtr&       p_audioPositionInterface);
	
	void registerSoundCueForUpdate(const SoundCueWeakPtr& p_soundCue);
	void keepCueAliveUntilDonePlaying(const SoundCuePtr& p_soundCue);
	
	void fadeOutAndKeepAliveUntilDone(const tt::audio::player::SoundCuePtr& p_soundCue, real p_duration);
	
	void pauseAllAudio();
	void resumeAllAudio();
	void stopAllAudio();
	
#if !defined(TT_BUILD_FINAL)
	// DEBUG statistics
	inline s32 getDebugPositionalSoundCount() const { tt::thread::CriticalSection critSec(&m_containerMutex); return static_cast<s32>(m_positionalEffects.size()); }
	std::string getDebugPositionalSoundNames() const;
#endif
	
	void stopEffect(const std::string& p_soundbank, const std::string& p_name);
	
	void pause (Category p_category);
	void resume(Category p_category);
	void stop  (Category p_category);
	
	/*! \brief Sets the playback volume for the specified sound category.
	    \param p_category The sound category for which to set the volume.
	    \param p_volume The volume to set, in range [0.0 - 1.0]. */
	void setVolume(Category p_category, real p_volume);
	real getVolume(Category p_category) const;
	
	void setListenerPosition(const tt::math::Vector3& p_position);
	
	bool isInAudibleRange(const tt::math::Vector3& p_position,
	                      real p_radius) const;
	
	void setReverbEffect(ReverbEffect p_effect);
	void setReverbVolumeForCategory(Category p_category, real p_normalizedVolume);
	void resetReverbSettings();
	
	/*! \param p_normalizedFadeStartVolume If >= 0, overall volume will fade from this value to p_normalizedVolume
	                                       (only has an effect if p_duration > 0). */
	void setOverallVolume(Device p_device, real p_normalizedVolume, real p_duration, real p_normalizedFadeStartVolume = -1.0f);
	void resetOverallVolume();  //!< Resets the overall device volumes to their defaults
	void setAudioEnabled (Device p_device, bool p_enabled);
	
	void update(real p_deltaTime);
	
	const tt::str::StringSet& getCueNames(const std::string& p_soundBankName) const;
	tt::str::StringSet        getAllCueNames() const;
	
	bool                      isCueLooping(const std::string& p_soundbank, const std::string& p_name) const;
	void                      reloadCueMetaData();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	inline MusicTrackMgr& getMusicTrackMgr() { return m_musicTrackMgr; }
	
	void setLoadingLevel(bool p_loading);
	inline bool isLoadingLevel() const { return m_loadingLevel; }
	
	//tt::audio::player::SoundPlayer* getSoundPlayer() const { return m_sound; }
	
	// NOTE: Volume settings are static members so that they can be loaded/queried before
	// full AudioPlayer initialization is required (which could take a long time, depending
	// on the platform). This allows e.g. the loading music to also use these settings.
	static void loadVolumeSettings();
	static void saveVolumeSettings();
	
	static void setMusicVolumeFromSettings(real p_normalizedVolume);
	static void setSfxVolumeFromSettings  (Category p_category, real p_normalizedVolume);
	
	inline static real getMusicVolumeFromSettings() { return ms_musicVolumeFromSettings; }
	static real getSfxVolumeFromSettings(Category p_category);
	
private:
	typedef tt::math::TimedLinearInterpolation<real> RealTLI;
	
	struct DeviceSettings
	{
		RealTLI overallVolume; // normalized volume
		//bool    audioEnabled;  // whether audio should be output on this device at all
		RealTLI audioEnableFade; // for fading in/out when enabling/disabling
		
		inline DeviceSettings()
		:
		overallVolume(1.0f),
		//audioEnabled(true),
		audioEnableFade(1.0f)
		{ }
	};
	
	struct CueMetaData
	{
		bool isLooping;
	};
	
	// Settings to be applied when level loading is done
	// (this struct will accumulate all setting changes made during load)
	struct PostLevelLoadSettings
	{
		real    categoryVolume[Category_Count];
		RealTLI overallVolume [Device_Count  ];
		bool    audioEnabled  [Device_Count  ];
	};
	
	typedef std::vector<PositionalEffectPtr>          PositionalEffects;
	typedef std::map<std::string, tt::str::StringSet> CueNames;
	typedef std::map<std::string, CueMetaData>        CueMetaDataCollection;
	
	
	explicit AudioPlayer(tt::audio::player::SoundPlayer* p_soundPlayer);
	~AudioPlayer();
	
	void applyOverallVolume (Device   p_device);
	void applyCategoryVolume(Category p_category);
	
	void loadCueMetaData(const std::string& p_filename);
	
	
	tt::audio::player::SoundPlayer* m_sound;
	
	DeviceSettings m_deviceSettings[Device_Count];
	
	real m_categoryVolume[Category_Count]; // normalized volume
	
	real         m_categoryReverbVolume[Category_Count]; // normalized volume
	ReverbEffect m_activeReverbEffect;
	
	mutable tt::thread::Mutex m_containerMutex; // All container members should be accessed/changed within a CriticalSection with this mutex.
	PositionalEffects m_positionalEffects;
	tt::math::Vector3 m_listenerPosition;
	
	// Listing of all cue names in the XACT data; used to produce editor lists (not used internally)
	CueNames m_cueNames;
	
	CueMetaDataCollection m_cueMetaDataCollection;
	
	typedef std::map<SoundCue*, SoundCueWeakPtr> RegisteredSoundCues;
	RegisteredSoundCues m_registeredSoundCues;
	
	typedef std::vector<SoundCuePtr> SoundCues;
	SoundCues m_soundCuesToKeepAliveUntilDonePlaying;
	
	struct AudioCueWithFade
	{
		tt::audio::player::SoundCuePtr cue;
		real                           duration;
		real                           time;
		
		AudioCueWithFade(const tt::audio::player::SoundCuePtr& p_cue,
		                 real                                  p_duration)
		:
		cue(p_cue),
		duration(p_duration),
		time(0.0f)
		{
		}
	};
	typedef std::vector<AudioCueWithFade> TTSoundCues;
	TTSoundCues m_fadeOutAndKeepAliveUntilDone;
	
	MusicTrackMgr m_musicTrackMgr;
	
	bool                  m_loadingLevel;  // whether currently loading a level (delays some operations)
	PostLevelLoadSettings m_postLevelLoadSettings;
	
	static AudioPlayer* ms_instance;
	static bool         ms_createOnMainThreadNow;
	
	static real ms_musicVolumeFromSettings;                // normalized volume; not serialized!
	static real ms_sfxVolumeFromSettings[Category_Count];  // normalized volume; not serialized!
	
	friend class PositionalEffect;
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_AUDIO_AUDIOPLAYER_H)
