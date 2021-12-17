#ifndef TT_AUDIO_CHIBI_DXMIXER_H
#define TT_AUDIO_CHIBI_DXMIXER_H

#if !defined(_XBOX)  // not available on Xbox


/** MIXER FOR DirectX: DirectSound **/

#include <Dsound.h>

#include <tt/audio/chibi/XMSoftwareMixer.h>


namespace tt {
namespace audio {
namespace chibi {

class DXMixer : public XMSoftwareMixer
{
public:
	DXMixer(LPDIRECTSOUND8 p_directSound = 0);
	virtual ~DXMixer();
	
	bool isPlaying();
	
	virtual void stop();
	virtual void play();
	
	virtual void pause();
	virtual void resume();
	
	virtual bool update();
	
	virtual void setVolume(real p_volume);
	
private:
	bool isPlayThreadActive();
	
	void setPlayThreadActive(bool active);
	
	bool allocate();
	
	void cleanup();
	
	bool fillBuffer(bool firstHalf);
	
	static unsigned int WINAPI playingThread(LPVOID lpParam);
	
	s32*                 m_internalBuffer;
	s16*                 m_internal16bitBuffer;
	int                  m_internalBufferSize;
	int                  m_internal16bitBufferSize;
	int                  m_internalBufferFrames;
	LPDIRECTSOUND8       m_directSound;
	bool                 m_owner;
	LPDIRECTSOUNDBUFFER8 m_buffer;            //!< Holds a DirectSound buffer.
	CRITICAL_SECTION     m_criticalSection;   //!< Pointer to critical section that protects shared memory.
	HANDLE               m_playThread;        //!< A handle for the play thread.
	bool                 m_firstHalfPlaying;  //!< True if the first half of the buffer is playing.
	LPDIRECTSOUNDNOTIFY8 m_playMarker;        //!< Holds a pointer to a marker that will be used to detect when a point is reached in the play buffer.
	HANDLE               m_firstHalfEvent;    //!< Holds a handle for the event that happens when first half of play buffer is reached
	HANDLE               m_secondHalfEvent;   //!< Holds a handle for the event that happens when second half of play buffer is reached
	HANDLE               m_stopPlaybackEvent; //!< Holds a handle to an event object used to signal that playback should stop
	bool                 m_playbackDone;      //!< Holds if playback is done (only interesting if we aren't looping).
	bool                 m_playThreadActive;  //!< True if the play thread is active.
};

} // namespace end
}
}


#endif  // !defined(_XBOX)

#endif // TT_AUDIO_CHIBI_DXMIXER_H
