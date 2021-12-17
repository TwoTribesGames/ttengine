#ifndef INC_TT_FS_DIR_H
#define INC_TT_FS_DIR_H

#include <tt/fs/DirEntry.h>
#include <tt/fs/fs.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace fs {

class Dir
{
public:
	// basic file functions
	
	/*! \brief Reads from a directory.
	    \param p_entry A reference to a DirEntry object to use.
	    \return Whether the directory information could be read or not.*/
	inline bool read(DirEntry& p_entry)
	{
		return fs::readDir(m_this.lock(), p_entry);
	}
	
	/*! \brief Gets the directory's internal data.
	    \return The directory's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the directory's internal data.
	    \param p_data The data to set. */
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the directory's filesystem identifier.
	    \return The directory's filesystem identifier.*/
	inline identifier getFileSystem() const { return m_identifier; }
	
#ifndef TT_BUILD_FINAL
	/*! \brief Gets the directory's path.
	    \return The directory's path.*/
	inline const std::string& getPath() const { return m_path; }
#else
	/*! \brief Gets the directory's path.
	    \return The directory's path.*/
	inline std::string getPath() const { TT_PANIC("Don't call in Final"); return std::string(); }
#endif
	
private:
	/*! \brief Constructs a directory
	    \param p_path The path of the directory.
	    \param p_identifier The filesystem assigned to the directory.*/
	inline Dir(const std::string& p_path, identifier p_identifier)
	:
#ifndef TT_BUILD_FINAL
	m_path(p_path),
#endif
	m_identifier(p_identifier),
	m_data(0),
	m_this()
	{
		(void)p_path;
	}
	
	/*! \brief Destructs a directory. */
	inline ~Dir()
	{
		fs::closeDir(this);
		if (m_data != 0)
		{
			TT_PANIC("Cleanup failed for %s", getPath().c_str());
		}
	}
	
	Dir(const Dir& p_rhs);
	Dir& operator=(const Dir& p_rhs);
	
	static void deleteDir(Dir* p_dir);
	
	friend DirPtr openDir(const std::string&, const std::string&, identifier);
	friend bool closeDir(Dir*);
	
#ifndef TT_BUILD_FINAL
	std::string m_path;
#endif
	identifier  m_identifier;
	void*       m_data;
	
	tt_ptr<Dir>::weak m_this;
	
};

// namespace end
}
}

#endif // INC_TT_FS_DIR_H
