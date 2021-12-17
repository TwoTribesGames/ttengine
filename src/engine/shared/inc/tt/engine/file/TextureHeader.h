#if !defined(INC_TT_ENGINE_FILE_TEXTUREHEADER_H)
#define INC_TT_ENGINE_FILE_TEXTUREHEADER_H


#include <tt/platform/tt_types.h>
#include <tt/streams/BIStream.h>
#include <tt/streams/BOStream.h>


namespace tt {
namespace engine {
namespace file {


enum CompressionType
{
	CompressionType_PNG,
	CompressionType_DDS,
	CompressionType_ARGB, // Uncompressed
	CompressionType_DXT1,
	CompressionType_DXT3,
	CompressionType_DXT5,
	CompressionType_FastLZ,
	CompressionType_LZ4,
	CompressionType_LZ4HC
};


enum TextureFlag
{
	TextureFlag_Premultiplied = 0x1,
	TextureFlag_SRGB = 0x2,
	TextureFlag_Paintable = 0x4,
};


struct TextureHeader
{
	u16 width;
	u16 height;
	u16 depth;
	u16 mipmapLevels;

	u8  compression;
	u8  pixelFormat;
	u8  flags;
	u8  reserved;

	TextureHeader()
	:
	width(0),
	height(0),
	depth(0),
	mipmapLevels(0),
	compression(0),
	pixelFormat(0),
	flags(0),
	reserved(0)
	{ }
};


// namespace end
}
}
}

inline tt::streams::BOStream& operator<<(tt::streams::BOStream& p_s,
										 const tt::engine::file::TextureHeader& p_rhs)
{
	p_s << p_rhs.width;
	p_s << p_rhs.height;
	p_s << p_rhs.depth;
	p_s << p_rhs.mipmapLevels;
	p_s << p_rhs.compression;
	p_s << p_rhs.pixelFormat;
	p_s << p_rhs.flags;
	p_s << p_rhs.reserved;

	return p_s;
}


inline tt::streams::BIStream& operator>>(tt::streams::BIStream& p_s,
										 tt::engine::file::TextureHeader& p_rhs)
{
	p_s >> p_rhs.width;
	p_s >> p_rhs.height;
	p_s >> p_rhs.depth;
	p_s >> p_rhs.mipmapLevels;
	p_s >> p_rhs.compression;
	p_s >> p_rhs.pixelFormat;
	p_s >> p_rhs.flags;
	p_s >> p_rhs.reserved;

	return p_s;
}



#endif // INC_TT_ENGINE_FILE_TEXTUREHEADER_H
