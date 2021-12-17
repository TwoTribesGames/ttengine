#ifndef INC_TT_AUDIO_CHIBI_XMFILEIO_H
#define INC_TT_AUDIO_CHIBI_XMFILEIO_H

#include <tt/audio/chibi/types.h>


namespace tt {
namespace audio {
namespace chibi {

class XMFileIO
{
public:
	
	enum IOError
	{
		IOError_Ok,
		IOError_CantOpen,
		IOError_InUse
	};
	
	virtual ~XMFileIO() {}
	
	virtual bool    inUse() const = 0;
	virtual IOError open(const char* p_file, bool p_bigEndian) = 0;
	virtual u8      getU8() = 0;
	virtual u16     getU16() = 0;
	virtual u32     getU32() = 0;
	virtual void    getByteArray(u8* p_dst, u32 p_count) = 0;
	virtual void    seekPos(u32 p_offset) = 0;
	virtual u32     getPos() = 0;
	virtual u32     getLength() = 0;
	virtual void    close() = 0;
	virtual void    decompress() = 0;
};

} // namespace end
}
}

#endif // INC_TT_AUDIO_CHIBI_XMFILEIO_H
