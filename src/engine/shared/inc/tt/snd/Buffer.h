#ifndef INC_TT_SND_BUFFER_H
#define INC_TT_SND_BUFFER_H

#include <tt/snd/snd.h>
#include <tt/snd/types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace snd {

class Buffer
{
public:
	// Parameter functions
	
	inline bool setBufferData(const void* p_data,
	                          size_type   p_frames,
	                          size_type   p_channels,
	                          size_type   p_sampleSize,
	                          size_type   p_sampleRate,
	                          bool        p_ownership = true)
	{
		return snd::setBufferData(m_this.lock(), p_data, p_frames, p_channels, p_sampleSize, p_sampleRate, p_ownership);
	}
	
	/*! \brief Gets the length in samples.
	    \return The length.*/
	inline size_type getLength()
	{
		return snd::getBufferLength(m_this.lock());
	}
	
	/*! \brief Gets the buffer channel count.
	    \return The number of channels.*/
	inline size_type getChannelCount()
	{
		return snd::getBufferChannelCount(m_this.lock());
	}
	
	/*! \brief Gets the buffer sample size in bits.
	    \return The sample size.*/
	inline size_type getSampleSize()
	{
		return snd::getBufferSampleSize(m_this.lock());
	}
	
	/*! \brief Gets the buffer sample rate in Hz.
	    \return The sample rate.*/
	inline size_type getSampleRate()
	{
		return snd::getBufferSampleRate(m_this.lock());
	}
	
	
	// Misc functions
	
	/*! \brief Gets the sound's internal data.
	    \return The sound's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the sound's internal data.
	    \param p_data The data to set. */
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the sound's soundsystem identifier.
	    \return The sound's soundsystem identifier.*/
	inline identifier getSoundSystem() const { return m_identifier; }
	
private:
	/*! \brief Constructs a buffer
	    \param p_identifier The soundsystem assigned to the buffer.*/
	inline Buffer(identifier p_identifier)
	:
	m_identifier(p_identifier),
	m_data(0),
	m_this()
	{
	}
	
	/*! \brief Destructs a buffer. */
	inline ~Buffer()
	{
		snd::closeBuffer(this);
		if (m_data != 0)
		{
			TT_PANIC("Cleanup failed for sound");
		}
	}
	
	Buffer(const Buffer& p_rhs);
	Buffer& operator=(const Buffer& p_rhs);
	
	static void deleteBuffer(Buffer* p_buffer);
	
	friend BufferPtr openBuffer(identifier);
	friend bool closeBuffer(Buffer*);
	
	identifier  m_identifier;
	void*       m_data;
	
	tt_ptr<Buffer>::weak m_this;
	
};

} // namespace end
}

#endif // INC_TT_SND_BUFFER_H
