#ifndef INC_TT_COMPRESSION_IMAGE_H
#define INC_TT_COMPRESSION_IMAGE_H


#include <tt/platform/tt_types.h>


namespace tt {

// Image formats
enum ImageFormat
{
	ImageFormat_ARGB8, // DDS native
	ImageFormat_RGBA8, // PNG native
	ImageFormat_A8,
	ImageFormat_I8,
	ImageFormat_IA8,

	ImageFormat_BC1,
	ImageFormat_BC2,
	ImageFormat_BC3,
	ImageFormat_BC4,
	ImageFormat_BC5,
	
	ImageFormat_PVRTC_2BPP_RGBA,
	ImageFormat_PVRTC_2BPP_RGB,
	ImageFormat_PVRTC_4BPP_RGBA,
	ImageFormat_PVRTC_4BPP_RGB,

	ImageFormat_Invalid
};


namespace compression {

// Information about the returned pixel data
// NOTE: Allocates data for image returned by pixels member
//       Pixel data is 4-byte aligned and must be freed by caller with tt::mem::free()
struct PixelData
{
	PixelData()
	:
	pixels(0),
	format(ImageFormat_Invalid),
	bitDepth(0),
	width(0),
	height(0),
	depth(0),
	pitch(0)
	{ }

	u8*         pixels;
	ImageFormat format;
	s32         bitDepth;
	s32         width;
	s32         height;
	s32         depth;
	s32         pitch;
};


// Namespace end
}
}

#endif // INC_TT_COMPRESSION_IMAGE_H
