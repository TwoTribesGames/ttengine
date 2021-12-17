#ifndef INC_TT_COMPRESSION_PVRTC_H
#define INC_TT_COMPRESSION_PVRTC_H

#include <string>
#include <tt/compression/image.h>
#include <tt/fs/types.h>


namespace tt {
namespace engine {
namespace file {
struct TextureHeader;
}
}
namespace compression {
		
		
bool decompressPVRTC(const std::string& p_filename, PixelData& p_out);
		

// Namespace end
}
}

#endif // INC_TT_COMPRESSION_PVRTC_H
