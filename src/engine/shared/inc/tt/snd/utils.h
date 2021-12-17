#if !defined(INC_TT_SND_UTILS_H)
#define INC_TT_SND_UTILS_H


#include <tt/platform/tt_error.h>
#include <tt/snd/Buffer.h>
#include <tt/snd/Stream.h>
#include <tt/snd/Voice.h>


namespace tt {
namespace snd {

inline bool checkVoiceOpen(const VoicePtr& p_voice)
{
	// Voice is checked for null at higher level (snd.cpp) so it should never be null here
	TT_NULL_ASSERT(p_voice);
	
	if (p_voice->getData() == 0)
	{
		TT_PANIC("Voice not open.");
		return false;
	}
	
	return true;
}


inline bool checkBufferOpen(const BufferPtr& p_buffer)
{
	// Buffer is checked for null at higher level (snd.cpp) so it should never be null here
	TT_NULL_ASSERT(p_buffer);
	
	if (p_buffer->getData() == 0)
	{
		TT_PANIC("Buffer not open.");
		return false;
	}
	
	return true;
}


inline bool checkStreamOpen(const StreamPtr& p_stream)
{
	// Stream is checked for null at higher level (snd.cpp) so it should never be null here
	TT_NULL_ASSERT(p_stream);
	
	if (p_stream->getData() == 0)
	{
		TT_PANIC("Stream not open.");
		return false;
	}
	
	return true;
}

// Namespace end
}
}


#endif  // !defined(INC_TT_SND_UTILS_H)
