#if !defined(INC_TT_FS_UTILS_MEMORYARCHIVE_UTILS_H)
#define INC_TT_FS_UTILS_MEMORYARCHIVE_UTILS_H

#include <string>
#include <vector>

#include <tt/code/fwd.h>
#include <tt/fs/types.h>
#include <tt/fs/MemoryArchive.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace fs {
namespace utils {

struct MemoryArchivePackSettings
{
	MemoryArchivePackSettings()
	:
	verbose(false),
	alignment(32),
	compressionType(MemoryArchive::CompressionType_None),
	deleteOriginals(false),
	emptyOriginals(false),
	emptySize(0)
	{
	}
	
	bool                           verbose;
	u32                            alignment;
	MemoryArchive::CompressionType compressionType;
	bool                           deleteOriginals;
	bool                           emptyOriginals;
	s32                            emptySize;  // size to make originals when emptyOriginals is enabled
};


struct MemoryArchiveUnpackSettings
{
	MemoryArchiveUnpackSettings()
	:
	verbose(false),
	keepNewer(false)
	{ }
	
	bool verbose;
	bool keepNewer;
};


bool MemoryArchivePack  (const std::string& p_configFile,
                         const std::string& p_inputPath,
                         const std::string& p_outputPath,
                         const MemoryArchivePackSettings& p_settings);

bool MemoryArchiveUnpack(const std::string& p_archiveFile,
                         const std::string& p_outputPath,
                         const MemoryArchiveUnpackSettings& p_settings);


// Namespace end
}
}
}


#endif  // !defined(INC_TT_FS_UTILS_MEMORYARCHIVE_UTILS_H)
