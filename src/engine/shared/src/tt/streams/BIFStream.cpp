#include <tt/fs/File.h>
#include <tt/streams/BIFStream.h>


namespace tt {
namespace streams {

BIFStream::BIFStream(const std::string& p_filename, fs::identifier p_type)
:
m_file()
{
	if (fs::fileExists(p_filename, p_type))
	{
		m_file = fs::open(p_filename, fs::OpenMode_Read, p_type);
	}
	
	if (m_file == 0)
	{
		clear(BIOStreamBase::IOState_FailBit);
	}
}


BIFStream::BIFStream(const fs::FilePtr& p_file)
:
m_file(p_file)
{
	if (m_file == 0)
	{
		clear(BIOStreamBase::IOState_FailBit);
	}
}


BIFStream::~BIFStream()
{
}


std::ptrdiff_t BIFStream::readBytes(u8* p_buffer, std::ptrdiff_t p_number)
{
	return static_cast<std::ptrdiff_t>(m_file->read(p_buffer, static_cast<fs::size_type>(p_number)));
}

// Namespace end
}
}
