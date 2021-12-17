#ifndef INC_TT_AUDIO_CHIBI_XMCOMPRESSOR_H
#define INC_TT_AUDIO_CHIBI_XMCOMPRESSOR_H

#include <tt/audio/chibi/types.h>


namespace tt {
namespace audio {
namespace chibi {

class XMCompressor
{
public:
	XMCompressor();
	~XMCompressor();
	
	/*! \brief Compresses a pattern.
	    \param p_rows Number of rows in the pattern.
	    \param p_channels Number of channels in the pattern.
	    \param p_src Pattern data.
	    \param p_dest Destination buffer, ignored when 0, must be large enough otherwise.
	    \return Size of the compressed pattern in bytes.*/
	u32 compressPattern(int p_rows, int p_channels, XMNote* p_src, u8* p_dest);
	
private:
	
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMCOMPRESSOR_H
