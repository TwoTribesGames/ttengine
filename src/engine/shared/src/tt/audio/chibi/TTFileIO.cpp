#include <tt/audio/chibi/TTFileIO.h>
#include <tt/audio/chibi/XMMemoryManager.h>
#include <tt/audio/chibi/XMUtil.h>
#include <tt/compression/compression.h>
#include <tt/fs/File.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>

#include <cstring>

namespace tt {
namespace audio {
namespace chibi {


TTFileIO::TTFileIO(fs::identifier p_type, size_t p_bufferSize)
:
m_file(),
m_bigEndian(false),
m_type(p_type),
m_buffer(0),
m_content(0),
m_size(0),
m_bufferMaxSize(p_bufferSize),
m_bufferSize(0),
m_bufferPosition(0),
m_filePos(0)
{
	m_buffer = new u8[m_bufferMaxSize];
	if (m_buffer == 0)
	{
		TT_PANIC("Unable to allocate %d bytes.", m_bufferMaxSize);
	}
}


TTFileIO::~TTFileIO()
{
	delete[] m_buffer;
}


bool TTFileIO::inUse() const
{
	return (m_file != 0) || (m_content != 0);
}


XMFileIO::IOError TTFileIO::open(const char* p_file, bool p_bigEndian)
{
	if (inUse())
	{
		return XMFileIO::IOError_InUse;
	}
	
	m_file = fs::open(p_file, fs::OpenMode_Read, m_type);
	
	if (m_file == 0)
	{
		return XMFileIO::IOError_CantOpen;
	}
	
	m_bigEndian = p_bigEndian;
	m_filePos = 0;
	fillBuffer();
	
	return XMFileIO::IOError_Ok;
}


u8 TTFileIO::getU8()
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return 0;
	}
	
	if (m_content != 0)
	{
		u8 b = m_content[m_bufferPosition];
		++m_bufferPosition;
		return b;
	}
	
	if (m_bufferPosition >= m_bufferSize)
	{
		fillBuffer();
		if (m_bufferSize == 0)
		{
			return 0;
		}
	}
	u8 b = m_buffer[m_bufferPosition];
	++m_bufferPosition;
	
	return b;
}


u16 TTFileIO::getU16()
{
	u8 a, b;
	u16 c;
	
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return 0;
	}
	
	if (m_bigEndian == false)
	{
		a = getU8();
		b = getU8();
	}
	else
	{
		b = getU8();
		a = getU8();
	}
	
	c = (u16)(((u16)b << 8 ) | a);
	
	return c;
}


u32 TTFileIO::getU32()
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return 0;
	}
	
	u16 a;
	u16 b;
	
	if (m_bigEndian == false)
	{
		a = getU16();
		b = getU16();
	}
	else
	{
		b = getU16();
		a = getU16();
	}
	
	u32 c = ((u32)b << 16 ) | a;
	
	return c;
}


void TTFileIO::getByteArray(u8* p_dst, u32 p_count)
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return;
	}
	
	if (m_content != 0)
	{
		std::memcpy(p_dst, m_content + m_bufferPosition, p_count);
		m_bufferPosition += p_count;
	}
	else
	{
		m_file->seek(static_cast<tt::fs::pos_type>(m_filePos + m_bufferPosition), tt::fs::SeekPos_Set);
		m_file->read(p_dst, static_cast<tt::fs::size_type>(p_count));
		fillBuffer();
	}
}


void TTFileIO::seekPos(u32 p_offset)
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return;
	}
	
	if (m_content != 0)
	{
		m_bufferPosition = p_offset;
	}
	else
	{
		m_file->seek(static_cast<fs::pos_type>(p_offset), fs::SeekPos_Set);
		fillBuffer();
	}
}


u32 TTFileIO::getPos()
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return 0;
	}
	
	if (m_content != 0)
	{
		return static_cast<u32>(m_bufferPosition);
	}
	else
	{
		return static_cast<u32>(m_filePos + m_bufferPosition);
	}
}


u32 TTFileIO::getLength()
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return 0;
	}
	
	if (m_content != 0)
	{
		return static_cast<u32>(m_size);
	}
	else
	{
		return static_cast<u32>(m_file->getLength());
	}
}


void TTFileIO::close()
{
	if (inUse() == false)
	{
		TT_WARN("File Not Open!");
		return;
	}
	
	if (m_content != 0)
	{
		XMUtil::getMemoryManager()->free(m_content, XMMemoryManager::AllocType_Sample);
		m_content = 0;
		m_size = 0;
	}
	
	m_file.reset();
}


void TTFileIO::decompress()
{
	// read remainder of file
	m_file->seek(static_cast<tt::fs::pos_type>(m_filePos + m_bufferPosition), tt::fs::SeekPos_Set);
	u32 size = static_cast<u32>(m_file->getLength() - m_file->getPosition());
	u8* buffer = reinterpret_cast<u8*>(XMUtil::getMemoryManager()->alloc(size, XMMemoryManager::AllocType_Sample));
	m_file->read(buffer, static_cast<fs::size_type>(size));
	m_size = tt::compression::getUncompressedSize(buffer);
	size = math::roundUp(static_cast<u32>(m_size), 4);
	m_content = reinterpret_cast<u8*>(XMUtil::getMemoryManager()->alloc(size, XMMemoryManager::AllocType_Sample));
	tt::compression::uncompressAny(buffer, m_content);
	XMUtil::getMemoryManager()->free(buffer, XMMemoryManager::AllocType_Sample);
	
	m_bufferSize = m_size;
	m_bufferPosition = 0;
}


void TTFileIO::fillBuffer()
{
	m_filePos = m_file->getPosition();
	tt::fs::size_type read = m_file->read(m_buffer, static_cast<tt::fs::size_type>(m_bufferMaxSize));
	m_bufferSize = static_cast<size_t>(read);
	m_bufferPosition = 0;
}

} // namespace end
}
}
