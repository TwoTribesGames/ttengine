#if !defined(INC_TT_FS_WINDOWSFILESYSTEM_H)
#define INC_TT_FS_WINDOWSFILESYSTEM_H


#include <tt/fs/types.h>
#include <tt/fs/FileSystem.h>


namespace tt {
namespace fs {

/*! \brief File system support for Windows and Xbox 360.
    \note Supported features differ between Windows and Xbox 360. */
class WindowsFileSystem : public FileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier, const std::string& p_appName);
	
	virtual ~WindowsFileSystem();
	
	// Feature support check functions
	virtual bool supportsSaving();
	virtual bool supportsDirectories();
	
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
	
	// Asynchronous status functions
	virtual bool isBusy     (const FilePtr& p_file);
	virtual bool isSucceeded(const FilePtr& p_file);
	virtual bool wait       (const FilePtr& p_file);
	virtual bool cancel     (const FilePtr& p_file);
	
	// Position / size functions
	virtual bool      seek       (const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	virtual pos_type  getPosition(const FilePtr& p_file);
	virtual size_type getLength  (const FilePtr& p_file);
	
	// Time functions
	virtual time_type getCreationTime(const FilePtr& p_file);
	virtual time_type getAccessTime  (const FilePtr& p_file);
	virtual time_type getWriteTime   (const FilePtr& p_file);
	virtual bool      setWriteTime   (const FilePtr& p_file, time_type p_time);
	virtual s64       convertToUnixTime  (time_type p_nativeTime);
	virtual time_type convertToNativeTime(s64       p_unixTime);
	
	// Directory functions
	virtual bool openDir (DirPtr& p_dir, const std::string& p_path, const std::string& p_filter);
	virtual bool closeDir(Dir* p_dir);
	virtual bool readDir (const DirPtr& p_dir, DirEntry& p_entry);
	
	// Create / Destroy functions
	virtual bool fileExists (const std::string& p_path);
	virtual bool destroyFile(const std::string& p_path);
	virtual bool copyFile   (const std::string& p_existingFile, const std::string& p_newFile, bool p_failIfExists);
	virtual bool moveFile   (const std::string& p_existingFile, const std::string& p_newFile, bool p_failIfExists);
	virtual bool dirExists  (const std::string& p_path);
	virtual bool createDir  (const std::string& p_path);
	virtual bool destroyDir (const std::string& p_path);
	
	// Working Directory functions
	virtual std::string getWorkingDir();
	virtual bool        setWorkingDir(const std::string& p_path);
	virtual std::string getApplicationDir();
	virtual std::string getTemporaryDir();
	virtual std::string getSaveRootDir();
	
	// Path functions
	virtual std::string             getAbsolutePath  (const std::string& p_path);
	virtual std::string             getRelativePath  (const std::string& p_path);
	virtual std::string             getRelativePathTo(const std::string& p_path, const std::string& p_dir);
	virtual std::string::value_type getDirSeparator() const;
	virtual std::string             getApplicationPath();
	
	/*! \brief Ensures the specified name is a valid Windows file or directory name
	           (not a path, just the file/directory name).
	           Replaces all invalid characters with underscores. */
	static std::string sanitizeFilename(const std::string& p_name);
	
	/*! \brief Checks the casing of the specified path to ensure it exactly matches the on-disk casing.
	    \note This function is only available in non-final builds (as it is a debug feature).
	    \param p_path The path to check.
	    \return True if path is correct (including casing), false if it is not. */
#if !defined(TT_BUILD_FINAL)
	static bool validatePathCasing(const std::string& p_path, identifier p_fsID = 0);
#else
	inline static bool validatePathCasing(const std::string&, identifier = 0) { return true; }
#endif
	
private:
	WindowsFileSystem(identifier p_id, const std::string& p_appName);
	
	// No copying
	WindowsFileSystem(const WindowsFileSystem& p_rhs);
	WindowsFileSystem& operator=(const WindowsFileSystem& p_rhs);
	
	
	// Name of the application for which the file system was created.
	// This is stored for situations where the name is required in file system operations
	// (such as retrieving the root save data directory)
	const std::string m_appName;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_FS_WINDOWSFILESYSTEM_H)
