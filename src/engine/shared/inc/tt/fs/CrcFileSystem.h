#if !defined(INC_TT_FS_CRCFILESYSTEM_H)
#define INC_TT_FS_CRCFILESYSTEM_H


#include <map>
#include <vector>

#include <tt/fs/FileSystem.h>
#include <tt/fs/PassThroughFileSystem.h>
#include <tt/fs/types.h>


namespace tt {
namespace fs {

/*! \brief CrcFileSystem automatically adds CRC checking to files. 
           Because the crc is calculated it is not allowed to mix 
           reading and writing or to seek in the file. 
           The CRC and filesize are written at the start of the file 
           when it is closed and the crc is checked when the file is 
           opened again returning an empty file when the check fails
     \note Not all open modes are supported. */
class CrcFileSystem : public PassThroughFileSystem
{
public:
	static FileSystemPtr instantiate(fs::identifier p_identifier,
	                                 fs::identifier p_source);
	
	virtual ~CrcFileSystem(){}
	
	// File functions
	
	virtual bool open(const FilePtr& p_file, const std::string& p_path, OpenMode p_mode);
	virtual bool close(File* p_file);
	virtual size_type read(const FilePtr& p_file, void* p_buffer, size_type p_length);
	virtual size_type write(const FilePtr& p_file, const void* p_buffer, size_type p_length);
	
	// Position / size functions
	
	virtual bool seek(const FilePtr& p_file, pos_type p_offset, SeekPos p_position);
	virtual pos_type getPosition(const FilePtr& p_file);
	virtual size_type getLength(const FilePtr& p_file);
	
private:
	CrcFileSystem(identifier p_id, identifier p_source);
	
	CrcFileSystem(const CrcFileSystem& p_rhs);
	CrcFileSystem& operator=(const CrcFileSystem& p_rhs);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_FS_CRCFILESYSTEM_H)
