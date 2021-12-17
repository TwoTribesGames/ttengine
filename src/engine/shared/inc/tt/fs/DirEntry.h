#ifndef INC_TT_FS_DIRENTRY_H
#define INC_TT_FS_DIRENTRY_H

#include <tt/fs/types.h>
#include <tt/fs/fs.h>


namespace tt {
namespace fs {

class DirEntry
{
public:
	inline DirEntry()
	:
	m_name(),
	m_dir(),
	m_data(0),
	m_isDir(false),
	m_isHidden(false),
	m_size(0),
	m_creation(),
	m_access(),
	m_write(),
	m_permissions()
	{ }
	
	/*! \brief Gets the directory entry's internal data.
	    \return The directory entry's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the directory entry's internal data.
	    \param p_data The data to set.*/
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the directory entry's directory object.
	    \return The directory entry's directory object.*/
	inline const DirPtr& getDir() const { return m_dir; }
	
	/*! \brief Sets the directory entry's directory object.
	    \param p_dir The directory object to set.*/
	inline void setDir(const DirPtr& p_dir) { m_dir = p_dir; }
	
	/*! \brief Gets the directory entry's name.
	    \return The directory entry's name.*/
	inline const std::string& getName() const { return m_name; }
	
	/*! \brief Sets the directory entry's name.
	    \param p_path The directory entry's name.*/
	inline void setName(const std::string& p_name) { m_name = p_name; }
	
	/*! \brief Returns whether or not the directory entry is a directory.
	    \return Whether or not the directory entry is a directory.*/
	inline bool isDirectory() const { return m_isDir; }
	
	/*! \brief Sets whether or not the directory entry is a directory.
	    \param p_isDir Whether or not the directory entry is a directory.*/
	inline void setIsDirectory(bool p_isDir) { m_isDir = p_isDir; }
	
	/*! \brief Returns whether or not the directory entry is hidden.
	    \return Whether or not the directory entry is hidden.*/
	inline bool isHidden() const { return m_isHidden; }
	
	/*! \brief Sets whether or not the directory entry is hidden.
	    \param p_isHiddenr Whether or not the directory entry is hidden.*/
	inline void setIsHidden(bool p_isHidden) { m_isHidden = p_isHidden; }
	
	/*! \brief Returns the size of the directory entry.
	    \return The size of the directory entry.*/
	inline size_type getSize() const { return m_size; }
	
	/*! \brief Sets the size of the directory entry.
	    \param p_size The size of the directory entry.*/
	inline void setSize(size_type p_size) { m_size = p_size; }
	
	/*! \brief Returns the creation time of the directory entry.
	    \return The creation time of the directory entry.*/
	inline time_type getCreationTime() const { return m_creation; }
	
	/*! \brief Sets the creation time of the directory entry.
	    \param p_size The creation time of the directory entry.*/
	inline void setCreationTime(time_type p_creation) { m_creation = p_creation; }
	
	/*! \brief Returns the access time of the directory entry.
	    \return The access time of the directory entry.*/
	inline time_type getAccessTime() const { return m_access; }
	
	/*! \brief Sets the access time of the directory entry.
	    \param p_size The access time of the directory entry.*/
	inline void setAccessTime(time_type p_access) { m_access = p_access; }
	
	/*! \brief Returns the write time of the directory entry.
	    \return The write time of the directory entry.*/
	inline time_type getWriteTime() const { return m_write; }
	
	/*! \brief Sets the access time of the directory entry.
	    \param p_size The access time of the directory entry.*/
	inline void setWriteTime(time_type p_write) { m_write = p_write; }
	
	/*! \brief Returns the permissions of the directory entry.
	    \return The permissions of the directory entry.*/
	inline permission_type getPermissions() const { return m_permissions; }
	
	/*! \brief Sets the permissions of the directory entry.
	    \param p_permissions The permissions of the directory entry.*/
	inline void setPermissions(permission_type p_permissions) { m_permissions = p_permissions; }
	
	/*! \brief Clears this entry's details. */
	inline void clear()
	{
		m_name.clear();
		m_dir.reset();
		m_data        = 0;
		m_isDir       = false;
		m_isHidden    = false;
		m_size        = 0;
		m_creation    = 0;
		m_access      = 0;
		m_write       = 0;
		m_permissions = 0;
	}
	
private:
	// No copying
	DirEntry(const DirEntry&);
	DirEntry& operator=(const DirEntry&);
	
	
	std::string     m_name;
	DirPtr          m_dir;
	void*           m_data;
	bool            m_isDir;
	bool            m_isHidden;
	size_type       m_size;
	time_type       m_creation;
	time_type       m_access;
	time_type       m_write;
	permission_type m_permissions;
};

// Namespace end
}
}

#endif // INC_TT_FS_DIRENTRY_H
