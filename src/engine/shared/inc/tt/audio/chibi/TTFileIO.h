#ifndef INC_TT_AUDIO_CHIBI_TTFILEIO_H
#define INC_TT_AUDIO_CHIBI_TTFILEIO_H

#include <tt/audio/chibi/XMFileIO.h>
#include <tt/fs/types.h>


namespace tt {
namespace audio {
namespace chibi {

class TTFileIO : public XMFileIO
{
public:
	explicit TTFileIO(fs::identifier p_type = 0, size_t p_bufferSize = 1024);
	virtual ~TTFileIO();
	
	virtual bool    inUse() const;
	virtual IOError open(const char* p_file, bool p_bigEndian);
	virtual u8      getU8();
	virtual u16     getU16();
	virtual u32     getU32();
	virtual void    getByteArray(u8* p_dst, u32 p_count);
	virtual void    seekPos(u32 p_offset);
	virtual u32     getPos();
	virtual u32     getLength();
	virtual void    close();
	virtual void    decompress();
	
private:
	void fillBuffer();
	
	// No copying
	TTFileIO(const TTFileIO&);
	TTFileIO& operator=(const TTFileIO&);
	
	
	fs::FilePtr    m_file;
	bool           m_bigEndian;
	fs::identifier m_type;
	u8*            m_buffer;
	u8*            m_content;
	size_t         m_size;
	size_t         m_bufferMaxSize;
	size_t         m_bufferSize;
	size_t         m_bufferPosition;
	
	fs::pos_type m_filePos;
};

// namespace end
}
}
}

#endif // INC_TT_AUDIO_CHIBI_TTFILEIO_H
