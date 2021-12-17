// Prevent multiple inclusion
#if !defined(INC_TT_ENGINE_RENDERER_IMAGETYPES_H)
#define INC_TT_ENGINE_RENDERER_IMAGETYPES_H

namespace tt {
namespace engine {
namespace renderer {

enum ImageType
{
	// Nitro types
	ImageType_Nitro_4Col = 0,
	ImageType_Nitro_16Col,
	ImageType_Nitro_256Col,
	ImageType_Nitro_16BitCol,
	ImageType_Nitro_32Alpha8Col,
	ImageType_Nitro_8Alpha32Col,
	ImageType_Nitro_2Col,
	ImageType_Nitro_4x4Compressed,

	ImageType_LastNitroTypeSentinel,

	// Revolution types
	ImageType_Rvl_RGBA8 = 16,
	ImageType_Rvl_RGB5A3,
	ImageType_Rvl_RGB565,
	ImageType_Rvl_CMPR,
	ImageType_Rvl_I4,  // Intensity
	ImageType_Rvl_I8,
	ImageType_Rvl_IA4, // Intensity + Alpha
	ImageType_Rvl_IA8,
	ImageType_Rvl_CI4, // Color Indexed
	ImageType_Rvl_CI8,
	ImageType_Rvl_CI14,

	ImageType_LastRvlTypeSentinel,

	// Generic types
	ImageType_Generic_ARGB8 = 32,
	ImageType_Generic_PNG,

	ImageType_LastGenericTypeSentinel,

	// Undefined
	ImageType_Undefined = 128
};

}
}
}

#endif // INC_TT_ENGINE_RENDERER_IMAGETYPES_H
