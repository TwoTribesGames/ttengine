#if !defined(INC_TT_FS_POSIXFILESYSTEM_H)
#define INC_TT_FS_POSIXFILESYSTEM_H


#include <tt/fs/FileSystem.h>
#include <tt/fs/types.h>


namespace tt {
namespace fs {

class PosixFileSystem : public FileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier, const std::string& p_appName);
	
	virtual ~PosixFileSystem();
	
	
	// Feature support check functions
	
	/*! \brief Returns whether saving is supported.
	    \return Whether saving is supported.*/
	virtual bool supportsSaving();
	
	/*! \brief Returns whether directories are supported.
	    \return Whether directories are supported.*/
	virtual bool supportsDirectories();
	
	
	// Basic file functions
	
	/*! \brief Opens a file.
	    \param p_file The File object to use.
	    \param p_path Path of the file to open.
	    \param p_mode mode in which to open the file.
	    \return false on fail, true on success.*/
	virtual bool open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	
	/*! \brief Closes a file.
	    \param p_file The file to close.
	    \return false on fail, true on success.*/
	virtual bool close(File* p_file);
	
	/*! \brief Reads from a file.
	    \param p_file The file to read from.
	    \param p_buffer The buffer to read to.
	    \param p_length The amount of data to read.
	    \return The amount of data read.*/
	virtual size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length);
	
	/*! \brief Reads from a file asynchronously.
	    \param p_file The file to read from.
	    \param p_buffer The buffer to read to.
	    \param p_length The amount of data to read.
	    \param p_callback Function to call when read completed.
	    \param p_arg Argument to pass to callback function.
	    \return The amount of data read, 0 when async read could not be started*/
	virtual size_type readAsync(const FilePtr& p_file,
	                            void*          p_buffer,
	                            size_type      p_length,
	                            callback       p_callback,
	                            void*          p_arg);
	
	/*! \brief Writes to a file.
	    \param p_file The file to write to.
	    \param p_buffer The buffer to write from.
	    \param p_length The amount of data to write.
	    \return The amount of data written.*/
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);
	
	/*! \brief Writes to a file.
	    \param p_file The file to write to.
	    \param p_buffer The buffer to write from.
	    \param p_length The amount of data to write.
	    \param p_callback Function to call when write completed.
	    \param p_arg Argument to pass to callback function.
	    \return The amount of data written.*/
	virtual size_type writeAsync(const FilePtr& p_file,
	                             const void*    p_buffer,
	                             size_type      p_length,
	                             callback       p_callback,
	                             void*          p_arg);
	
	/*! \brief Flushes a file's buffers.
	    \param p_file The file to flush.
	    \return True on success, false on failure.*/
	virtual bool flush(const FilePtr& p_file);
	
	
	// Asynchronous status functions
	
	/*! \brief Checks whether a file is busy.
	    \param p_file The file to check.
	    \return Whether the file is busy.*/
	virtual bool isBusy(const FilePtr& p_file);
	
	/*! \brief Checks whether an asynchronous function has completed successfully.
	    \param p_file The file to check.
	    \return Whether the asynchronous function has completed successfully.*/
	virtual bool isSucceeded(const FilePtr& p_file);
	
	/*! \brief Waits for a file's asynchronous process to complete.
	    \param p_file The file to wait for.
	    \return Whether the wait succeeded.*/
	virtual bool wait(const FilePtr& p_file);
	
	/*! \brief Cancels a file's asynchronous process.
	    \param p_file The file whose asynchronous process must be canceled.
	    \return Whether the asynchronous process could be canceled.*/
	virtual bool cancel(const FilePtr& p_file);
	
	
	// Position / size functions
	
	/*! \brief Moves the file pointer within a file.
	    \param p_file The file to seek in.
	    \param p_offset The number of bytes to move the filepointer.
	    \param p_position The position from which to move the filepointer.
	    \return Whether the seek completed successfully.*/
	virtual bool seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	
	/*! \brief Gets the position of the filepointer.
	    \param p_file The file to retrieve the position from.
	    \return The position of the file pointer.*/
	virtual pos_type getPosition(const FilePtr& p_file);
	
	/*! \brief Gets the length of the file.
	    \param p_file The file to retrieve the length from.
	    \return The length of the file.*/
	virtual size_type getLength(const FilePtr& p_file);
	
	
	// Time functions
	
	/*! \brief Gets the time of creation of the file.
	    \param p_file The file to retrieve the time of creation from.
	    \return The time of creation of the file.*/
	virtual time_type getCreationTime(const FilePtr& p_file);
	
	/*! \brief Gets the time of last access of the file.
	    \param p_file The file to retrieve the time of access from.
	    \return The time of access of the file.*/
	virtual time_type getAccessTime(const FilePtr& p_file);
	
	/*! \brief Gets the time of last write of the file.
	    \param p_file The file to retrieve the time of write from.
	    \return The time of write of the file.*/
	virtual time_type getWriteTime(const FilePtr& p_file);
	
		/*! \brief Sets the time of last write of the file.
	    \param p_file The file to set the time of write to.
	    \param p_time The time of write.
	    \return false on fail, true on success.*/
	virtual bool setWriteTime(const FilePtr& p_file, time_type p_time);
	virtual s64       convertToUnixTime  (time_type p_nativeTime);
	virtual time_type convertToNativeTime(s64       p_unixTime);

	// Directory functions
	
	/*! \brief Opens a directory for reading.
	    \param p_dir The Dir object to use.
	    \param p_path The path of the directory to open.
	    \param p_filter The filter to use.
	    \return false on fail, true on success.*/
	virtual bool openDir(DirPtr& p_dir, const std::string& p_path, const std::string& p_filter);
	
	/*! \brief Closes a directory.
	    \param p_dir The directory to close.
	    \return false on fail, true on success.*/
	virtual bool closeDir(Dir* p_dir);
	
	/*! \brief Reads from a directory.
	    \param p_dir The directory to read from.
	    \param p_entry Pointer to a DirEntry object in which the result may be written.
	    \return false when no files remain, true on success.*/
	virtual bool readDir(const DirPtr& p_dir, DirEntry& p_entry);
	
	
	// Create / Destroy functions
	
	/*! \brief Checks whether a file exists.
	    \param p_path The file to check.
	    \return false when the file does not exits, true when it does.*/
	virtual bool fileExists(const std::string& p_path);
	
	/*! \brief Destroys a file.
	    \param p_path The file to destroy.
	    \return false on fail, true on success.*/
	virtual bool destroyFile(const std::string& p_path);
	
	/*! \brief Copies a file.
	    \param p_existingFile The name of an existing file.
	    \param p_newFile The name of the new file.
	    \param p_failIfExists Whether the function should fail when the new file already exists.
	    \return false on fail, true on success.*/
	virtual bool copyFile(const std::string& p_existingFile,
	                      const std::string& p_newFile,
	                      bool               p_failIfExists);
	
	/*! \brief Moves a file.
	    \param p_existingFile The name of an existing file.
	    \param p_newFile The name of the new file.
	    \param p_failIfExists Whether the function should fail when the new file already exists.
	    \return false on fail, true on success.*/
	virtual bool moveFile(const std::string& p_existingFile,
	                      const std::string& p_newFile,
	                      bool               p_failIfExists);
	
	/*! \brief Checks whether a directory exists.
	    \param p_path The directory to check.
	    \return false when the directory does not exits, true when it does.*/
	virtual bool dirExists(const std::string& p_path);
	
	/*! \brief Creates a directory.
	    \param p_path The directory to create.
	    \return false when the directory could not be created, true on success.*/
	virtual bool createDir(const std::string& p_path);
	
	/*! \brief Destroys a directory.
	    \param p_path The directory to destroy.
	    \return false when the directory could not be destroyed, true on success.*/
	virtual bool destroyDir(const std::string& p_path);
	
	
	// Working Directory functions
	
	/*! \brief Retrieves current working directory (with trailing directory separator).
	    \return The current working directory on success, empty string on fail.*/
	virtual std::string getWorkingDir();
	
	/*! \brief Sets current working directory.
	    \param p_path New working directory.
	    \return True on success, false on fail.*/
	virtual bool setWorkingDir(const std::string& p_path);
	
	/*! \brief Retrieves the application directory (with trailing directory separator).
	    \return The directory in which the application is located, empty string on fail.*/
	virtual std::string getApplicationDir();
	
	/*! \brief Retrieves the temporary directory (with trailing directory separator).
	    \return The directory in which temporary files are stored, empty string on fail.*/
	virtual std::string getTemporaryDir();
	
	/*! \brief Retrieves the root directory for save data (including dir separator, if required).
	    \return The root directory for save data. */
	virtual std::string getSaveRootDir();
	
	
	// Path functions
	
	/*! \brief Retrieves absolute path of a file.
	    \param p_path The path to convert.
	    \return The absolute path on success, empty string on fail.*/
	virtual std::string getAbsolutePath(const std::string& p_path);
	
	/*! \brief Retrieves relative path of a file.
	    \param p_path The path to convert.
	    \return The relative path on success, empty string on fail.*/
	virtual std::string getRelativePath(const std::string& p_path);
	
	/*! \brief Retrieves relative path of a file compared to a directory.
	    \param p_path The path to convert.
	    \param p_dir The directory to compare the path to.
	    \return The relative path on success, empty string on fail.*/
	virtual std::string getRelativePathTo(const std::string& p_path, const std::string& p_dir);
	
	/*! \brief Retrieves the directory separator for this filesystem.
	    \return The directory separator for this filesystem.*/
	virtual std::string::value_type getDirSeparator() const;
	
	virtual std::string getApplicationPath();
private:
	PosixFileSystem(identifier p_id, const std::string& p_appName);
	void addTrailingDirSeparator(std::string& p_path);
	
	// No copying
	PosixFileSystem(const PosixFileSystem& p_rhs);
	PosixFileSystem& operator=(const PosixFileSystem& p_rhs);
	
	
	const std::string m_appName;

	std::string m_saveDir;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_FS_POSIXFILESYSTEM_H)
