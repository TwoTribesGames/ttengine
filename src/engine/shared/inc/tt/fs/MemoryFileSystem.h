#ifndef INC_TT_FS_MEMORYFILESYSTEM_H
#define INC_TT_FS_MEMORYFILESYSTEM_H

#include <map>
#include <vector>

#include <tt/fs/types.h>
#include <tt/fs/PassThroughFileSystem.h>


namespace tt {
namespace fs {

class MemoryFileSystem : public PassThroughFileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier,
	                                 fs::identifier p_source);
	
	virtual ~MemoryFileSystem();
	
	// Basic file functions
	virtual bool open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	virtual bool close(File* p_file);
	virtual size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length);
	virtual size_type readAsync(const FilePtr& p_file,
	                            void*          p_buffer,
	                            size_type      p_length,
	                            callback       p_callback,
	                            void*          p_arg);
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);
	virtual size_type writeAsync(const FilePtr& p_file,
	                             const void*    p_buffer,
	                             size_type      p_length,
	                             callback       p_callback,
	                             void*          p_arg);
	virtual bool flush(const FilePtr& p_file);
	
	// Content functions
	virtual code::BufferPtr getFileContent(const std::string& p_path);
	virtual code::BufferPtr getFileContent(const FilePtr& p_file);
	
	// Asynchronous status functions
	virtual bool isBusy     (const FilePtr& p_file);
	virtual bool isSucceeded(const FilePtr& p_file);
	virtual bool wait       (const FilePtr& p_file);
	virtual bool cancel     (const FilePtr& p_file);
	
	// Position / size functions
	virtual bool      seek       (const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	virtual bool      seekToBegin(const FilePtr& p_file);
	virtual bool      seekToEnd  (const FilePtr& p_file);
	virtual pos_type  getPosition(const FilePtr& p_file);
	virtual size_type getLength  (const FilePtr& p_file);
	
	// Time functions
	virtual time_type getCreationTime(const FilePtr& p_file);
	virtual time_type getAccessTime  (const FilePtr& p_file);
	virtual time_type getWriteTime   (const FilePtr& p_file);
	virtual bool      setWriteTime   (const FilePtr& p_file, time_type p_time);
	
	// Create / Destroy functions
	virtual bool fileExists (const std::string& p_path);
	
	// Memory file/archive functions
	
	/*! \brief Adds a memory archive.
	    \param p_path The archive to add.
	    \return false when the archive could not be added, true when it does.*/
	static bool addMemoryArchive(MemoryArchive* p_archive);
	
	/*! \brief Removes a memory archive.
	    \param p_path The archive to remove.
	    \return false when the archive could not be removed, true when it does.*/
	static bool removeMemoryArchive(MemoryArchive* p_archive);
	
	// FIXME: Get rid of these static variables
	inline static bool isInstantiated() { return ms_isInstantiated; }
	inline static s32  getID()          { return ms_id; }
	
private:
	MemoryFileSystem(identifier p_id, identifier p_source);
	
	MemoryFileSystem(const MemoryFileSystem& p_rhs);
	MemoryFileSystem& operator=(const MemoryFileSystem& p_rhs);
	
	typedef std::vector<MemoryArchive*> Archives;
	
	static Archives ms_archives;
	static bool     ms_isInstantiated;
	static s32      ms_id;
};

// namespace end
}
}

#endif // INC_TT_FS_MEMORYFILESYSTEM_H
