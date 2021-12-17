#ifndef INC_TT_FS_FILESYSTEM_H
#define INC_TT_FS_FILESYSTEM_H


#include <string>

#include <tt/code/Buffer.h>
#include <tt/fs/types.h>


namespace tt {
namespace fs {

class FileSystem
{
public:
	FileSystem(identifier p_id);
	virtual ~FileSystem();
	
	
	// Feature support check functions
	
	/*! \brief Returns whether saving is supported.
	    \return Whether saving is supported.*/
	virtual bool supportsSaving();
	
	/*! \brief Returns whether asynchronous reading/writing is supported.
	    \return Whether asynchronous reading/writing is supported.*/
	virtual bool supportsAsync();
	
	/*! \brief Returns whether directories are supported.
	    \return Whether directories are supported.*/
	virtual bool supportsDirectories();
	
	/*! \brief Returns whether committing is supported.
	    \return Whether committing is supported.*/
	virtual bool supportsCommitting();
	
	
	// Commit functions
	
	/*! \brief Prepares the save volume for reading and/or writing save data (if the file system has such a requirement).
	    \param p_mode How to mount the save volume: for reading and/or writing.
	    \return true if mounting was successful, false in case of error. */
	virtual bool mountSaveVolume(OpenMode p_mode);
	
	/*! \brief Releases the save volume (if the file system has such a requirement).
	    \return true if unmounting was successful, false in case of error. */
	virtual bool unmountSaveVolume();
	
	/*! \brief Indicates whether the save volume has been mounted. */
	virtual bool isSaveVolumeMounted();
	
	/*! \brief Checks whether there are unsaved changes.
	    \return true when there are unsaved changes, false when there aren't.*/
	virtual bool hasChanges();
	
	/*! \brief Checks whether there are enough resources to save the changes.
	    \return true when there are enough resources, false when there aren't.*/
	virtual bool canCommit();
	
	/*! \brief Saves all unsaved changes.
	    \return true on success, false on fail.*/
	virtual bool commit();
	
	
	// Basic file functions
	
	/*! \brief Opens a file.
	    \param p_file The File object to use.
	    \param p_path Path of the file to open.
	    \param p_mode mode in which to open the file.
	    \return false on fail, true on success.*/
	virtual bool open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode) = 0;
	
	/*! \brief Closes a file.
	    \param p_file The file to close.
	    \return false on fail, true on success.*/
	virtual bool close(File* p_file) = 0;
	
	/*! \brief Reads from a file.
	    \param p_file The file to read from.
	    \param p_buffer The buffer to read to.
	    \param p_length The amount of data to read.
	    \return The amount of data read.*/
	virtual size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length) = 0;
	
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
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length) = 0;
	
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
	
	
	// Content functions
	
	/*! \brief Gets the content of a file.
	    \param p_path Path of the file to open.
	    \return Pointer to a buffer object containing the content of the file, null pointer on fail.*/
	virtual code::BufferPtr getFileContent(const std::string& p_path);
	
	/*! \brief Gets the content of a file.
	    \param p_file The file to get the content of.
	    \return Pointer to a buffer object containing the content of the file, null pointer on fail.*/
	virtual code::BufferPtr getFileContent(const FilePtr& p_file);
	
	
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
	virtual bool seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position) = 0;
	
	/*! \brief Moves the file pointer to the beginning of a file.
	    \param p_file The file to seek in.
	    \return Whether the seek completed successfully.*/
	virtual bool seekToBegin(const FilePtr& p_file);
	
	/*! \brief Moves the file pointer to the end of a file.
	    \param p_file The file to seek in.
	    \return Whether the seek completed successfully.*/
	virtual bool seekToEnd(const FilePtr& p_file);
	
	/*! \brief Gets the position of the filepointer.
	    \param p_file The file to retrieve the position from.
	    \return The position of the file pointer.*/
	virtual pos_type getPosition(const FilePtr& p_file) = 0;
	
	/*! \brief Gets the length of the file.
	    \param p_file The file to retrieve the length from.
	    \return The length of the file.*/
	virtual size_type getLength(const FilePtr& p_file) = 0;
	
	
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
	
	/*! \brief Converts the platform-specific time to UNIX time (number of seconds since 1970-01-01).
	    \param p_nativeTime The platform-specific time to convert.
	    \return p_nativeTime converted to UNIX time. */
	virtual s64 convertToUnixTime(time_type p_nativeTime);
	
	/*! \brief Converts the UNIX time (number of seconds since 1970-01-01) to platform-specific time.
	    \param p_unixTime The UNIX time to convert.
	    \return p_unixTime converted to platform-specific time. */
	virtual time_type convertToNativeTime(s64 p_unixTime);
	
	
	// Directory functions
	
	/*! \brief Opens a directory for reading.
	    \param p_dir The Dir object to use.
	    \param p_path The path of the directory to open.
	    \param p_filter file filter to use.
	    \return false on fail, true on success.*/
	virtual bool openDir(DirPtr&            p_dir,
	                     const std::string& p_path,
	                     const std::string& p_filter = "*");
	
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
	virtual bool fileExists(const std::string& p_path) = 0;
	
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
	
	/*! \brief Creates the root directory for save files, if necessary.
	    \return false when the directory could not be created, true on success.*/
	virtual bool createSaveRootDir();
	
	/*! \brief Destroys a directory.
	    \param p_path The directory to destroy.
	    \return false when the directory could not be destroyed, true on success.*/
	virtual bool destroyDir(const std::string& p_path);
	
	
	// Working Directory functions
	
	/*! \brief Retrieves current working directory
	    \return The current working directory on success, empty string on fail.*/
	virtual std::string getWorkingDir();
	
	/*! \brief Sets current working directory
	    \param p_path New working directory.
	    \return True on success, false on fail.*/
	virtual bool setWorkingDir(const std::string& p_path);
	
	/*! \brief Retrieves the application directory
	    \return The directory in which the application is located, empty string on fail.*/
	virtual std::string getApplicationDir();
	
	/*! \brief Retrieves the temporary directory
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
	
	/*! \brief Retrieves the application path
	    \return The path of the application binary, empty string on fail.*/
	virtual std::string getApplicationPath();
	
	
	// Misc functions
	
	/*! \brief Returns the memory allocation type of this filesystem
	    \return The memory allocation type.*/
	virtual s32 getAllocationType() const;
	
	inline identifier getIdentifier() const { return m_identifier; }
	
protected:
	/*! \brief Checks that the file pointer is not null, file has internal data set for it
	           and the file's file system ID matches this file system ID.
	           This function will produce panics if any of the criteria aren't valid.
	    \return true if file is valid for use in this file system, false if not. */
	bool validate(const FilePtr& p_file) const;
	
	/*! \brief Checks that the directory pointer is not null, directory has internal data set for it
	           and the directory's file system ID matches this file system ID (FIXME: this last check is disabled at the moment).
	           This function will produce panics if any of the criteria aren't valid.
	    \return true if dir is valid for use in this file system, false if not. */
	bool validate(const DirPtr& p_dir) const;
	
private:
	// No copying
	FileSystem(const FileSystem&);
	FileSystem& operator=(const FileSystem&);
	
	
	identifier m_identifier;
	bool       m_registered;
	
	friend bool registerFileSystem(FileSystem*, identifier);
};

// namespace end
}
}

#endif // INC_TT_FS_FILESYSTEM_H
