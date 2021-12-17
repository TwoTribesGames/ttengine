#ifndef INC_TT_COMPRESSION_PNG_H
#define INC_TT_COMPRESSION_PNG_H

#include <libpng/png.h>

#include <tt/compression/image.h>
#include <tt/fs/types.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace compression {


enum PixelTransformation
{
	Transform_None                 = 0x0,
	Transform_ConvertToRGB         = 0x1,
	Transform_ConvertToLuminance   = 0x2,
	Transform_AddAlphaChannel      = 0x4,
	Transform_StripAlphaChannel    = 0x8,
	Transform_SwapAlphaChannel     = 0x10, // ARGB format
	Transform_SwapColorChannels    = 0x20  // BGR format
};


// Support for loading PNG images from memory
struct PNGSourceData
{
	explicit PNGSourceData(const u8* p_data, s32 p_length, const char* p_filename)
	:
	data(p_data),
	length(p_length),
	currentReadByteIndex(0),
	filename(p_filename),
	maxDimensions(math::Point2::zero),
	transforms(Transform_None)
	{ }
	
	const u8*    data;
	s32          length;
	s32          currentReadByteIndex;
	const char*  filename;
	math::Point2 maxDimensions;
	u32          transforms;
};

// Support for writing PNG images to file
struct PNGTargetData
{
	inline PNGTargetData(const fs::FilePtr& p_outFile,
	                     u32                p_transforms)
	:
	outFile(p_outFile),
	transforms(p_transforms)
	{ }
	
	fs::FilePtr outFile;
	u32         transforms;
};


// Decompression

bool decompressPNG(const fs::FilePtr&  p_file,
	               PixelData&          p_out,
	               u32                 p_transforms    = Transform_None,
	               const math::Point2& p_maxDimensions = math::Point2::zero);

bool decompressPNG(PNGSourceData& p_source, PixelData& p_out);


// Compression

bool compressPNG(const fs::FilePtr&  p_outputFile,
                 const PixelData&    p_pixelData,
                 u32                 p_transforms = Transform_None);

bool compressPNG(PNGTargetData& p_target, const PixelData& p_pixelData);


// Helper functions

struct PNGHeader;
void modifySettingsForRGBA(png_structp p_png, const PNGHeader& p_header, const PNGSourceData& p_src);


// Namespace end
}
}

#endif // INC_TT_COMPRESSION_PNG_H
