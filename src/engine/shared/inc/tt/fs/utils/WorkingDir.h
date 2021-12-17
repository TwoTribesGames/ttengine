#ifndef INC_TT_FS_UTILS_WORKINGDIR_H
#define INC_TT_FS_UTILS_WORKINGDIR_H

#include <string>

#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace fs {
namespace utils {

class WorkingDir
{
public:
	WorkingDir(const std::string& p_path, identifier p_identifier = 0)
	:
	m_identifier(p_identifier)
	{
		m_restore = fs::getWorkingDir(m_identifier);
		if (m_restore.empty())
		{
			TT_PANIC("Unable to get working directory.");
		}
		else if (fs::setWorkingDir(p_path, m_identifier) == false)
		{
			TT_PANIC("Unable to set working directory to %s.", p_path.c_str());
		}
	}
	
	~WorkingDir()
	{
		if (m_restore.empty() == false && fs::setWorkingDir(m_restore, m_identifier) == false)
		{
			TT_PANIC("Unable to restore working directory to %s.", m_restore.c_str());
		}
	}
	
	inline bool isValid() const { return m_restore.empty() == false; }
	
private:
	WorkingDir(const WorkingDir&);
	const WorkingDir& operator=(const WorkingDir&);
	
	std::string m_restore;
	identifier  m_identifier;
};

// namespace end
}
}
}


#endif // INC_TT_FS_UTILS_WORKINGDIR_H
