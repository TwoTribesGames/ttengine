#ifndef INC_TT_FS_FILE_H
#define INC_TT_FS_FILE_H

#include <tt/fs/fs.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace fs {

class File
{
public:
	// basic file functions
	
	/*! \brief Reads from a file.
	    \param p_buffer The buffer to read to.
	    \param p_length The amount of data to read.
	    \return The amount of data read.*/
	inline size_type read(void* p_buffer, size_type p_length)
	{
		return fs::read(m_this.lock(), p_buffer, p_length);
	}
	
	/*! \brief Reads from a file asynchronously.
	    \param p_buffer The buffer to read to.
	    \param p_length The amount of data to read.
	    \param p_callback Function to call when read completed.
	    \param p_arg Argument to pass to callback function.
	    \return The amount of data read, 0 when async read could not be started*/
	inline size_type readAsync(void*     p_buffer,
	                           size_type p_length,
	                           callback  p_callback,
	                           void*     p_arg)
	{
		return fs::readAsync(m_this.lock(), p_buffer, p_length, p_callback, p_arg);
	}
	
	/*! \brief Writes to a file.
	    \param p_buffer The buffer to write from.
	    \param p_length The amount of data to write.
	    \return The amount of data written.*/
	inline size_type write(const void* p_buffer, size_type p_length)
	{
		return fs::write(m_this.lock(), p_buffer, p_length);
	}
	
	/*! \brief Writes to a file.
	    \param p_buffer The buffer to write from.
	    \param p_length The amount of data to write.
	    \param p_callback Function to call when write completed.
	    \param p_arg Argument to pass to callback function.
	    \return The amount of data written.*/
	inline size_type writeAsync(const void* p_buffer,
	                            size_type   p_length,
	                            callback    p_callback,
	                            void*       p_arg)
	{
		return fs::writeAsync(m_this.lock(), p_buffer, p_length, p_callback, p_arg);
	}
	
	/*! \brief Flushes the file's buffer.
	    \return True on succes, false on failure.*/
	inline bool flush()
	{
		return fs::flush(m_this.lock());
	}
	
	
	// Content functions
	
	/*! \brief Returns the file's content.
	    \return Buffer containing the file's content on success, null on fail.*/
	inline code::BufferPtr getContent()
	{
		return fs::getFileContent(m_this.lock());
	}
	
	
	// Asynchronous status functions
	
	/*! \brief Checks whether a file is busy.
	    \return Whether the file is busy.*/
	inline bool isBusy() const
	{
		return fs::isBusy(m_this.lock());
	}
	
	/*! \brief Checks whether an asynchronous function has completed successfully.
	    \return Whether the asynchronous function has completed successfully.*/
	inline bool isSucceeded() const
	{
		return fs::isSucceeded(m_this.lock());
	}
	
	/*! \brief Waits for a file's asynchronous process to complete.
	    \return Whether the wait succeeded.*/
	inline bool wait()
	{
		return fs::wait(m_this.lock());
	}
	
	/*! \brief Cancels a file's asynchronous process.
	    \return Whether the asynchronous process could be canceled.*/
	inline bool cancel()
	{
		return fs::cancel(m_this.lock());
	}
	
	// Position / size functions
	
	/*! \brief Moves the file pointer within a file.
	    \param p_offset The number of bytes to move the filepointer.
	    \param p_position The position from which to move the filepointer.
	    \return Whether the seek completed successfully.*/
	inline bool seek(pos_type p_offset, SeekPos p_position)
	{
		return fs::seek(m_this.lock(), p_offset, p_position);
	}
	
	/*! \brief Moves the file pointer to the beginning of a file.
	    \return Whether the seek completed successfully.*/
	inline bool seekToBegin()
	{
		return fs::seekToBegin(m_this.lock());
	}
	
	/*! \brief Moves the file pointer to the end of a file.
	    \return Whether the seek completed successfully.*/
	inline bool seekToEnd()
	{
		return fs::seekToEnd(m_this.lock());
	}
	
	/*! \brief Gets the position of the filepointer.
	    \return The position of the file pointer.*/
	inline pos_type getPosition() const
	{
		return fs::getPosition(m_this.lock());
	}
	
	/*! \brief Gets the length of the file.
	    \return The length of the file.*/
	inline size_type getLength() const
	{
		return fs::getLength(m_this.lock());
	}
	
	// Time functions
	
	/*! \brief Gets the time of creation of the file.
	    \return The time of creation of the file.*/
	inline time_type getCreationTime() const
	{
		return fs::getCreationTime(m_this.lock());
	}
	
	/*! \brief Gets the time of last access of the file.
	    \return The time of access of the file.*/
	inline time_type getAccessTime() const
	{
		return fs::getAccessTime(m_this.lock());
	}
	
	/*! \brief Gets the time of last write of the file.
	    \return The time of write of the file.*/
	inline time_type getWriteTime() const
	{
		return fs::getWriteTime(m_this.lock());
	}
	
	/*! \brief Sets the time of last write of the file.
	    \param p_time The time of write.
	    \return false on fail, true on success.*/
	inline bool setWriteTime(time_type p_time)
	{
		return fs::setWriteTime(m_this.lock(), p_time);
	}
	
	/*! \brief Gets the file's internal data.
	    \return The file's internal data.*/
	inline void* getData() const { return m_data; }
	
	/*! \brief Sets the file's internal data.
	    \param p_data The data to set. */
	inline void setData(void* p_data) { m_data = p_data; }
	
	/*! \brief Gets the file's filesystem identifier.
	    \return The file's filesystem identifier.*/
	inline identifier getFileSystem() const { return m_identifier; }
	
	/*! \brief Gets the file's path. (Debug function. Don't use in Final builds!)
	    \return The file's path.*/
#ifndef TT_BUILD_FINAL
	inline const char* getPath() const { return m_path.c_str(); }
#else
	inline const char* getPath() const { return ""; }
#endif
	
	/*! \brief Return whether the file is in append mode.
	    \return Whether the file is in append mode.*/
	inline bool getAppendMode() const { return m_append; }
	
	/*! \brief Return whether the file is in binary mode.
	    \return Whether the file is in binary mode.*/
	inline bool getBinaryMode() const { return m_binary; }
	
private:
	/*! \brief Constructs a file
	    \param p_path The path of the file.
	    \param p_identifier The filesystem assigned to the file.*/
	File(const std::string& p_path, identifier p_identifier)
	:
#ifndef TT_BUILD_FINAL
	m_path(p_path),
#endif
	m_identifier(p_identifier),
	m_data(0),
	m_append(false),
	m_binary(true),
	m_this()
	{
		(void)p_path;
	}
	
	/*! \brief Destructs a file. */
	~File()
	{
		fs::close(this);
#ifndef TT_BUILD_FINAL
		TT_ASSERTMSG(m_data == 0, "Cleanup failed for file '%s'", m_path.c_str());
#endif
	}
	
	File(const File& p_rhs);
	File& operator=(const File& p_rhs);
	
	static void deleteFile(File* p_file);
	
	friend FilePtr open(const std::string&, OpenMode, identifier);
	friend bool close(File*);
	
	
#ifndef TT_BUILD_FINAL
	std::string m_path;
#endif
	identifier  m_identifier;
	void*       m_data;
	bool        m_append;
	bool        m_binary;
	
	tt_ptr<File>::weak m_this;
};

// namespace end
}
}


#endif // INC_TT_FS_FILE_H
