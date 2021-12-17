#include <limits>
#include <vector>

#include <tt/compression/dds.h>

#include <tt/code/bufferutils.h>
#include <tt/engine/file/TextureHeader.h>
#include <tt/fs/File.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>


namespace tt {
namespace compression {

// Four character code
inline u32 fourCC(char p_0, char p_1, char p_2, char p_3)
{
	return (p_0 | (p_1 << 8) | (p_2 << 16) | (p_3 << 24));
}

inline bool isFlagSet(u32 p_flags, u32 p_flagToCheck)
{
	return (p_flags & p_flagToCheck) == p_flagToCheck;
}


// DDS types & structs
// We define our own versions to be able to use the code on non-Microsoft platforms

// Flags that indicate which header members contain valid data
enum DDSFlag
{
	DDSFlag_Caps        = 0x000001, // Always required
	DDSFlag_Height      = 0x000002, // Always required
	DDSFlag_Width       = 0x000004, // Always required
	DDSFlag_Pitch       = 0x000008, // Required when pitch is provided for *uncompressed* texture
	DDSFlag_PixelFormat = 0x001000, // Always required
	DDSFlag_MipMapCount = 0x020000, // Required for mipmapped texture
	DDSFlag_LinearSize  = 0x080000, // Required when pitch is provided for *compressed* texture
	DDSFlag_Depth       = 0x800000  // Required in a depth texture (probably the docs mean volume texture)
};
static const u32 requiredFlags = (DDSFlag_Caps|DDSFlag_Height|DDSFlag_Width|DDSFlag_PixelFormat);

// Flags indicating data complexity
enum DDSNormalCaps
{
	DDSCaps_Complex = 0x000008,  // File contains multiple surfaces (mipmaps, environment, volume)
	DDSCaps_MipMap  = 0x400000,  // Optional, should be used for mipmaps
	DDSCaps_Texture = 0x001000   // Required
};

enum DDSAdditionalCaps
{
	DDSCaps2_CubeMap           = 0x000200, // Required for cube map
	DDSCaps2_CubeMap_PositiveX = 0x000400, // Required if this face of cube map is stored
	DDSCaps2_CubeMap_NegativeX = 0x000800, // Required if this face of cube map is stored
	DDSCaps2_CubeMap_PositiveY = 0x001000, // Required if this face of cube map is stored
	DDSCaps2_CubeMap_NegativeY = 0x002000, // Required if this face of cube map is stored
	DDSCaps2_CubeMap_PositiveZ = 0x004000, // Required if this face of cube map is stored
	DDSCaps2_CubeMap_NegativeZ = 0x008000, // Required if this face of cube map is stored
	DSSCaps2_Volume            = 0x200000  // Required for volume texture
};


enum DDSPixelFormatEnum
{
	DDSPixelFormat_AlphaPixels = 0x00001, // Texture contains alpha, alphaBitMask is valid
	DDSPixelFormat_Alpha       = 0x00002, // [OLD] Uncompressed alpha only, RGBBitCount has alpha bitcount
	DDSPixelFormat_FourCC      = 0x00004, // Compressed RGB data, fourCC is valid
	DDSPixelFormat_RGB         = 0x00040, // Uncompressed RGB, all RGB members have valid data
	DDSPixelFormat_YUV         = 0x00200, // [OLD] Uncompressed YUV, same as above, but RGB = YUV
	DDSPixelFormat_Luminance   = 0x20000  // [OLD] Uncompressed single channel, redBitMask is valid
};


// Pixel format description
struct DDSPixelFormat
{
	u32 structSize;    // Size of this structure, must be 32
	u32 flags;         // See DDSPixelFormatEnum
	u32 fourCC;        // Four character code indicating compressed / custom format
	u32 RGBBitCount;   // Number of bits in RGB format
	u32 redBitMask;    // Red   mask for reading color data
	u32 greenBitMask;  // Green mask for reading color data
	u32 blueBitMask;   // Blue  mask for reading color data
	u32 alphaBitMask;  // Alpha mask for reading alpha data
};
static const u32 DDSPixelFormatSize = 32;


inline tt::ImageFormat getImageFormat(const DDSPixelFormat& p_ddsFormat)
{
	TT_ASSERTMSG((p_ddsFormat.flags & DDSPixelFormat_FourCC) == 0, "Compressed data in DDS not supported.");
	TT_ASSERTMSG((p_ddsFormat.flags & DDSPixelFormat_YUV   ) == 0, "YUV data in DDS not supported.");

	if (isFlagSet(p_ddsFormat.flags, DDSPixelFormat_Alpha))
	{
		TT_ASSERTMSG(p_ddsFormat.RGBBitCount == 8, "Only 8-bit alpha supported");
		return tt::ImageFormat_A8;
	}
	else if (isFlagSet(p_ddsFormat.flags, DDSPixelFormat_Luminance))
	{
		TT_ASSERTMSG(p_ddsFormat.RGBBitCount == 8, "Only 8-bit luminance supported");
		return tt::ImageFormat_I8;
	}
	else if (isFlagSet(p_ddsFormat.flags, DDSPixelFormat_RGB))
	{
		TT_ASSERTMSG(p_ddsFormat.RGBBitCount == 32, "Only 8-bit RGBA supported");
		if (isFlagSet(p_ddsFormat.flags, DDSPixelFormat_AlphaPixels))
		{
			return tt::ImageFormat_ARGB8;
		}
		/*
		TODO: Support RGB format
		else
		{
			return tt::ImageFormat_RGB8;
		}
		*/
	}
	return tt::ImageFormat_Invalid;
}


// Description of stored surfaces
struct DDSCaps
{
	u32 caps1; // Complexity of data, see DDSCaps
	u32 caps2; // Additional detail, see DDSAdditionalCaps
	u32 caps3; // Unused
	u32 caps4; // Unused
};


// DDS Format Header (DDS_HEADER in DirectX SDK)

struct DDSHeader
{
	u32 structSize;      // Must be set to 124
	u32 flags;           // Combination of DDSFlags
	u32 height;
	u32 width;
	u32 linearSize;      // Pitch for uncompressed, total size of top-level for compressed
	u32 depth;
	u32 mipmapCount;
	u32 reserved1[11];          // Unused
	DDSPixelFormat pixelFormat; // See DDSPixelFormat
	DDSCaps caps;               // See DDSCaps
	u32 reserved2;
};
static const u32 DDSHeaderSize = 124;

// Helper function declarations
bool readDDSHeader(const fs::FilePtr& p_file, DDSHeader& p_header);


//--------------------------------------------------------------------------------------------------
// Decompression

bool decompressDDS(const fs::FilePtr& p_file, PixelData& p_out)
{
	// Read DDS header
	DDSHeader header;
	if(readDDSHeader(p_file, header) == false)
	{
		return false;
	}

	// Handle compression
	if(isFlagSet(header.pixelFormat.flags, DDSPixelFormat_FourCC))
	{
		// DX10 => Data contains extra header
		if(header.pixelFormat.fourCC == fourCC('D','X','1','0'))
		{
			TT_PANIC("DDS DX10 Format not yet supported!");
			return false;
		}

		// TODO: Support compressed textures
		TT_PANIC("Compressed textures are not yet supported");
		return false;
	}

	// TODO: Read DX10 Header if available

	// Make sure the necessary data members are valid
	if(isFlagSet(header.flags, requiredFlags) == false)
	{
		TT_PANIC("Not all required flags are found in DDS header");
		return false;
	}

	// Determine size of data to read
	u32 bytesPerPixel(header.pixelFormat.RGBBitCount / 8);
	u32 pitch(header.width * bytesPerPixel);

	if(isFlagSet(header.flags, DDSFlag_Pitch))
	{
		pitch = header.linearSize;
	}
	TT_ASSERT(pitch == header.width * bytesPerPixel);

	fs::size_type mainSurfaceSize(pitch * header.height);

	if (isFlagSet(header.caps.caps2, DSSCaps2_Volume))
	{
		// Volume Texture
		TT_ASSERT(header.depth >= 1);
		mainSurfaceSize *= header.depth;
	}

	std::vector<u8> pixelData(mainSurfaceSize, 0);
	if(p_file->read(&pixelData[0], mainSurfaceSize) != mainSurfaceSize)
	{
		return false;
	}

	// TODO: Support cube textures

	// Read additional
	if(isFlagSet(header.caps.caps1, DDSCaps_MipMap))
	{
		u32 mipWidth(header.width);
		u32 mipHeight(header.height);

		// Read mipmaps
		for(u32 i = 0; i < (header.mipmapCount - 1); ++i) // mipmapCount includes level 0
		{
			mipWidth  = std::max(u32(1), mipWidth  / 2);
			mipHeight = std::max(u32(1), mipHeight / 2);
			fs::size_type mipSize = mipWidth * mipHeight * bytesPerPixel;

			// Read mipmap
			std::vector<u8>::size_type offset(pixelData.size());
			pixelData.resize(pixelData.size() + mipSize);
			if(p_file->read(&pixelData[offset], mipSize) != mipSize)
			{
				return false;
			}
		}
	}

	p_out.width    = header.width;
	p_out.height   = header.height;
	p_out.depth    = header.depth;
	p_out.pitch    = pitch;
	p_out.bitDepth = 8;
	p_out.format   = getImageFormat(header.pixelFormat);
	p_out.pixels   = static_cast<u8*>(mem::alloc(static_cast<mem::size_type>(pixelData.size())));

	mem::copy8(p_out.pixels, &pixelData[0], static_cast<mem::size_type>(pixelData.size()));

	return true;
}


//--------------------------------------------------------------------------------------------------

bool fillTextureHeaderFromDDS(const fs::FilePtr& p_file, engine::file::TextureHeader& p_header)
{
	// Read DDS header
	DDSHeader ddsHeader;
	if(readDDSHeader(p_file, ddsHeader) == false)
	{
		return false;
	}

	// Get mipmap count
	u8 mipmapCount(0);
	if(isFlagSet(ddsHeader.caps.caps1, DDSCaps_MipMap))
	{
		TT_ASSERT(ddsHeader.mipmapCount < std::numeric_limits<u8>::max());

		if(ddsHeader.mipmapCount > 1)
		{
			// Store mipmap count minus level 0 (main surface)
			mipmapCount = static_cast<u8>(ddsHeader.mipmapCount - 1);
		}
	}

	TT_ASSERT(ddsHeader.width  < std::numeric_limits<u16>::max());
	TT_ASSERT(ddsHeader.height < std::numeric_limits<u16>::max());

	// Get information
	p_header.width        = static_cast<u16>(ddsHeader.width);
	p_header.height       = static_cast<u16>(ddsHeader.height);
	p_header.depth        = static_cast<u16>(ddsHeader.depth);
	p_header.mipmapLevels = mipmapCount;
	p_header.flags        = 0;

	return true;
}


//--------------------------------------------------------------------------------------------------
// Helper functions

bool readDDSHeader(const fs::FilePtr& p_file, DDSHeader& p_header)
{
	// Check DDS signature
	u32 signature(0);
	if (fs::readInteger(p_file, &signature) == false)
	{
		return false;
	}
	
	if(signature != fourCC('D', 'D', 'S', ' '))
	{
		TT_PANIC("Data is not a valid DDS format! In file: %s", p_file->getPath());
		return false;
	}

	// Read DDS header
	u8 ddsHeader[sizeof(DDSHeader)];
	if(p_file->read(ddsHeader, sizeof(DDSHeader)) != sizeof(DDSHeader))
	{
		return false;
	}

	using namespace code::bufferutils;

	code::BufferReadContext readContext =
		code::BufferReadContext::createForRawBuffer(ddsHeader, sizeof(DDSHeader));

	p_header.structSize   = get<u32>(&readContext);
	p_header.flags        = get<u32>(&readContext);
	p_header.height       = get<u32>(&readContext);
	p_header.width        = get<u32>(&readContext);
	p_header.linearSize   = get<u32>(&readContext);
	p_header.depth        = get<u32>(&readContext);
	p_header.mipmapCount  = get<u32>(&readContext);

	// Read reserved bytes (11 * u32)
	get(reinterpret_cast<u8*>(p_header.reserved1), 11 * sizeof(u32), &readContext);

	p_header.pixelFormat.structSize   = get<u32>(&readContext);
	p_header.pixelFormat.flags        = get<u32>(&readContext);
	p_header.pixelFormat.fourCC       = get<u32>(&readContext);
	p_header.pixelFormat.RGBBitCount  = get<u32>(&readContext);
	p_header.pixelFormat.redBitMask   = get<u32>(&readContext);
	p_header.pixelFormat.greenBitMask = get<u32>(&readContext);
	p_header.pixelFormat.blueBitMask  = get<u32>(&readContext);
	p_header.pixelFormat.alphaBitMask = get<u32>(&readContext);

	p_header.caps.caps1 = get<u32>(&readContext);
	p_header.caps.caps2 = get<u32>(&readContext);
	p_header.caps.caps3 = get<u32>(&readContext);
	p_header.caps.caps4 = get<u32>(&readContext);
	p_header.reserved2  = get<u32>(&readContext);

	if (readContext.statusCode != 0)
	{
		TT_PANIC("Reading DDS Header failed, read context status code is %d", readContext.statusCode);
		return false;
	}

	// Check structure sizes
	if(p_header.structSize != DDSHeaderSize)
	{
		TT_PANIC("Invalid DDS header struct size. Header size is %d, must be %d.",
			p_header.structSize, DDSHeaderSize);
		return false;
	}
	if(p_header.pixelFormat.structSize != DDSPixelFormatSize)
	{
		TT_PANIC("Invalid DDS pixel format struct size. Header size is %d, must be %d.",
			p_header.pixelFormat.structSize, DDSPixelFormatSize);
		return false;
	}

	return true;
}


// Namespace end
}
}
