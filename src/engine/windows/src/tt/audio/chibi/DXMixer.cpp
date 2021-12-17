#if !defined(_XBOX)  // not available on Xbox

#include <cmath>
#include <process.h>

#include <tt/audio/helpers.h>
#include <tt/audio/chibi/DXMixer.h>
#include <tt/code/helpers.h>
#include <tt/engine/renderer/DXUT/DXUT.h>
#include <tt/engine/renderer/directx.h>
#include <tt/math/math.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace audio {
namespace chibi {

// Safe resource deallocation helper
template<typename T> inline void SafeCloseHandle(T& h)
{
	if (h != 0)
	{
		CloseHandle(h);
		h = 0;
	}
}


DXMixer::DXMixer(LPDIRECTSOUND8 p_directSound)
:
XMSoftwareMixer(44100, 32),
m_directSound(p_directSound),
m_owner(p_directSound == 0),
m_firstHalfPlaying(true),
m_buffer(0),
m_playMarker(0),
m_firstHalfEvent(0),
m_secondHalfEvent(0),
m_playThread(0),
m_playThreadActive(false),
m_stopPlaybackEvent(0),
m_playbackDone(false)
{
	if (m_owner)
	{
		if(FAILED(DirectSoundCreate8(NULL, &m_directSound, NULL)))
		{
			TT_WARN("Unable to create DirectSound device.");
		}
		else
		{
			if(FAILED(m_directSound->SetCooperativeLevel(DXUTGetHWND(), DSSCL_NORMAL)))
			{
				TT_WARN("Unable to set DirectSound cooperative level.");
			}
		}
	}
	
	//----- create critical section to guard shared memory
	InitializeCriticalSection(&m_criticalSection);
}


DXMixer::~DXMixer()
{
	// Stop playing the file
	stop();
	
	// Delete the critical section (safe to do now since the play thread
	// is guaranteed dead)
	DeleteCriticalSection(&m_criticalSection);
	
	// clean up device
	if (m_owner)
	{
		IDirectSound_Release(m_directSound);
	}
}


/*! \returns true if the Ogg is playing, otherwise false. */
bool DXMixer::isPlaying()
{
	return isPlayThreadActive();
}


/*! Stops the Ogg Vorbis file from playing.
    If the Ogg Vorbis file was not playing, nothing happens. */
void DXMixer::stop()
{
	//----- Safeguard
	if ( isPlaying() == false )
	{
		return;
	}
	
	//----- Signal the play thread to stop
	SetEvent(m_stopPlaybackEvent);
	
	// Wait for playing thread to exit
	if (WaitForSingleObject(m_playThread, 1000) == WAIT_ABANDONED)
	{
		TT_WARN("Forcibly killing play thread.");
		
		// The thread hasn't terminated as expected. Kill it
		TerminateThread(m_playThread, 1);
		
		// Not playing any more
		setPlayThreadActive(false);
		
		// Since playing thread has not cleaned up, this thread will have to
		cleanup();
		
		// TODO: You should report this error here somehow
	}
	
	//----- store that we are not playing any longer
	setPlayThreadActive(false);
}


void DXMixer::setVolume(real p_volume)
{
	real dB = audio::helpers::ratioTodB(p_volume);
	LONG volume = static_cast<LONG>(dB * 100);
	if (volume > DSBVOLUME_MAX)
	{
		volume = DSBVOLUME_MAX;
	}
	if (volume < DSBVOLUME_MIN)
	{
		volume = DSBVOLUME_MIN;
	}
	
	if (m_buffer != 0)
	{
		m_buffer->SetVolume(volume);
	}
}


void DXMixer::play()
{
	// Stop playback if already going on
	stop();
	
	// Mark that thread is now active
	setPlayThreadActive(true);
	
	//----- Create play thread
	
	// Temp storage for thread ID, which is not needed
	unsigned int threadID;
	
	// Create thread and store handle to thread
	m_playThread = (HANDLE)_beginthreadex(0, 0, DXMixer::playingThread,
	                                      this, 0, &threadID);
}


void DXMixer::pause()
{
	if (isPlaying() == false)
	{
		return;
	}
	
	if (FAILED(m_buffer->Stop()))
	{
		TT_WARN("Pausing the buffer failed!\n");
	}
}


void DXMixer::resume()
{
	if (isPlaying())
	{
		return;
	}
	
	if (FAILED(m_buffer->Play(0, 0, DSBPLAY_LOOPING)))
	{
		TT_WARN("Resuming the buffer failed!\n");
	}
}


bool DXMixer::update()
{
	return true;
}


// Private functions

/*! \returns True if the playing thread is currently running, otherwise false */
bool DXMixer::isPlayThreadActive()
{
	EnterCriticalSection(&m_criticalSection);
	bool active = m_playThreadActive;
	LeaveCriticalSection(&m_criticalSection);
	
	return active;
}


/*! \param active True if you want isPlayThreadActive() to return true,
                  or false if you want it to return false. */
void DXMixer::setPlayThreadActive(bool active)
{
	EnterCriticalSection(&m_criticalSection);
	m_playThreadActive = active;
	LeaveCriticalSection(&m_criticalSection);
}


/*!
Called internally to set up the DirectSound secondary buffer,
check that the Ogg Vorbis file is present, that it contains
Vorbis encoded audio and to set up notification events for
streaming the decoded audio into the DirectSound buffer.

play() starts up a play thread that calls this function.
*/
bool DXMixer::allocate()
{
	//----- Set up buffer to hold data
	m_internalBufferFrames = 2048;
	m_internalBufferSize = m_internalBufferFrames * sizeof(s32) * 2;
	m_internal16bitBufferSize = m_internalBufferFrames * sizeof(s16) * 2;
	m_internalBuffer = new s32[m_internalBufferFrames * 2];
	m_internal16bitBuffer = new s16[m_internalBufferFrames * 2];
	
	// Get a wave format structure, which we will use later to create the DirectSound buffer description
	WAVEFORMATEX waveFormat;
	memset(&waveFormat, 0, sizeof(waveFormat));
	
	// Set up wave format structure
	waveFormat.cbSize          = sizeof(waveFormat);                                   // How big this structure is
	waveFormat.nChannels       = static_cast<WORD>(2);                                 // Stereo
	waveFormat.wBitsPerSample  = 16;                                                   // PCM 16 bit
	waveFormat.nSamplesPerSec  = 44100;                                                // Sampling rate (44 KHz)
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nChannels * 2; // Average bytes per second
	waveFormat.nBlockAlign     = 2 * waveFormat.nChannels;                             // What block boundaries exist
	waveFormat.wFormatTag      = WAVE_FORMAT_PCM;                                      // PCM
	
	//----- Prepare DirectSound buffer description
	
	// Get a buffer description
	DSBUFFERDESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(DSBUFFERDESC));
	
	// Set up buffer description structure
	bufferDesc.dwSize        = sizeof(DSBUFFERDESC);                              // How big this structure is
	bufferDesc.lpwfxFormat   = &waveFormat;                                       // Information about the sound that the buffer will contain
	bufferDesc.dwBufferBytes = m_internal16bitBufferSize * 2;                     // Total buffer size = 2 * half size
	bufferDesc.dwFlags       = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLVOLUME;  // Buffer must support notifications
	
	//----- Create sound buffer
	
	// Pointer to old interface, used to obtain pointer to new interface
	LPDIRECTSOUNDBUFFER tempBuffer = 0;
	
	// Create old interface
	if (FAILED(m_directSound->CreateSoundBuffer(&bufferDesc, &tempBuffer, 0)))
	{
		TT_WARN("Creating DirectSound sound buffer failed.");
		
		// Return error
		return false;
	}
	
	// Query for updated interface
	if (FAILED(tempBuffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&m_buffer)))
	{
		TT_WARN("Querying IDirectSoundBuffer8 interface failed.");
		
		// Return error
		return false;
	}
	
	// Release old, temp interface
	safeRelease(tempBuffer);
	
	
	//----- Set up the two half play markers
	
	// First get notification interface
	if (FAILED(m_buffer->QueryInterface(IID_IDirectSoundNotify8, (void**)&m_playMarker)))
	{
		TT_WARN("Querying IDirectSoundNotify8 interface failed.");
		
		// Return error
		return false;
	}
	
	// Get two notification structures used to set up the play markers
	DSBPOSITIONNOTIFY notificationMarker[2];
	
	// Create notification event objects
	m_firstHalfEvent  = CreateEvent(0, FALSE, FALSE, 0);
	m_secondHalfEvent = CreateEvent(0, FALSE, FALSE, 0);
	
	// Set up position for first event object to be fired
	notificationMarker[0].dwOffset     = 0;
	notificationMarker[0].hEventNotify = m_firstHalfEvent;
	
	// Set up position for second event object to be fired
	notificationMarker[1].dwOffset     = m_internal16bitBufferSize;
	notificationMarker[1].hEventNotify = m_secondHalfEvent;
	
	// Now actually set the notification points
	if (FAILED(m_playMarker->SetNotificationPositions(2, notificationMarker)))
	{
		TT_WARN("Setting play notification positions failed.");
		
		// Return error
		return false;
	}
	
	//----- Create notification event object used to signal when playback should stop
	m_stopPlaybackEvent = CreateEvent(0, FALSE, FALSE, 0);
	
	//----- Fill first half of buffer with initial data
	fillBuffer(true);
	
	//----- Success
	return true;
}


/*! Closes the Ogg Vorbis file, deallocates all DirectSound interfaces and closes all event handles. */
void DXMixer::cleanup()
{
	// Release player marker
	safeRelease(m_playMarker);
	
	// Close first half event handle
	SafeCloseHandle(m_firstHalfEvent);
	
	// Close second half event handle
	SafeCloseHandle(m_secondHalfEvent);
	
	// Close stop playback event handle
	SafeCloseHandle(m_stopPlaybackEvent);
	
	// Release DirectSound buffer
	safeRelease(m_buffer);
	
	tt::code::helpers::safeDeleteArray(m_internalBuffer);
	tt::code::helpers::safeDeleteArray(m_internal16bitBuffer);
}


bool DXMixer::fillBuffer(bool p_firstHalf)
{
	XMUtil::getMemoryManager()->zeroMem(m_internalBuffer, m_internalBufferSize);
	
	if ( mixToBuffer(m_internalBuffer, m_internalBufferFrames) != 
	     static_cast<u32>(m_internalBufferFrames) )
	{
		m_playbackDone = true;
	}
	
	// shift down to 16 bit samples (2 samples per frame)
	for ( int i = 0; i < m_internalBufferFrames * 2; ++i )
	{
		m_internal16bitBuffer[i] = static_cast<s16>(m_internalBuffer[i]);
	}
	
	//----- Initial allocations
	LPVOID firstSegment;          // Holds a pointer to the first segment that we lock
	DWORD  firstSegmentSize  = 0; // Holds how big the first segment is
	LPVOID secondSegment;         // Holds a pointer to the second segment that we lock
	DWORD  secondSegmentSize = 0; // Holds how big the second segment is
	
	//----- Lock DirectSound buffer half
	if (FAILED(m_buffer->Lock((p_firstHalf ? 0 : m_internal16bitBufferSize), // If we are locking the first half, lock from 0, otherwise lock from BUFFER_HALF_SIZE
	                          m_internal16bitBufferSize,                     // How big a chunk of the buffer to block
	                          &firstSegment,                                 // Pointer that will receive the locked segment start address
	                          &firstSegmentSize,                             // Will return how big the first segment is (should always be BUFFER_HALF_SIZE)
	                          &secondSegment,                                // Pointer that will receive the second locked segment start address (in case of wrapping)
	                          &secondSegmentSize,                            // How big a chunk we wrapped with (in case of wrapping)
	                          0)))                                           // Flags: no extra settings
	{
		TT_WARN("Locking DirectSound buffer failed.");
		
		// Return error
		return false;
	}
	
	//----- Debug safety: we should always have locked a complete segment of the size we requested
	TT_ASSERT(firstSegmentSize == static_cast<DWORD>(m_internal16bitBufferSize));
	
	memcpy(firstSegment, m_internal16bitBuffer, m_internal16bitBufferSize);
	
	//----- Unlock buffer
	m_buffer->Unlock(firstSegment, firstSegmentSize, secondSegment, secondSegmentSize);
	
	return true;
}


unsigned int DXMixer::playingThread(LPVOID param)
{
	// Cast thread parameter to a OggVorbisPlayer pointer
	DXMixer* playerInstance = static_cast<DXMixer*>(param);
	
	// Allocate all resources
	if ( playerInstance->allocate() == false )
	{
		TT_WARN("Allocating resources failed.");
		playerInstance->cleanup();
		return 1;
	}
	
	//----- Allocations
	bool errorOccurred = false; // Assume everything will go ok
	
	//----- Indicate that we are playing
	playerInstance->m_playbackDone = false;
	
	//----- Start playing the buffer (looping because we are going to refill the buffer)
	if (FAILED(playerInstance->m_buffer->Play(0, 0, DSBPLAY_LOOPING)))
	{
		TT_WARN("Playing the buffer failed!\n");
	}
	
	//----- Go into loop waiting on notification event objects
	
	// Create tracker of what half we have are playing
	//bool playingFirstHalf = true;
	
	// Create array of event objects
	HANDLE eventObjects[] =
	{
		playerInstance->m_firstHalfEvent,   // Push event object for reaching first half
		playerInstance->m_secondHalfEvent,  // Push event object for reaching second half
		playerInstance->m_stopPlaybackEvent // Push event object used to signal for playback stop
	};
	DWORD numObjects = sizeof(eventObjects) / sizeof(HANDLE);
	
	// Keep going in the loop until we need to stop
	bool continuePlaying       = true;  // Used to keep track of when to stop the while loop
	bool playbackDoneProcessed = false; // Used ot only process m_playbackDone once
	int  stopAtNextHalf        = 0;     // 0 signals "do not stop",
	                                    // 1 signals "stop at first half",
	                                    // 2 signals "stop at second half"
	
	// Enter refill loop
	while ( continuePlaying && (errorOccurred == false) )
	{
		switch (WaitForMultipleObjects(numObjects, eventObjects, FALSE, INFINITE))
		{
		//----- First half was reached
		case WAIT_OBJECT_0:
			// Check if we should stop playing back
			if (stopAtNextHalf == 1)
			{
				// Stop playing
				continuePlaying = false;
				
				// Leave and do not fill the next buffer half
				break;
			}
			
			// Fill second half with sound
			if ( (playerInstance->fillBuffer(false)) == false )
			{
				TT_WARN("Filling the second half of the buffer failed!");
				errorOccurred = true;
			}
			
			// If the last fill was the final fill, we should stop next time we reach this half (i.e. finish playing whatever audio we do have)
			if ((playerInstance->m_playbackDone) && (!playbackDoneProcessed))
			{
				// Make the while loop stop after playing the next half of the buffer
				stopAtNextHalf = 1;
				
				// Indicate that we have already processed the playback done flag
				playbackDoneProcessed = true;
			}
			break;
			
		//----- Second half was reached
		case WAIT_OBJECT_0 + 1:
			// Check if we should stop playing back
			if (stopAtNextHalf == 2)
			{
				// Stop playing
				continuePlaying = false;
				
				// Leave and do not fill the next buffer half
				break;
			}
			
			// Fill first half with sound
			if (!(playerInstance->fillBuffer(true)))
			{
				TT_WARN("Filling the first half of the buffer failed!");
				errorOccurred = true;
			}
			
			// If this last fill was the final fill, we should stop next time we reach this half (i.e. finish playing whatever audio we do have)
			if ((playerInstance->m_playbackDone) && (!playbackDoneProcessed))
			{
				// Make the while loop stop after playing the next half of the buffer
				stopAtNextHalf = 2;
				
				// Indicate that we have already processed the playback done flag
				playbackDoneProcessed = true;
			}
			break;
			
		//----- Stop event has happened
		case WAIT_OBJECT_0 + 2:
			// Exit the while loop
			continuePlaying = false;
			break;
		}
	}
	
	//----- Stop the buffer from playing
	if (FAILED(playerInstance->m_buffer->Stop()))
	{
		TT_WARN("Stopping the buffer failed!");
	}
	
	//----- Thread no longer active
	playerInstance->setPlayThreadActive(false);
	
	//----- Perform cleanup
	playerInstance->cleanup();
	
	//----- Done
	return (errorOccurred ? 1 : 0);
}


} // namespace end
}
}

#endif  // !defined(_XBOX)  // not available on Xbox
