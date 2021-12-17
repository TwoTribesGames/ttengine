#include <tt/fs/File.h>
#include <tt/streams/BOFStream.h>


namespace tt {
namespace streams {


BOFStream::BOFStream(const std::string& p_filename, fs::identifier p_type)
:
m_file(fs::open(p_filename, fs::OpenMode_Write, p_type))
{
	if (m_file == 0)
	{
		clear(BIOStreamBase::IOState_FailBit);
	}
}


BOFStream::BOFStream(const fs::FilePtr& p_file)
:
m_file(p_file)
{
	if (m_file == 0)
	{
		clear(BIOStreamBase::IOState_FailBit);
	}
}


BOFStream::~BOFStream()
{
}


std::ptrdiff_t BOFStream::writeBytes(const u8* p_buffer, std::ptrdiff_t p_number)
{
	return static_cast<std::ptrdiff_t>(m_file->write(p_buffer, static_cast<fs::size_type>(p_number)));
}

// Namespace end
}
}
