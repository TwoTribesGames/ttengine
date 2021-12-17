#include <algorithm>
#include <cstdio>

#include <tt/fs/File.h>
#include <tt/fs/BufferedFileSystem.h>
#include <tt/math/math.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>


namespace tt {
namespace fs {


struct BufferInternal
{
	u8* const       buffer;         //!< Buffer for storing the data from the file for fast access.
	                                //   Is allocated once / fixed size.
	const size_type bufferSize;
	FilePtr         file;           //!< Pointer to the file which is being buffered.
	pos_type        bufferPosition; //!< Position within the buffer where current IO operations are.
	size_type       bufferReadUsageSize; //!< Size for which part of the buffer is being used.
	
	//! \brief Dirty range for when the buffer is written to.
	//         It indicates which range (always starting at 0) should be written back to source FS.
	size_type       bufferWriteUsageRange;
	
	bool bufferContainsWholeFile;
	
	
	inline BufferInternal(size_type p_bufferSize, mem::size_type p_bufferAlignment)
	:
	buffer(reinterpret_cast<u8*>(mem::alloc(p_bufferSize, p_bufferAlignment))),
	bufferSize(p_bufferSize),
	file(),
	bufferPosition(0),
	bufferReadUsageSize(0),
	bufferWriteUsageRange(0),
	bufferContainsWholeFile(false)
	{
		TT_NULL_ASSERT(buffer);
	}
	
	inline ~BufferInternal()
	{
		if (isWriteDirty())
		{
			flushWriteBuffer();
		}
		
		mem::free(buffer);
	}
	
	/*! \brief Return how many bytes there are left to be read from the buffer before a buffer 
	           refill is needed. */
	inline size_type getReadBytesRemainingInBuffer() const
	{
		return static_cast<size_type>(bufferReadUsageSize - bufferPosition);
	}
	
	inline bool isReadAtEndOfBuffer() const { return bufferPosition == static_cast<pos_type>(bufferReadUsageSize); }
	inline bool isReadFilled()        const { return bufferReadUsageSize >  0;               }
	inline bool isReadEmpty()         const { return bufferReadUsageSize <= 0;               }
	
	inline bool isWriteDirty()        const { return bufferWriteUsageRange > 0;           }
	// \return Whether the buffer can grow any larger. (for writes)
	inline bool isWriteFilledCompletely() const { return bufferWriteUsageRange == bufferSize; }
	
	/*! \brief Return how many bytes there are left to be written in the buffer before a buffer 
	           flush is needed. */
	inline size_type getBytesRemainingToWriteInBuffer() const
	{
		return static_cast<size_type>(bufferSize - bufferPosition);
	}
	
	inline bool isBufferUsed() const { return bufferReadUsageSize > 0 || bufferWriteUsageRange > 0; }
	
	void resetBuffer()
	{
		if (file == 0)
		{
			return;
		}
		bufferPosition      = 0;
		bufferReadUsageSize = 0;
		TT_ASSERTMSG(bufferWriteUsageRange == 0,
		             "Buffer was reset without first writing buffer changes back to source FS!");
		
		bufferWriteUsageRange = 0;
	}
	
	inline void refillReadBufferIfneeded()
	{
		// When buffer contains the entirety of the file's contents, there's never a need
		// to refill buffers (unless the contents weren't read yet).
		// Otherwise, the buffer should be refilled if we've reached the end of the buffer.
		if (isReadAtEndOfBuffer() &&
		    (bufferContainsWholeFile == false || isBufferUsed() == false))
		{
			refillReadBuffer();
		}
	}
	
	void refillReadBuffer()
	{
		if (file == 0)
		{
			return;
		}
		
		flushWriteBuffer();
		
		bufferPosition  = 0;
		
		size_type read = file->read(buffer, bufferSize);
		
		if (read <= 0)
		{
			bufferReadUsageSize = 0;
		}
		else
		{
			TT_ASSERT(isWriteDirty() == false); // Can't handle read/write at same time.
			bufferReadUsageSize = read;
			TT_ASSERT(file->getPosition() >= static_cast<pos_type>(bufferWriteUsageRange));
		}
		bufferPosition = 0;
	}
	
	void flushWriteBuffer()
	{
		if (isBufferUsed() == false)
		{
			return;
		}
		
		if (isWriteDirty())
		{
			// Reads will move file position.
			// If we need to read the file position needs to be moved back.
			if (bufferReadUsageSize > 0)
			{
				TT_PANIC("Read and write at the same time is not supported. (Will not work, and might crash!)");
				
				TT_ASSERTMSG(file->getPosition() >= static_cast<pos_type>(bufferReadUsageSize),
				             "Can't restore position by moving back %d from position %d!", 
				             static_cast<pos_type>(bufferReadUsageSize), file->getPosition());
				// Move back to the same position as buffer.
				bool result = file->seek(-static_cast<pos_type>(bufferReadUsageSize), SeekPos_Cur);
				
				TT_ASSERTMSG(result,
				             "Failed to seek to new file position while flushing write buffer."
				             "(Trying to seek: %d, from current pos. file pos: %d, file size: %u)",
				             -static_cast<pos_type>(bufferReadUsageSize),
				             file->getPosition(),
				             file->getLength());
			}
			
			// Write the dirty write range.
			size_type result = file->write(buffer, bufferWriteUsageRange);
			
			// FIXME: Find a better way to report this.
			//        The actual write call might be very old.
			TT_ASSERTMSG(result == bufferWriteUsageRange,
			             "Failed to write buffered changes!");
			
			// Check if read was further than write.
			// If so the file position needs to be moved futher as well.
			if (bufferReadUsageSize >= bufferWriteUsageRange)
			{
				// If more was read behind the part that was written,
				// make sure the file is at correct position.
				
				// Make sure the buffer pointer position is used to correct the intern file's position.
				bool seekResult = file->seek(static_cast<pos_type>(bufferWriteUsageRange - bufferReadUsageSize),
				                             SeekPos_Cur);
				TT_ASSERTMSG(seekResult,
				             "Failed to seek to new file position (after write) while flushing write buffer."
				             "(Trying to seek: %d, from current pos. file pos: %d, file size: %u)",
				             static_cast<pos_type>(bufferWriteUsageRange - bufferReadUsageSize),
				             file->getPosition(),
				             file->getLength());
			}
			
			// Write usages written back to source FS, reset range.
			bufferWriteUsageRange = 0;
		}
		else
		{
			TT_ASSERT(bufferPosition <= static_cast<pos_type>(bufferReadUsageSize));
			
			// Make sure the file position is set to the position the buffer is.
			if (bufferPosition < static_cast<pos_type>(bufferReadUsageSize))
			{
				file->seek(static_cast<pos_type>(bufferPosition - bufferReadUsageSize), SeekPos_Cur);
			}
		}
		
		// Because we bypassed the buffer it no longer contains valid data.
		// So it needs to be reset.
		resetBuffer();
	}
	
private:
	// No copying
	BufferInternal(const BufferInternal&);
	BufferInternal& operator=(const BufferInternal&);
};


//--------------------------------------------------------------------------------------------------
// Public member functions

FileSystemPtr BufferedFileSystem::instantiate(identifier     p_identifier,
                                              identifier     p_source,
                                              size_type      p_bufferSize,
                                              mem::size_type p_bufferAlignment,
                                              bool           p_allowBufferBypass)
{
	FileSystemPtr filesys(new BufferedFileSystem(
			p_identifier, p_source, p_bufferSize, p_bufferAlignment, p_allowBufferBypass));
	if (filesys == 0)
	{
		TT_PANIC("Failed to instantiate filesystem.");
		return filesys;
	}
	
	if (fs::registerFileSystem(filesys.get(), p_identifier) == false)
	{
		TT_PANIC("Failed to register filesytem.");
		filesys.reset();
	}
	
	return filesys;
}


BufferedFileSystem::~BufferedFileSystem()
{
}


bool BufferedFileSystem::open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_path.empty())
	{
		TT_PANIC("No path specified.");
		return false;
	}
	
	if (p_file->getData() != 0)
	{
		TT_PANIC("File already open.");
		return false;
	}
	
	// Note: Maybe truncate works with the addition of write support. (Test first!)
	if ((p_mode & OpenMode_Truncate) != 0)
	{
		TT_PANIC("Write mode OpenMode_Truncate not available.");
		return false;
	}
	
	// Note: Maybe append works with the addition of write support. (Test first!)
	if ((p_mode & OpenMode_Append) != 0)
	{
		TT_PANIC("Write mode OpenMode_Append not available.");
		return false;
	}
	
	FilePtr sourceFile = fs::open(p_path, p_mode, getSourceID());
	if (sourceFile == 0)
	{
		return false;
	}
	
	// In read mode, don't make a buffer that's larger than the file size (taking the required alignment into account)
	bool      bufferIsFileSized = false;
	size_type bufferSizeToUse   = m_bufferSize;
	if ((p_mode & OpenMode_Read) != 0)
	{
		const size_type fileSize        = fs::getLength(sourceFile);
		const size_type alignedFileSize = math::roundUp(fileSize, m_bufferAlignment);
		
		if (alignedFileSize > 0 && alignedFileSize <= bufferSizeToUse)
		{
			bufferIsFileSized = true;
			bufferSizeToUse   = alignedFileSize;
		}
	}
	
	TT_ASSERT(bufferSizeToUse > 0);
	
	BufferInternal* intern = new BufferInternal(bufferSizeToUse, m_bufferAlignment);
	intern->file                    = sourceFile;
	intern->bufferContainsWholeFile = bufferIsFileSized;
	intern->resetBuffer();
	
	p_file->setData(reinterpret_cast<void*>(intern));
	return true;
}


bool BufferedFileSystem::close(File* p_file)
{
	if (p_file == 0)
	{
		TT_PANIC("No file specified.");
		return false;
	}
	
	if (p_file->getData() == 0)
	{
		TT_WARN("File not open.");
		return false;
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	delete intern;
	p_file->setData(0);
	
	return true;
}


size_type BufferedFileSystem::read(const FilePtr& p_file, void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	// Check if requested size is larger than buffer size.
	if (m_allowBufferBypass && p_length > intern->bufferSize)
	{
		// In this case it's better to use the file that's being buffered directly.
		intern->flushWriteBuffer();
		return intern->file->read(p_buffer, p_length);
	}
	
	size_type bytesRead = 0;
	u8* dest = static_cast<u8*>(p_buffer);
	intern->refillReadBufferIfneeded();
	while (p_length > 0)
	{
		// How many bytes can we read?
		size_type todo = std::min(intern->getReadBytesRemainingInBuffer(), p_length);
		TT_ASSERT(todo >= 0);
		if (todo == 0)
		{
			return bytesRead;
		}
		
		const u8* source = intern->buffer + intern->bufferPosition;
		mem::copy8(dest, source, static_cast<mem::size_type>(todo));
		bytesRead += todo;
		p_length -= todo;
		dest += todo;
		intern->bufferPosition += static_cast<pos_type>(todo);
		intern->refillReadBufferIfneeded();
	}
	
	return bytesRead;
}


size_type BufferedFileSystem::readAsync(const FilePtr& p_file,
                                        void*          p_buffer,
                                        size_type      p_length,
                                        callback       p_callback,
                                        void*          p_arg)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::readAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


size_type BufferedFileSystem::write(const FilePtr& p_file, const void* p_buffer, size_type p_length)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	if (p_buffer == 0)
	{
		TT_PANIC("No buffer specified.");
		return size_type();
	}
	
	if (p_length == 0)
	{
		return p_length;
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	// Check if requested size is larger than buffer size.
	if (m_allowBufferBypass && p_length > intern->bufferSize)
	{
		// In this case it's better to use the file that's being buffered directly.
		intern->flushWriteBuffer();
		return intern->file->write(p_buffer, p_length);
	}
	
	size_type bytesWritten = 0;
	const u8* source = static_cast<const u8*>(p_buffer);
	if (intern->isWriteFilledCompletely())
	{
		intern->flushWriteBuffer();
	}
	while (p_length > 0)
	{
		
		// How many bytes can we write?
		size_type todo = std::min(intern->getBytesRemainingToWriteInBuffer(), p_length);
		if (todo == 0)
		{
			return bytesWritten;
		}
		
		u8* dest = intern->buffer + intern->bufferPosition;
		mem::copy8(dest, source, static_cast<mem::size_type>(todo));
		bytesWritten           += todo;
		source                 += todo;
		intern->bufferPosition += static_cast<pos_type>(todo);
		TT_ASSERT(intern->bufferReadUsageSize <= 0); // Can't handle read/write at same time.
		intern->bufferWriteUsageRange = std::max(intern->bufferWriteUsageRange,
		                                         static_cast<size_type>(intern->bufferPosition));
		p_length               -= todo;
		
		if (intern->isWriteFilledCompletely())
		{
			intern->flushWriteBuffer();
		}
	}
	
	return bytesWritten;
}


size_type BufferedFileSystem::writeAsync(const FilePtr& p_file,
                                         const void*    p_buffer,
                                         size_type      p_length,
                                         callback       p_callback,
                                         void*          p_arg)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::writeAsync(p_file, p_buffer, p_length, p_callback, p_arg);
}


bool BufferedFileSystem::flush(const FilePtr& /*p_file*/)
{
	// FIXME: What to do here?
	return true;
}


code::BufferPtr BufferedFileSystem::getFileContent(const FilePtr& p_file)
{
	// Forward this call to the source file system directly (no need to buffer the entire file content)
	if (validate(p_file) == false)
	{
		return code::BufferPtr();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	FileSystem* source = getSource();
	if (source == 0)
	{
		TT_WARN("Source file system went away.");
		return code::BufferPtr();
	}
	
	return source->getFileContent(intern->file);
}


bool BufferedFileSystem::isBusy(const FilePtr& /*p_file*/)
{
	// Async I/O is not supported by BufferedFileSystem: ignore calls
	return false;
}


bool BufferedFileSystem::isSucceeded(const FilePtr& /*p_file*/)
{
	// Async I/O is not supported by BufferedFileSystem: ignore calls
	return true;
}


bool BufferedFileSystem::wait(const FilePtr& /*p_file*/)
{
	// Async I/O is not supported by BufferedFileSystem: ignore calls
	return true;
}


bool BufferedFileSystem::cancel(const FilePtr& /*p_file*/)
{
	// Async I/O is not supported by BufferedFileSystem: ignore calls
	return true;
}


bool BufferedFileSystem::seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position)
{
	if (validate(p_file) == false)
	{
		return false;
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	if (intern->bufferContainsWholeFile)
	{
		// We can perform cheaper seeks for files that are buffered completely
		pos_type newPos = intern->bufferPosition;
		switch (p_position)
		{
		case SeekPos_Set: newPos  = p_offset;                               break;
		case SeekPos_Cur: newPos += p_offset;                               break;
			// bufferReadUsageSize for fully buffered files is the length of the file
		case SeekPos_End: newPos  = intern->bufferReadUsageSize + p_offset; break;
		}
		
		// <= because SeekPos_End with offset 0 should work: a subsequent getPosition will then return the size of file
		if (newPos >= 0 && newPos <= static_cast<pos_type>(intern->bufferReadUsageSize))
		{
			intern->bufferPosition = newPos;
			return true;
		}
		
		// Entire file is buffered, but new position is not in range: cannot seek
		return false;
	}
	
	if (intern->isBufferUsed())
	{
		// A posible performance improvement is checking if the new position is still within the 
		// current buffer. If this is the case just moving the buffer position is enough.
		
		const pos_type startOfBufInFilePos =
				intern->file->getPosition()                           // File pos
				- static_cast<pos_type>(intern->bufferReadUsageSize); // - minus buffer part.
		
		switch (p_position)
		{
		case SeekPos_Set:
			if (p_offset >= startOfBufInFilePos &&
			    p_offset <  startOfBufInFilePos + static_cast<pos_type>(intern->bufferReadUsageSize))
			{
				// New position will fall within buffer.
				intern->bufferPosition = static_cast<pos_type>(p_offset - startOfBufInFilePos);
				return true;
			}
			break;
			
		case SeekPos_Cur:
			{
				const pos_type newBufferPos = intern->bufferPosition + p_offset;
				if (newBufferPos >= 0 && newBufferPos < static_cast<pos_type>(intern->bufferReadUsageSize))
				{
					intern->bufferPosition = newBufferPos;
					return true;
				}
			}
			break;
			
		case SeekPos_End:
			{
				pos_type length = static_cast<pos_type>(getLength(p_file));
				if (length + p_offset >= startOfBufInFilePos &&
				    length + p_offset <  startOfBufInFilePos +
				                         static_cast<pos_type>(intern->bufferReadUsageSize))
				{
					// New position will fall within buffer.
					intern->bufferPosition = static_cast<pos_type>(length + p_offset - startOfBufInFilePos);
					return true;
				}
			}
			break;
		}
	}
	
	// Will correct the file position of the interal file so no longer points at end of buffer.
	intern->flushWriteBuffer();
	
	return intern->file->seek(p_offset, p_position);
}


bool BufferedFileSystem::seekToBegin(const FilePtr& p_file)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::seekToBegin(p_file);
}


bool BufferedFileSystem::seekToEnd(const FilePtr& p_file)
{
	// NOTE: Explicitly using FileSystem's default implementation,
	//       since these calls shouldn't be forwarded to the source file system
	return FileSystem::seekToEnd(p_file);
}


pos_type BufferedFileSystem::getPosition(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return pos_type();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	return intern->file->getPosition()                          // File pos
	       - static_cast<pos_type>(intern->bufferReadUsageSize) // - minus buffer part.
	       + static_cast<pos_type>(intern->bufferPosition);     // + internal pos.
}


size_type BufferedFileSystem::getLength(const FilePtr& p_file)
{
	if (validate(p_file) == false)
	{
		return size_type();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	return intern->file->getLength() + intern->bufferWriteUsageRange;
}


time_type BufferedFileSystem::getCreationTime(const FilePtr& p_file)
{
	// Forward this call to the source file system directly
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	FileSystem* source = getSource();
	if (source == 0)
	{
		TT_WARN("Source file system went away.");
		return time_type();
	}
	
	return source->getCreationTime(intern->file);
}


time_type BufferedFileSystem::getAccessTime(const FilePtr& p_file)
{
	// Forward this call to the source file system directly
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	FileSystem* source = getSource();
	if (source == 0)
	{
		TT_WARN("Source file system went away.");
		return time_type();
	}
	
	return source->getAccessTime(intern->file);
}


time_type BufferedFileSystem::getWriteTime(const FilePtr& p_file)
{
	// Forward this call to the source file system directly
	if (validate(p_file) == false)
	{
		return time_type();
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	
	FileSystem* source = getSource();
	if (source == 0)
	{
		TT_WARN("Source file system went away.");
		return time_type();
	}
	
	return source->getWriteTime(intern->file);
}


bool BufferedFileSystem::setWriteTime(const FilePtr& p_file, time_type p_time)
{
	// Forward this call to the source file system directly
	if (validate(p_file) == false)
	{
		return false;
	}
	
	FileSystem* source = getSource();
	if (source == 0)
	{
		TT_WARN("Source file system went away.");
		return false;
	}
	
	BufferInternal* intern = reinterpret_cast<BufferInternal*>(p_file->getData());
	return source->setWriteTime(intern->file, p_time);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

BufferedFileSystem::BufferedFileSystem(identifier     p_id,
                                       identifier     p_source,
                                       size_type      p_bufferSize,
                                       mem::size_type p_bufferAlignment,
                                       bool           p_allowBufferBypass)
:
PassThroughFileSystem(p_id, p_source),
m_bufferSize(p_bufferSize),
m_bufferAlignment(p_bufferAlignment),
m_allowBufferBypass(p_allowBufferBypass)
{
}

// Namespace end
}
}
