#if !defined(INC_TT_FS_BUFFEREDFILESYSTEM_H)
#define INC_TT_FS_BUFFEREDFILESYSTEM_H


#include <tt/fs/FileSystem.h>
#include <tt/fs/PassThroughFileSystem.h>
#include <tt/fs/types.h>
#include <tt/mem/types.h>


namespace tt {
namespace fs {

/*! \brief BufferedFileSystem can be used to buffer the IO access from another filesystem.
           Used to improve performance of lots of small IO requests by turning them into a smaller 
           number of bigger reads/writes.
           
           Buffering is done in small chunks per file. (size is p_bufferSize which is set during 
           instantiate.) When doing IO for the first time this buffer is filled with as much data 
           that fits. All subsequential requests is done to/from this buffer. (So from memory and 
           not as actual (disc) IO.) When a request needs bytes which aren't in the buffer (e.g. 
           the end of the buffer has been reached) the next part of the file is copied into the 
           buffer which is once again done as a single big IO request.
           
     \note Not all open modes are supported. */
class BufferedFileSystem : public PassThroughFileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier,
	                                 fs::identifier p_source,
	                                 fs::size_type  p_bufferSize,
	                                 mem::size_type p_bufferAlignment   = 4,
	                                 bool           p_allowBufferBypass = true);
	
	virtual ~BufferedFileSystem();
	
	// NOTE: Since BufferedFileSystem has its own custom File internal data,
	//       all functions that operate on a File instance need to be overridden
	//       (cannot simply be passed to the source file system, since it expects different internal data)
	
	// File functions
	
	virtual bool open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	virtual bool close(File* p_file);
	virtual size_type read (const FilePtr& p_file, void* p_buffer, size_type p_length);
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
	virtual code::BufferPtr getFileContent(const FilePtr& p_file);
	using PassThroughFileSystem::getFileContent;
	
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
	
private:
	BufferedFileSystem(identifier     p_id,
	                   identifier     p_source,
	                   size_type      p_bufferSize,
	                   mem::size_type p_bufferAlignment,
	                   bool           p_allowBufferBypass);
	
	BufferedFileSystem(const BufferedFileSystem& p_rhs);
	BufferedFileSystem& operator=(const BufferedFileSystem& p_rhs);
	
	
	const size_type      m_bufferSize;
	const mem::size_type m_bufferAlignment;
	
	// Whether this FS is allowed to bypass its internal buffering in certain situations
	// (such as when a requested read or write is larger than the internal buffer)
	const bool m_allowBufferBypass;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_FS_BUFFEREDFILESYSTEM_H)
