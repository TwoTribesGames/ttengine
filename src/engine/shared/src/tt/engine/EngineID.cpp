#include <sstream>
#include <iomanip>

#include <tt/engine/EngineID.h>
#include <tt/engine/file/FileUtils.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {

#if !defined(TT_BUILD_FINAL)

EngineID::EngineID(u32 p_crc1, u32 p_crc2)
:
crc1(p_crc1),
crc2(p_crc2),
m_name("unknown"),
m_namespace("unknown")
{}


EngineID::EngineID(const std::string& p_name, const std::string& p_namespace)
:
crc1(file::FileUtils::getInstance()->getCRC(p_name, 0)),
crc2(file::FileUtils::getInstance()->getCRC(p_namespace, 1)),
m_name(p_name),
m_namespace(p_namespace)
{
}


std::string EngineID::toDebugString() const
{
	std::stringstream s;
	
	if (m_name.empty() == false)
	{
		s << "[Name: " << m_name << "]";
	}
	
	if (m_namespace.empty() == false)
	{
		s << "[Namespace: " << m_namespace << "]";
	}
	
	s << "[CRC: ";
	s << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << crc1;
	s << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << crc2;
	s << "]";
	
	return s.str();
}

#else // Final Build versions

EngineID::EngineID(u32 p_crc1, u32 p_crc2)
:
crc1(p_crc1),
crc2(p_crc2)
{}


EngineID::EngineID(const std::string& p_name, const std::string& p_namespace)
:
crc1(file::FileUtils::getInstance()->getCRC(p_name, 0)),
crc2(file::FileUtils::getInstance()->getCRC(p_namespace, 1))
{
}

#endif  // !defined(TT_BUILD_FINAL)


bool EngineID::load(const fs::FilePtr& p_file)
{
	return (p_file->read(&crc1, sizeof(crc1)) == sizeof(crc1)) &&
	       (p_file->read(&crc2, sizeof(crc2)) == sizeof(crc2));
}


std::string EngineID::toString() const
{
	std::stringstream s;
	s << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << crc1;
	s << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << crc2;
	
	return s.str();
}

// Namespace end
}
}
