#ifndef INC_TT_FS_FS_H
#define INC_TT_FS_FS_H


#include <limits>
#include <string>

#include <tt/code/Buffer.h>
#include <tt/fs/types.h>


namespace tt {
namespace fs {
	
// Filesystem functions

/*! \brief Registers a filesystem.
    \param p_fileSystem The FileSystem to register.
    \param p_identifier The identifier to assign to the filesystem.
    \return false on fail, true on success.*/
bool registerFileSystem(FileSystem* p_fileSystem, identifier p_identifier);

/*! \brief Unregisters a filesystem.
    \param p_identifier The identifier of the filesystem to unregister.
    \return false on fail, true on success.*/
bool unregisterFileSystem(identifier p_identifier);

/*! \param p_identifier The identifier for which to get the file system implementation.
    \return Pointer to a file system that was registered for ID p_identifier. Returns null if no file system was registered with this ID.
    \note This function is for tt::fs internal usage only! */
FileSystem* getFileSystem(identifier p_identifier);


// Feature support check functions

/*! \brief Returns whether saving is supported.
    \param p_identifier The identifier of the filesystem to check.
    \return Whether saving is supported.*/
bool supportsSaving(identifier p_identifier);

/*! \brief Returns whether asynchronous reading/writing is supported.
    \param p_identifier The identifier of the filesystem to check.
    \return Whether asynchronous reading/writing is supported.*/
bool supportsAsync(identifier p_identifier);

/*! \brief Returns whether directories are supported.
    \param p_identifier The identifier of the filesystem to check.
    \return Whether directories are supported.*/
bool supportsDirectories(identifier p_identifier);

/*! \brief Returns whether committing is supported.
    \param p_identifier The identifier of the filesystem to check.
    \return Whether committing is supported.*/
bool supportsCommitting(identifier p_identifier);


// Commit functions

/*! \brief Prepares the save volume for reading and/or writing save data (if the file system has such a requirement).
    \param p_mode How to mount the save volume: for reading and/or writing.
    \param p_identifier The identifier of the filesystem to affect.
    \return true if mounting was successful, false in case of error. */
bool mountSaveVolume(OpenMode p_mode, identifier p_identifier);

/*! \brief Releases the save volume (if the file system has such a requirement).
    \param p_identifier The identifier of the filesystem to affect.
    \return true if unmounting was successful, false in case of error. */
bool unmountSaveVolume(identifier p_identifier);

/*! \brief Indicates whether the save volume has been mounted.
    \param p_identifier The identifier of the filesystem to affect. */
bool isSaveVolumeMounted(identifier p_identifier);

/*! \brief Checks whether there are unsaved changes.
    \param p_identifier The identifier of the filesystem to check for changes.
    \return true when there are unsaved changes, false when there aren't.*/
bool hasChanges(identifier p_identifier);

/*! \brief Checks whether there are enough resources to save the changes.
    \param p_identifier The identifier of the filesystem to use.
    \return true when there are enough resources, false when there aren't.*/
bool canCommit(identifier p_identifier);

/*! \brief Saves all unsaved changes.
    \param p_identifier The identifier of the filesystem to use.
    \return true on success, false on fail.*/
bool commit(identifier p_identifier);


// File functions

/*! \brief Opens a file.
    \param p_path Path of the file to open.
    \param p_mode mode in which to open the file.
    \param p_identifier Filesystem to use.
    \return null on fail, pointer to file on success.*/
FilePtr open(const std::string& p_path, OpenMode p_mode, identifier p_identifier = 0);

/*! \brief Closes a file.
    \param p_file The file to close.
    \return false on fail, true on success.*/
bool close(File* p_file);

/*! \brief Reads from a file.
    \param p_file The file to read from.
    \param p_buffer The buffer to read to.
    \param p_length The amount of data to read.
    \return The amount of data read.*/
size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length);

/*! \brief Reads from a file asynchronously.
    \param p_file The file to read from.
    \param p_buffer The buffer to read to.
    \param p_length The amount of data to read.
    \param p_callback Function to call when read completed.
    \param p_arg Argument to pass to callback function.
    \return The amount of data read, 0 when async read could not be started*/
size_type readAsync(const FilePtr& p_file,
                    void*          p_buffer,
                    size_type      p_length,
                    callback       p_callback,
                    void*          p_arg);

/*! \brief Writes to a file.
    \param p_file The file to write to.
    \param p_buffer The buffer to write from.
    \param p_length The amount of data to write.
    \return The amount of data written.*/
size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);

/*! \brief Writes to a file.
    \param p_file The file to write to.
    \param p_buffer The buffer to write from.
    \param p_length The amount of data to write.
    \param p_callback Function to call when write completed.
    \param p_arg Argument to pass to callback function.
    \return The amount of data written.*/
size_type writeAsync(const FilePtr& p_file,
                     const void*    p_buffer,
                     size_type      p_length,
                     callback       p_callback,
                     void*          p_arg);

/*! \brief Flushes a file's buffers.
    \param p_file The file to flush.
    \return True on success, false on failure.*/
bool flush(const FilePtr& p_file);


// Content functions

/*! \brief Gets the content of a file.
    \param p_path Path of the file to open.
    \param p_identifier Filesystem to use.
    \return Pointer to a buffer object containing the content of the file, null pointer on fail.*/
code::BufferPtr getFileContent(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Gets the content of a file.
    \param p_file The file to get the content of.
    \return Pointer to a buffer object containing the content of the file, null pointer on fail.*/
code::BufferPtr getFileContent(const FilePtr& p_file);


// Asynchronous status functions

/*! \brief Checks whether a file is busy.
    \param p_file The file to check.
    \return Whether the file is busy.*/
bool isBusy(const FilePtr& p_file);

/*! \brief Checks whether an asynchronous function has completed successfully.
    \param p_file The file to check.
    \return Whether the asynchronous function has completed successfully.*/
bool isSucceeded(const FilePtr& p_file);

/*! \brief Waits for a file's asynchronous process to complete.
    \param p_file The file to wait for.
    \return Whether the wait succeeded.*/
bool wait(const FilePtr& p_file);

/*! \brief Cancels a file's asynchronous process.
    \param p_file The file whose asynchronous process must be canceled.
    \return Whether the asynchronous process could be canceled.*/
bool cancel(const FilePtr& p_file);


// Position / size functions

/*! \brief Moves the file pointer within a file.
    \param p_file The file to seek in.
    \param p_offset The number of bytes to move the filepointer.
    \param p_position The position from which to move the filepointer.
    \return Whether the seek completed successfully.*/
bool seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position);

/*! \brief Moves the file pointer to the beginning of a file.
    \param p_file The file to seek in.
    \return Whether the seek completed successfully.*/
bool seekToBegin(const FilePtr& p_file);

/*! \brief Moves the file pointer to the end of a file.
    \param p_file The file to seek in.
    \return Whether the seek completed successfully.*/
bool seekToEnd(const FilePtr& p_file);

/*! \brief Gets the position of the filepointer.
    \param p_file The file to retrieve the position from.
    \return The position of the file pointer.*/
pos_type getPosition(const FilePtr& p_file);

/*! \brief Gets the length of the file.
    \param p_file The file to retrieve the length from.
    \return The length of the file.*/
size_type getLength(const FilePtr& p_file);


// Time functions

/*! \brief Gets the time of creation of the file.
    \param p_file The file to retrieve the time of creation from.
    \return The time of creation of the file.*/
time_type getCreationTime(const FilePtr& p_file);

/*! \brief Gets the time of last access of the file.
    \param p_file The file to retrieve the time of access from.
    \return The time of access of the file.*/
time_type getAccessTime(const FilePtr& p_file);

/*! \brief Gets the time of last write of the file.
    \param p_file The file to retrieve the time of write from.
    \return The time of write of the file.*/
time_type getWriteTime(const FilePtr& p_file);

/*! \brief Sets the time of last write of the file.
    \param p_file The file to set the time of write to.
    \param p_time The time of write.
    \return false on fail, true on success.*/
bool setWriteTime(const FilePtr& p_file, time_type p_time);

/*! \brief Converts the platform-specific time to UNIX time (number of seconds since 1970-01-01).
    \param p_nativeTime The platform-specific time to convert.
    \return p_nativeTime converted to UNIX time. */
s64 convertToUnixTime(time_type p_nativeTime, identifier p_identifier = 0);

/*! \brief Converts the UNIX time (number of seconds since 1970-01-01) to platform-specific time.
    \param p_unixTime The UNIX time to convert.
    \return p_unixTime converted to platform-specific time. */
time_type convertToNativeTime(s64 p_unixTime, identifier p_identifier = 0);


// Directory functions

/*! \brief Opens a directory for reading.
    \param p_path The path of the directory to open.
    \param p_filter file filter to use.
    \param p_identifier Filesystem to use.
    \return null on fail, dir object on success.*/
DirPtr openDir(const std::string& p_path,
               const std::string& p_filter = "*",
               identifier         p_identifier = 0);

/*! \brief Closes a directory.
    \param p_dir The directory to close.
    \return false on fail, true on success.*/
bool closeDir(Dir* p_dir);

/*! \brief Reads from a directory.
    \param p_dir The directory to read from.
    \param p_entry Pointer to a DirEntry object in which the result may be written.
    \return false when no files remain, true on success.*/
bool readDir(const DirPtr& p_dir, DirEntry& p_entry);


// Create / Destroy functions

/*! \brief Checks whether a file exists.
    \param p_path The file to check.
    \param p_identifier Filesystem to use.
    \return false when the file does not exits, true when it does.*/
bool fileExists(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Destroys a file.
    \param p_path The file to destroy.
    \param p_identifier Filesystem to use.
    \return false on fail, true on success.*/
bool destroyFile(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Copies a file.
    \param p_existingFile The name of an existing file.
    \param p_newFile The name of the new file.
    \param p_failIfExists Whether the function should fail when the new file already exists.
    \param p_identifier Filesystem to use.
    \return false on fail, true on success.*/
bool copyFile(const std::string& p_existingFile,
              const std::string& p_newFile,
              bool               p_failIfExists,
              identifier         p_identifier = 0);

/*! \brief Moves a file.
    \param p_existingFile The name of an existing file.
    \param p_newFile The name of the new file.
    \param p_failIfExists Whether the function should fail when the new file already exists.
    \param p_identifier Filesystem to use.
    \return false on fail, true on success.*/
bool moveFile(const std::string& p_existingFile,
              const std::string& p_newFile,
              bool               p_failIfExists,
              identifier         p_identifier = 0);

/*! \brief Checks whether a directory exists.
    \param p_path The directory to check.
    \param p_identifier Filesystem to use.
    \return false when the directory does not exits, true when it does.*/
bool dirExists(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Creates a directory.
    \param p_path The directory to create.
    \param p_identifier Filesystem to use.
    \return false when the directory could not be created, true on success.*/
bool createDir(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Creates the root directory for save files, if necessary.
    \param p_identifier Filesystem to use.
    \return false when the directory could not be created, true on success.*/
bool createSaveRootDir(identifier p_identifier = 0);

/*! \brief Destroys a directory.
    \param p_path The directory to destroy.
    \param p_identifier Filesystem to use.
    \return false when the directory could not be destroyed, true on success.*/
bool destroyDir(const std::string& p_path, identifier p_identifier = 0);


// Working Directory functions

/*! \brief Retrieves current working directory
    \param p_identifier Filesystem to use.
    \return The current working directory on success, empty string on fail.*/
std::string getWorkingDir(identifier p_identifier = 0);

/*! \brief Sets current working directory
    \param p_path New working directory.
    \param p_identifier Filesystem to use.
    \return True on success, false on fail.*/
bool setWorkingDir(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves the application directory (path to the directory where the executable resides).
    \param p_identifier Filesystem to use.
    \return The directory in which the application is located, empty string on fail.*/
std::string getApplicationDir(identifier p_identifier = 0);

/*! \brief Retrieves the temporary directory
    \param p_identifier Filesystem to use.
    \return The directory in which temporary files are stored, empty string on fail.*/
std::string getTemporaryDir(identifier p_identifier = 0);

/*! \brief Retrieves the root directory for save data.
    \param p_identifier Filesystem to use.
    \return The root directory for save data. */
std::string getSaveRootDir(identifier p_identifier = 0);


// Path functions

/*! \brief Retrieves absolute path of a file.
    \param p_path The path to convert.
    \param p_identifier Filesystem to use.
    \return The absolute path on success, empty string on fail.*/
std::string getAbsolutePath(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves relative path of a file.
    \param p_path The path to convert.
    \param p_identifier Filesystem to use.
    \return The relative path on success, empty string on fail.*/
std::string getRelativePath(const std::string& p_path, identifier p_identifier = 0);

/*! \brief Retrieves relative path of a file compared to a directory.
    \param p_path The path to convert.
    \param p_dir The directory to compare the path to.
    \param p_identifier Filesystem to use.
    \return The relative path on success, empty string on fail.*/
std::string getRelativePathTo(const std::string& p_path,
                              const std::string& p_dir,
                              identifier         p_identifier = 0);

/*! \brief Retrieves the directory separator for a filesystem.
    \param p_identifier Filesystem to use.
    \return The directory separator for the specified filesystem.*/
std::string::value_type getDirSeparator(identifier p_identifier = 0);

/*! \brief Retrieves the application path (full executable path, including filename).
    \param p_identifier Filesystem to use.
    \return The path of the application binary, empty string on fail.*/
std::string getApplicationPath(identifier p_identifier = 0);


// Misc functions

/*! \brief Gets the memory allocation type for a given File System.
    \param p_identifier Filesystem to get allocation type of.
    \return The memory allocation type of the specified filesystem.*/
s32 getAllocationType(identifier p_identifier = 0);


// Helper functions

/*! \brief Checks whether a file is up to date.
    \param p_target The target file.
    \param p_prerequisite The file on which the target depends.
    \param p_identifier Filesystem to use.
    \return true when target is newer than prerequisite
            false when prerequisite is newer than target
            false when prerequisite or target do not exist.*/
bool isUpToDate(const std::string& p_target,
                const std::string& p_prerequisite,
                identifier         p_identifier = 0);

template<typename T>
inline bool readInteger(const FilePtr& p_file, T* p_t)
{
	if (p_t == 0 || p_file == 0)
	{
		return false;
	}
	
	u8 buf[sizeof(T)];
	*p_t = 0;
	if (read(p_file, buf, static_cast<size_type>(sizeof(T))) != static_cast<size_type>(sizeof(T)))
	{
		return false;
	}
	
	for (std::size_t i = 0; i < sizeof(T); ++i)
	{
		*p_t += static_cast<T>(buf[i]) << (8 * i);
	}
	return true;
}

template<typename T>
inline bool writeInteger(const FilePtr& p_file, T p_t)
{
	if (p_file == 0)
	{
		return false;
	}
	
	u8 buf[sizeof(T)];
	for (std::size_t i = 0; i < sizeof(T); ++i)
	{
		buf[i] = static_cast<u8>((p_t >> (8 * i)) & 0xFF);
	}
	
	if (write(p_file, buf, static_cast<size_type>(sizeof(T))) != static_cast<size_type>(sizeof(T)))
	{
		return false;
	}
	
	return true;
}

template<typename SizeType, typename EnumType>
inline bool readEnum(const FilePtr& p_file, EnumType* p_t)
{
	SizeType tmpBuf = 0;
	if (readInteger(p_file, &tmpBuf))
	{
		(*p_t) = static_cast<EnumType>(tmpBuf);
		return true;
	}
	return false;
}

template<typename SizeType, typename EnumType>
inline bool writeEnum(const FilePtr& p_file, EnumType p_t)
{
	// Make sure the value will fit in SizeType. (Bias to unsigned SizeTypes.)
	if (static_cast<u32>(p_t) >= static_cast<u32>(std::numeric_limits<SizeType>::max()))
	{
		TT_PANIC("Enum value (%u) too large for SizeType (%u).",
		         static_cast<u32>(p_t), static_cast<u32>(std::numeric_limits<SizeType>::max()));
		return false;
	}
	
	SizeType tmpBuf = static_cast<SizeType>(p_t);
	return writeInteger(p_file, tmpBuf);
}


template<typename SizeType>
inline bool readBool(const FilePtr& p_file, bool* p_value)
{
	SizeType tmpBuf = 0;
	if (readInteger(p_file, &tmpBuf))
	{
		(*p_value) = (tmpBuf != 0);
		return true;
	}
	return false;
}
// Non-template version which defaults to u8 as SizeType.
inline bool readBool(const FilePtr& p_file, bool* p_value) { return readBool<u8>(p_file, p_value); }

template<typename SizeType>
inline bool writeBool(const FilePtr& p_file, bool p_value)
{
	SizeType tmpBuf = (p_value) ? SizeType(1) : SizeType(0);
	return writeInteger(p_file, tmpBuf);
}
// Non-template version which defaults to u8 as SizeType.
inline bool writeBool(const FilePtr& p_file, bool p_value) { return writeBool<u8>(p_file, p_value); }

bool readNarrowString(const FilePtr& p_file, std::string* p_value);
bool writeNarrowString(const FilePtr& p_file, const std::string& p_string);

bool readWideString(const FilePtr& p_file, std::wstring* p_value);
bool writeWideString(const FilePtr& p_file, const std::wstring& p_string);

bool readReal(const FilePtr& p_file, real* p_value);
bool writeReal(const FilePtr& p_file, real p_value);
bool readReal(const FilePtr& p_file, real64* p_value);
bool writeReal(const FilePtr& p_file, real64 p_value);

// namespace end
}
}


#endif // INC_TT_FS_FS_H
