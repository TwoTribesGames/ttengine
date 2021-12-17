#if !defined(INC_TT_SND_OPENALSOUNDSYSTEM_H)
#define INC_TT_SND_OPENALSOUNDSYSTEM_H


#if defined(TT_PLATFORM_OSX) && !defined(TT_FORCE_OPENAL_SOFT)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#if defined(TT_PLATFORM_OSX_MAC)
#include <OpenAL/MacOSX_OALExtensions.h>
#endif
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#endif

#include <list>
#include <tt/math/Vector3.h>
#include <tt/snd/SoundSystem.h>
#include <tt/snd/types.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Semaphore.h>
#include <tt/thread/types.h>


namespace tt {
namespace snd {

class OpenALSoundSystem : public SoundSystem
{
public:
	static SoundSystemPtr instantiate(identifier p_id, bool p_updateStreamsInThread = false);
	
	virtual ~OpenALSoundSystem();
	
	// Master volume functions
	virtual real getMasterVolume();
	virtual void setMasterVolume(real p_volume);
	
	// Master suspend functions
	virtual bool suspend();
	virtual bool resume();
	
	// Voice Creation functions
	virtual bool openVoice (const VoicePtr& p_voice, size_type p_priority);
	virtual bool closeVoice(Voice* p_voice);
	
	// Voice Playback functions
	virtual bool playVoice  (const VoicePtr& p_voice, bool p_loop);
	virtual bool stopVoice  (const VoicePtr& p_voice);
	virtual bool pauseVoice (const VoicePtr& p_voice);
	virtual bool resumeVoice(const VoicePtr& p_voice);
	
	// Voice Status functions
	virtual bool isVoicePlaying(const VoicePtr& p_voice);
	virtual bool isVoicePaused (const VoicePtr& p_voice);
	
	// Voice Parameter functions
	virtual BufferPtr getVoiceBuffer       (const VoicePtr& p_voice);
	virtual bool      setVoiceBuffer       (const VoicePtr& p_voice, const BufferPtr& p_buffer);
	virtual real      getVoicePlaybackRatio(const VoicePtr& p_voice);
	virtual bool      setVoicePlaybackRatio(const VoicePtr& p_voice, real p_playbackRatio);
	virtual real      getVoiceVolumeRatio  (const VoicePtr& p_voice);
	virtual bool      setVoiceVolumeRatio  (const VoicePtr& p_voice, real p_volumeRatio);
	virtual real      getVoicePanning      (const VoicePtr& p_voice);
	virtual bool      setVoicePanning      (const VoicePtr& p_voice, real p_panning);
	virtual real      getVoice360Panning   (const VoicePtr& p_voice);
	virtual bool      setVoice360Panning   (const VoicePtr& p_voice, real p_panning);
	
	virtual bool setVoicePosition(const VoicePtr& p_voice, const math::Vector3& p_position);
	virtual bool setVoiceRadius  (const VoicePtr& p_voice, real p_inner, real p_outer);
	
	virtual bool setVoiceReverbVolume(const VoicePtr& p_voice, real p_volumeInDB);
	virtual bool setReverbEffect(ReverbPreset p_preset);
	
	// Buffer creation functions
	virtual bool openBuffer (const BufferPtr& p_buffer);
	virtual bool closeBuffer(Buffer* p_buffer);
	
	// Buffer Parameter functions
	virtual bool setBufferData(const BufferPtr& p_buffer,
	                           const void*      p_data,
	                           size_type        p_frames,
	                           size_type        p_channels,
	                           size_type        p_sampleSize,
	                           size_type        p_sampleRate,
	                           bool             p_ownership);
	virtual size_type getBufferLength      (const BufferPtr& p_buffer);
	virtual size_type getBufferChannelCount(const BufferPtr& p_buffer);
	virtual size_type getBufferSampleSize  (const BufferPtr& p_buffer);
	virtual size_type getBufferSampleRate  (const BufferPtr& p_buffer);
	
	// Stream functions
	virtual bool openStream (const StreamPtr& p_stream);
	virtual bool closeStream(Stream* p_stream);
	
	// Playback functions
	virtual bool playStream  (const StreamPtr& p_stream);
	virtual bool stopStream  (const StreamPtr& p_stream);
	virtual bool pauseStream (const StreamPtr& p_stream);
	virtual bool resumeStream(const StreamPtr& p_stream);
	virtual bool updateStream(const StreamPtr& p_stream);
	
	// Status functions
	virtual bool isStreamPlaying(const StreamPtr& p_stream);
	virtual bool isStreamPaused (const StreamPtr& p_stream);
	
	// Parameter functions
	virtual real getStreamVolumeRatio(const StreamPtr& p_stream);
	virtual bool setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio);
	virtual real getStreamVolume     (const StreamPtr& p_stream);
	virtual bool setStreamVolume     (const StreamPtr& p_stream, real p_volume);
	
	virtual bool set3DAudioEnabled(bool p_enabled);
	virtual bool setListenerPosition(const math::Vector3& p_position);
	virtual const math::Vector3& getListenerPosition() const;
	virtual bool setPositionalAudioModel(const VoicePtr& p_voice, const audio::xact::RPCCurve* p_curve);
	
private:
	struct VoiceData;
	struct BufferData;
	struct StreamData;
#if defined(OPENAL_RESOURCE_LEAK)
	struct OpenALSource;
#endif
	
	struct FxExtension
	{
		bool supported;  // whether an OpenAL audio effects extension is supported on this system
		                 // (can be OpenAL EFX for non-Apple systems or Apple's ASA for Mac OS X)
		
#if defined(TT_FORCE_OPENAL_SOFT)	
#elif defined(TT_PLATFORM_OSX_MAC)
		
		// Mac OS X doesn't have EFX, but has its own "Apple Spatial Audio" extension instead
		alcASASetListenerProcPtr alcASASetListener;
		alcASAGetListenerProcPtr alcASAGetListener;
		alcASASetSourceProcPtr   alcASASetSource;
		alcASAGetSourceProcPtr   alcASAGetSource;
		
		ALCenum enumALC_ASA_REVERB_ON;
		ALCenum enumALC_ASA_REVERB_QUALITY;
		ALCenum enumALC_ASA_REVERB_ROOM_TYPE;
		ALCenum enumALC_ASA_REVERB_SEND_LEVEL;
		ALCenum enumALC_ASA_REVERB_GLOBAL_LEVEL;
		
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
		
		// Function pointers to EFX extension functions
		// - Effect objects
		LPALGENEFFECTS    alGenEffects;
		LPALDELETEEFFECTS alDeleteEffects;
		LPALISEFFECT      alIsEffect;
		LPALEFFECTI       alEffecti;
		LPALEFFECTIV      alEffectiv;
		LPALEFFECTF       alEffectf;
		LPALEFFECTFV      alEffectfv;
		LPALGETEFFECTI    alGetEffecti;
		LPALGETEFFECTIV   alGetEffectiv;
		LPALGETEFFECTF    alGetEffectf;
		LPALGETEFFECTFV   alGetEffectfv;
		
		// - Filter objects
		LPALGENFILTERS    alGenFilters;
		LPALDELETEFILTERS alDeleteFilters;
		LPALISFILTER      alIsFilter;
		LPALFILTERI       alFilteri;
		LPALFILTERIV      alFilteriv;
		LPALFILTERF       alFilterf;
		LPALFILTERFV      alFilterfv;
		LPALGETFILTERI    alGetFilteri;
		LPALGETFILTERIV   alGetFilteriv;
		LPALGETFILTERF    alGetFilterf;
		LPALGETFILTERFV   alGetFilterfv;
		
		// - Auxiliary slot object
		LPALGENAUXILIARYEFFECTSLOTS    alGenAuxiliaryEffectSlots;
		LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
		LPALISAUXILIARYEFFECTSLOT      alIsAuxiliaryEffectSlot;
		LPALAUXILIARYEFFECTSLOTI       alAuxiliaryEffectSloti;
		LPALAUXILIARYEFFECTSLOTIV      alAuxiliaryEffectSlotiv;
		LPALAUXILIARYEFFECTSLOTF       alAuxiliaryEffectSlotf;
		LPALAUXILIARYEFFECTSLOTFV      alAuxiliaryEffectSlotfv;
		LPALGETAUXILIARYEFFECTSLOTI    alGetAuxiliaryEffectSloti;
		LPALGETAUXILIARYEFFECTSLOTIV   alGetAuxiliaryEffectSlotiv;
		LPALGETAUXILIARYEFFECTSLOTF    alGetAuxiliaryEffectSlotf;
		LPALGETAUXILIARYEFFECTSLOTFV   alGetAuxiliaryEffectSlotfv;
		
#endif
		
		FxExtension();
		void init(ALCdevice* p_device);
	};
	
	
	OpenALSoundSystem(identifier p_id, bool p_updateStreamsInThread);
	
	OpenALSoundSystem(const OpenALSoundSystem& p_rhs);
	OpenALSoundSystem& operator=(const OpenALSoundSystem& p_rhs);
	
	bool   allocateSources();
	bool   freeSources();
	ALuint allocateSource();
	void   freeSource(ALuint p_source);
	
	VoiceData*  getVoiceData (const VoicePtr&  p_voice);
	BufferData* getBufferData(const BufferPtr& p_buffer);
	StreamData* getStreamData(const StreamPtr& p_stream);
	
	
	ALCdevice*    m_device;
	ALCcontext*   m_context;
#if defined(OPENAL_RESOURCE_LEAK)
	OpenALSource* m_sources;
#endif
	math::Vector3 m_listenerPosition;
	
	ReverbPreset m_activeReverbPreset;
	FxExtension  m_fx;
#if defined(TT_FORCE_OPENAL_SOFT)	
#elif defined(TT_PLATFORM_OSX_MAC)
	// FIXME: Need custom Apple "ASA" extensions for reverb here...
#elif !defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_LNX)
	ALuint      m_effectSlot;  // OpenAL auxiliary effect slot
	ALuint      m_effect;      // OpenAL EFX effect
#endif
	
	const bool m_updateStreamsInThread;
	
	////////////////////////////////////
	// Stream update/decoding thread
	
	static int staticDecodeStreamsThread(void* p_soundSystem);
	void decodeStreams();
	
	typedef std::list<Stream*> StreamList;
	tt::thread::handle        m_decodeThread;
	bool                      m_decodeThreadShouldExit;
	StreamList                m_streamsToAdd;
	StreamList                m_activeStreams;
	Stream*                   m_streamToRemove;
	thread::Mutex             m_decodeMutex;
	thread::OptionalSemaphore m_decodeSemaphore;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SND_OPENALSOUNDSYSTEM_H)
