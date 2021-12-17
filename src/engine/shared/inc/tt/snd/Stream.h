#ifndef INC_TT_SND_STREAM_H
#define INC_TT_SND_STREAM_H

#include <tt/snd/snd.h>
#include <tt/snd/types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace snd {

class Stream
{
public:
	// Playback functions
	
	/*! \brief Plays the stream.
	    \return false on fail, true on success.*/
	inline bool play()
	{
		return snd::playStream(m_this.lock());
	}
	
	/*! \brief Stops the stream.
	    \return false on fail, true on success.*/
	inline bool stop()
	{
		return snd::stopStream(m_this.lock());
	}
	
	/*! \brief Pauses the stream.
	    \return false on fail, true on success.*/
	inline bool pause()
	{
		return snd::pauseStream(m_this.lock());
	}
	
	/*! \brief Resumes the stream.
	    \return false on fail, true on success.*/
	inline bool resume()
	{
		return snd::resumeStream(m_this.lock());
	}
	
	/*! \brief Update the stream.
	    \return false on fail, true on success.*/
	inline bool update()
	{
		return snd::updateStream(m_this.lock());
	}
	
	
	// Status functions
	
	/*! \brief Returns whether the stream is playing.
	    \return false when stopped, true when playing.*/
	inline bool isPlaying()
	{
		return snd::isStreamPlaying(m_this.lock());
	}
	
	/*! \brief Returns whether the stream is paused.
	    \return false when not paused, true when paused.*/
	inline bool isPaused()
	{
		return snd::isStreamPaused(m_this.lock());
	}
	
	
	// Parameter functions
	
	/*! \brief Gets the stream's volume ratio.
	    \return The volume ratio.*/
	inline real getVolumeRatio()
	{
		return snd::getStreamVolumeRatio(m_this.lock());
	}
	
	/*! \brief Sets the stream's volume ratio.
	    \param p_volumeRatio The volume ratio of the stream.
	    \return Whether the operation succeeded.*/
	inline bool setVolumeRatio(real p_volumeRatio)
	{
		return snd::setStreamVolumeRatio(m_this.lock(), p_volumeRatio);
	}
	
	/*! \brief Gets the stream's volume in dB.
	    \return The volume in dB.*/
	inline real getVolume()
	{
		return snd::getStreamVolume(m_this.lock());
	}
	
	/*! \brief Sets the stream's volume in dB.
	    \param p_volume The volume of the stream.
	    \return Whether the operation succeeded.*/
	inline bool setVolume(real p_volume)
	{
		return snd::setStreamVolume(m_this.lock(), p_volume);
	}
	
	
	// Misc functions
	
	/*! \brief Gets the stream's internal data.
	    \return The stream's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the stream's internal data.
	    \param p_data The data to set. */
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the stream's soundsystem identifier.
	    \return The stream's soundsystem identifier.*/
	inline identifier getSoundSystem() const { return m_identifier; }
	
	/*! \brief Gets the stream's source.
	    \return The stream's source.*/
	inline StreamSource* getSource() const { return m_source; }
	
private:
	/*! \brief Constructs a stream
	    \param p_identifier The soundsystem assigned to the stream.
	    \param p_source The source of the stream.*/
	inline Stream(identifier p_identifier, StreamSource* p_source)
	:
	m_identifier(p_identifier),
	m_data(0),
	m_source(p_source),
	m_this()
	{
	}
	
	/*! \brief Destructs a file. */
	inline ~Stream()
	{
		snd::closeStream(this);
		if (m_data != 0)
		{
			TT_PANIC("Cleanup failed for stream");
		}
	}
	
	Stream(const Stream& p_rhs);
	Stream& operator=(const Stream& p_rhs);
	
	static void deleteStream(Stream* p_stream);
	
	friend StreamPtr openStream(StreamSource*, identifier);
	friend bool closeStream(Stream*);
	
	identifier    m_identifier;
	void*         m_data;
	StreamSource* m_source;
	
	tt_ptr<Stream>::weak m_this;
	
};

} // namespace end
}

#endif // INC_TT_SND_STREAM_H
