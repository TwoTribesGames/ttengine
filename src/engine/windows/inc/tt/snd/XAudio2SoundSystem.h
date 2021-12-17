#if !defined(INC_TT_SND_XAUDIO2SOUNDSYSTEM_H)
#define INC_TT_SND_XAUDIO2SOUNDSYSTEM_H


#include <list>
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
#include <XAudio2.h>
#else
#include <C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include\XAudio2.h>
#endif

#include <tt/snd/SoundSystem.h>
#include <tt/snd/types.h>
#include <tt/thread/Mutex.h>
#include <tt/thread/Semaphore.h>
#include <tt/thread/types.h>


namespace tt {
namespace snd {

class XAudio2SoundSystem : public SoundSystem
{
public:
	static SoundSystemPtr instantiate(identifier p_id, bool p_updateStreamsInThread = false);
	
	virtual ~XAudio2SoundSystem();
	
	// Master volume functions
	virtual real getMasterVolume();
	virtual void setMasterVolume(real p_volume);
	
	// Master suspend functions
	virtual bool suspend();
	virtual bool resume();
	
	// Voice functions
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
	
	// Stream Playback functions
	virtual bool playStream  (const StreamPtr& p_stream);
	virtual bool stopStream  (const StreamPtr& p_stream);
	virtual bool pauseStream (const StreamPtr& p_stream);
	virtual bool resumeStream(const StreamPtr& p_stream);
	virtual bool updateStream(const StreamPtr& p_stream);
	
	// Stream Status functions
	virtual bool isStreamPlaying(const StreamPtr& p_stream);
	virtual bool isStreamPaused (const StreamPtr& p_stream);
	
	// Stream Parameter functions
	virtual real getStreamVolumeRatio(const StreamPtr& p_stream);
	virtual bool setStreamVolumeRatio(const StreamPtr& p_stream, real p_volumeRatio);
	virtual real getStreamVolume     (const StreamPtr& p_stream);
	virtual bool setStreamVolume     (const StreamPtr& p_stream, real p_volume);
	
private:
	struct BufferData;
	struct VoiceData;
	struct StreamData;
	
	
	XAudio2SoundSystem(identifier p_id, bool p_updateStreamsInThread);
	
	bool init();
	bool uninit();
	
	VoiceData*  getVoiceData (const VoicePtr&  p_voice);
	BufferData* getBufferData(const BufferPtr& p_buffer);
	StreamData* getStreamData(const StreamPtr& p_stream);
	
	// No copying
	XAudio2SoundSystem(const XAudio2SoundSystem&);
	XAudio2SoundSystem& operator=(const XAudio2SoundSystem&);
	
	
	IXAudio2*               m_engine; //!< XAudio2 interface.
	IXAudio2MasteringVoice* m_device; //!< Mastering voice interface
	
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


#endif  // !defined(INC_TT_SND_XAUDIO2SOUNDSYSTEM_H)
