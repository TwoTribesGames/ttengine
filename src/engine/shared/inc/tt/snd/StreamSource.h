#ifndef INC_TT_SND_STREAMSOURCE_H
#define INC_TT_SND_STREAMSOURCE_H

#include <tt/snd/snd.h>
#include <tt/snd/types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace snd {

//! Stream source interface for providing a stream with data.
/*!
    The StreamSource interface provides both the data for a stream as well as
    the parameters required for playback. Sample type and samplerate as well as 
    buffersize will be queried before the initial buffer filling.
*/

class StreamSource
{
public:
	StreamSource() {}
	virtual ~StreamSource() {}
	
	/*! \brief Fills specified buffer. Called periodically from a stream.
	    \param p_frames   Number of frames to fill.
	    \param p_channels Number of channels.
	    \param p_buffer   Array of p_channels pointers to buffers of at least p_frames samples.
	    \param p_notifySourceWhenPlaybackReachesThisFrame_OUT
	             Allows the source to indicate it wishes to be notified if stream playback reaches this frame in the buffer.
	             No notification is sent if this value is a negative number (this is the default).
	    \return Number of frames written.*/
	virtual size_type fillBuffer(size_type  p_frames,
	                             size_type  p_channels,
	                             void**     p_buffer,
	                             size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT) = 0;
	
	/*! \brief Fills specified buffer interleaved. Called periodically from a stream.
	    \param p_frames   Number of frames to fill.
	    \param p_channels Number of channels.
	    \param p_buffer   Pointers to a buffer of at least p_channels * p_frames samples.
	    \param p_notifySourceWhenPlaybackReachesThisFrame_OUT
	             Allows the source to indicate it wishes to be notified if stream playback reaches this frame in the buffer.
	             No notification is sent if this value is a negative number (this is the default).
	    \return Number of frames written.*/
	virtual size_type fillBufferInterleaved(size_type  p_frames,
	                                        size_type  p_channels,
	                                        void*      p_buffer,
	                                        size_type* p_notifySourceWhenPlaybackReachesThisFrame_OUT) = 0;
	
	/*! \brief Returns the size of a single sample in bits.
	    \return The size of a single sample in bits.*/
	virtual size_type getSampleSize() const = 0;
	
	/*! \brief Returns the requested size of the buffer in frames.
	    \return The requested size of the buffer in frames.*/
	virtual size_type getBufferSize() const = 0;
	
	/*! \brief Returns the requested framerate of the buffer in Hz.
	    \return The requested framerate of the buffer Hz.*/
	virtual size_type getFramerate() const = 0;
	
	/*! \brief Returns the requested amount of channels.
	    \return The requested amount of channels.*/
	virtual size_type getChannelCount() const = 0;
	
	/*! \brief Gets called when stream playback reaches (or passes) the playback frame in the stream
	           buffer that was requested during a call to fillBuffer or fillBufferInterleaved. */
	virtual void onStreamReachedNotificationFrame() { }
	
private:
	StreamSource(const StreamSource& p_rhs);
	StreamSource& operator=(const StreamSource& p_rhs);
};

} // namespace end
}

#endif // INC_TT_SND_STREAMSOURCE_H
