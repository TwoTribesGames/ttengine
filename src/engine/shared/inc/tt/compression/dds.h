#ifndef INC_TT_COMPRESSION_DDS_H
#define INC_TT_COMPRESSION_DDS_H


#include <tt/compression/image.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace file {
	struct TextureHeader;
}
}
namespace compression {


bool decompressDDS(const fs::FilePtr& p_file, PixelData& p_out);

// Used by Asset Tool to get DDS header info
bool fillTextureHeaderFromDDS(const fs::FilePtr& p_file, engine::file::TextureHeader& p_header);

// Namespace end
}
}

#endif // INC_TT_COMPRESSION_DDS_H
