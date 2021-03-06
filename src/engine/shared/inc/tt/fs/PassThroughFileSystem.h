#if !defined(INC_TT_FS_PASSTHROUGHFILESYSTEM_H)
#define INC_TT_FS_PASSTHROUGHFILESYSTEM_H


#include <map>
#include <vector>

#include <tt/fs/types.h>
#include <tt/fs/FileSystem.h>


namespace tt {
namespace fs {

/*! \brief PassThroughFileSystem is an intermediate filesystem where all
           operations are passed through a source filesystem.
           To use this FileSystem it should be derived and the needed
           methods (those that differ from the source implementation)
           should be overriden.
*/
class PassThroughFileSystem : public FileSystem
{
public:
	virtual ~PassThroughFileSystem() { }
	
	// Feature support check functions
	virtual bool supportsSaving();
	virtual bool supportsAsync();
	virtual bool supportsDirectories();
	virtual bool supportsCommitting();
	
	// Commit functions
	virtual bool mountSaveVolume(OpenMode p_mode);
	virtual bool unmountSaveVolume();
	virtual bool isSaveVolumeMounted();
	virtual bool hasChanges();
	virtual bool canCommit();
	virtual bool commit();
	
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
	virtual s64       convertToUnixTime  (time_type p_nativeTime);
	virtual time_type convertToNativeTime(s64 p_unixTime);
	
	// Directory functions
	virtual bool openDir(DirPtr&            p_dir,
	                     const std::string& p_path,
	                     const std::string& p_filter = "*");
	virtual bool closeDir(Dir* p_dir);
	virtual bool readDir(const DirPtr& p_dir, DirEntry& p_entry);
	
	// Create / Destroy functions
	virtual bool fileExists (const std::string& p_path);
	virtual bool destroyFile(const std::string& p_path);
	virtual bool copyFile(const std::string& p_existingFile,
	                      const std::string& p_newFile,
	                      bool               p_failIfExists);
	virtual bool moveFile(const std::string& p_existingFile,
	                      const std::string& p_newFile,
	                      bool               p_failIfExists);
	virtual bool dirExists(const std::string& p_path);
	virtual bool createDir(const std::string& p_path);
	virtual bool createSaveRootDir();
	virtual bool destroyDir(const std::string& p_path);
	
	// Working Directory functions
	virtual std::string getWorkingDir();
	virtual bool setWorkingDir(const std::string& p_path);
	virtual std::string getApplicationDir();
	virtual std::string getTemporaryDir();
	virtual std::string getSaveRootDir();
	
	// Path functions
	virtual std::string getAbsolutePath(const std::string& p_path);
	virtual std::string getRelativePath(const std::string& p_path);
	virtual std::string getRelativePathTo(const std::string& p_path, const std::string& p_dir);
	virtual std::string::value_type getDirSeparator() const;
	virtual std::string getApplicationPath();
	
	// Misc functions
	virtual s32 getAllocationType() const;
	
protected:
	PassThroughFileSystem(identifier p_id, identifier p_source);
	
	FileSystem* getSource() const;
	
	inline identifier getSourceID() const { return m_source; }
	
private:
	const identifier m_source;
	
	
	// Disable assignment/copy
	PassThroughFileSystem(const PassThroughFileSystem& p_rhs);
	PassThroughFileSystem& operator=(const PassThroughFileSystem& p_rhs);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_FS_PASSTHROUGHFILESYSTEM_H)
