#ifndef INC_FS_MEMORYARCHIVE_H
#define INC_FS_MEMORYARCHIVE_H

#include <string>
#include <map>

#include <tt/code/Buffer.h>
#include <tt/fs/types.h>
#include <tt/math/hash/Hash.h>


namespace tt {
namespace fs {

class MemoryArchive
{
public:
	enum CompressionType
	{
		CompressionType_None,
		CompressionType_FastLZ,
		CompressionType_LZ4,
		CompressionType_LZ4HC,
		CompressionType_LZMA
	};
	
	struct File
	{
		File()
		:
		path(),
		writeTime(0),
		content()
		{ }
		
		std::string     path;
		fs::time_type   writeTime;
		code::BufferPtr content;
	};
	typedef tt::math::hash::Hash<32> NameHash;
	typedef std::map<NameHash, File> Files;
	
	MemoryArchive();
	
	static MemoryArchivePtr load(const std::string& p_path);
	bool                    save(const std::string& p_path,
	                             CompressionType    p_compressionType,
	                             u32                p_fileAlignment = 32);
	bool            hasFile       (const std::string& p_relativeFilePath) const;
	bool            addFile       (const std::string& p_relativeFilePath,
	                               const std::string& p_relativeRootPath);
	File            getFile       (const std::string& p_relativeFilePath) const;
	code::BufferPtr getFileContent(const std::string& p_relativeFilePath) const;
	
	inline const Files& getFiles () const { return m_files; }
private:
	MemoryArchive(const code::BufferPtr& p_buffer);
	MemoryArchive(const MemoryArchive&);
	MemoryArchive& operator=(const MemoryArchive&);
	
	bool addFile(const File& p_file);
	
	typedef std::map<std::string, code::BufferPtr> MemFiles;
	
	Files           m_files;
	code::BufferPtr m_archive;
};

}
}

#endif // INC_FS_MEMORYARCHIVE_H
