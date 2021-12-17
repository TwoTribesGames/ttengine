#include <limits>

#include <tt/engine/renderer/TextureBase.h>

#include <tt/compression/dds.h>
#include <tt/compression/fastlz.h>
#include <tt/compression/lz4/lz4.h>
#include <tt/compression/image.h>
#include <tt/compression/png.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/TextureHardware.h>
#include <tt/fs/File.h>
#include <tt/mem/mem.h>
#include <tt/platform/tt_printf.h>


namespace tt {
namespace engine {
namespace renderer {


bool TextureBase::ms_enforceHardwareDimensions = true;
bool TextureBase::ms_deviceSupportsNonPow2     = true;
bool TextureBase::ms_forcedPow2Only            = false;
bool TextureBase::ms_forceMipmapLodBias        = false;


TextureBase::TextureBase(const EngineID& p_id, u32 p_flags)
:
m_info(),
m_state(),
m_hwState(),
m_pixels(0),
m_compressedPixels(0),
m_imageSize(0),
m_compressedSize(0),
m_id(p_id)
{
	// Init texture state
	setAddressMode(AddressMode_Clamp, AddressMode_Clamp, AddressMode_Clamp);
	setFilterMode (FilterMode_Linear, FilterMode_Linear, FilterMode_None);

	m_info.paintable = (p_flags & tt::engine::file::TextureFlag_Paintable) != 0;
}


TextureBase::TextureBase(const TextureBaseInfo& p_info)
:
m_info(p_info),
m_state(),
m_hwState(),
m_pixels(0),
m_compressedPixels(0),
m_imageSize(0),
m_compressedSize(0),
m_id(0,0)
{
	// Init texture state
	setAddressMode(AddressMode_Clamp, AddressMode_Clamp, AddressMode_Clamp);
	setFilterMode (FilterMode_Linear, FilterMode_Linear, FilterMode_None);
	
	if(m_info.usage == Usage_MipVisualization)
	{
		createMipVisualizationTexture();
	}
	else if (m_info.usage == Usage_Text)
	{
		m_info.paintable = true;
	}
}


TextureBase::~TextureBase()
{
	mem::free(m_pixels);
}


bool TextureBase::loadPixelData(const fs::FilePtr& p_file, u32 p_pngTransforms, bool p_decompress)
{
	// Read texture header
	file::TextureHeader header;

	if(p_file->read(&header, sizeof(header)) != sizeof(header))
	{
		TT_PANIC("[ENGINE] Failed to read texture header from file '%s'", p_file->getPath());
		return false;
	}

	// Fill texture info
	m_info.width     = header.width;
	m_info.height    = header.height;
	m_info.depth     = header.depth;
	m_info.mipLevels = header.mipmapLevels;
	m_info.format    = static_cast<ImageFormat>(header.pixelFormat);
	m_info.premultiplied = (header.flags & file::TextureFlag_Premultiplied) != 0;
	m_info.type      = header.depth > 1 ? Type_VolumeTexture : Type_Texture;

	// Enable filtering for mipmaps
	if(m_info.mipLevels > 0)
	{
		setMipmapFilter(FilterMode_Linear);
	}

	// Check hardware limits
	std::string textureName;
	std::string fileName;
#if !defined(TT_BUILD_FINAL)
	textureName = m_id.getName();
	fileName    = p_file->getPath();
#endif
	const TextureHardware::Requirements& textureRequirements(TextureHardware::getRequirements());
	if(textureRequirements.checkDimension(header.width,  true, textureName, fileName) == false ||
	   textureRequirements.checkDimension(header.height, true, textureName, fileName) == false)
	{
		return false;
	}

	bool useImageDataPixels(true);

	// Read image data
	compression::PixelData imageData;

	switch(header.compression)
	{
		case file::CompressionType_PNG:
		{
			// Return color data as BGRA
			const u32 pngTransforms = p_pngTransforms | compression::Transform_AddAlphaChannel;

			if(compression::decompressPNG(p_file, imageData, pngTransforms) == false)
			{
				return false;
			}
			break;
		}

		case file::CompressionType_DDS:
		{
			if(compression::decompressDDS(p_file, imageData) == false)
			{
				return false;
			}
			break;
		}

		case file::CompressionType_ARGB:
		{
			imageData.bitDepth = 8;
			imageData.depth    = header.depth;
			imageData.width    = header.width;
			imageData.height   = header.height;
			imageData.format   = static_cast<tt::ImageFormat>(header.pixelFormat);

			u32 imageSize(0);
			if(p_file->read(&imageSize, sizeof(imageSize)) != sizeof(imageSize))
			{
				return false;
			}
			
			imageData.pixels = static_cast<u8*>(mem::alloc(imageSize, 8));
			if(p_file->read(imageData.pixels, imageSize) != static_cast<fs::size_type>(imageSize))
			{
				return false;
			}

			break;
		}

		case file::CompressionType_FastLZ:
		case file::CompressionType_LZ4:
		case file::CompressionType_LZ4HC:
		{
			imageData.bitDepth = 8;
			imageData.depth    = header.depth;
			imageData.width    = header.width;
			imageData.height   = header.height;
			imageData.format   = static_cast<tt::ImageFormat>(header.pixelFormat);
			imageData.pixels   = 0;

			if(p_file->read(&m_imageSize, sizeof(m_imageSize)) != sizeof(m_imageSize))
			{
				return false;
			}

			if(p_file->read(&m_compressedSize, sizeof(m_compressedSize)) != sizeof(m_compressedSize))
			{
				return false;
			}
			
			m_compressedPixels = static_cast<u8*>(mem::alloc(m_compressedSize));
			TT_NULL_ASSERT(m_compressedPixels);
			
			if(p_file->read(m_compressedPixels, m_compressedSize) != static_cast<fs::size_type>(m_compressedSize))
			{
				return false;
			}
			
			if(p_decompress)
			{
				decompressPixelData(static_cast<file::CompressionType>(header.compression));
				useImageDataPixels = false;
			}

			break;
		}

		default:
			TT_PANIC("Unsupported texture compression (%d)", header.compression);
	}

	// Check read image
	TT_ASSERT(imageData.bitDepth == 8);
	TT_ASSERT(imageData.format == ImageFormat_ARGB8 ||
	          imageData.format == ImageFormat_RGBA8 ||
	          imageData.format == ImageFormat_A8    ||
	          imageData.format == ImageFormat_I8);
	TT_ASSERTMSG(imageData.width == header.width,
		"Image width (%d) does not match header information (%d) for texture (%s)",
		imageData.width, header.width, p_file->getPath());

	// PNG will return a higher image if mipmapped
	TT_ASSERTMSG((header.mipmapLevels == 0 && imageData.height == header.height) ||
	             (header.mipmapLevels >  0 && imageData.height >= header.height),
		"Image height (%d) does not match header information (%d) for texture (%s)",
		imageData.height, header.height, p_file->getPath());

	m_info.format = imageData.format;

	//TT_Printf("Loaded Image of %u x %u pixels with %u mipmaps.\n", m_info.width, m_info.height, m_info.mipLevels);

	// Free previously allocated data
	if(useImageDataPixels)
	{
		mem::free(m_pixels);

		// Keep pixel data in memory until after HW texture creation
		m_pixels = imageData.pixels;
	}

	return true;
}


bool TextureBase::loadFromPNG(const fs::FilePtr& p_file, u32 p_pngTransforms)
{
	TT_NULL_ASSERT(p_file);
	if (p_file == 0)
	{
		return false;
	}
	
	compression::PixelData imageData;
	const u32 pngTransforms = compression::Transform_AddAlphaChannel | p_pngTransforms;
	
	if(compression::decompressPNG(p_file, imageData, pngTransforms) == false)
	{
		return false;
	}
	
	m_info.format    = imageData.format;
	m_info.width     = static_cast<u16>(imageData.width );
	m_info.height    = static_cast<u16>(imageData.height);
	m_info.mipLevels = 0;
	m_info.premultiplied = false;
	
	mem::free(m_pixels);
	
	// Keep pixel data in memory until after HW texture creation
	m_pixels = imageData.pixels;
	
	return true;
}


bool TextureBase::loadPlatformData(const fs::FilePtr&)
{
	TT_PANIC("This function should be implemented by platform specific class");
	return false;
}


void TextureBase::decompressPixelData(file::CompressionType p_type)
{
	TT_ASSERTMSG(m_compressedPixels != 0, "Pixel data is not compressed");

	if(m_compressedPixels != 0)
	{
		mem::free(m_pixels);
		m_pixels = static_cast<u8*>(mem::alloc(m_imageSize, 8));
		if (m_pixels != 0)
		{
			switch (p_type)
			{
			case file::CompressionType_FastLZ:
			{
				u32 decompressedSize = fastlz_decompress(m_compressedPixels, m_compressedSize, m_pixels, m_imageSize);
				TT_ASSERT(decompressedSize == m_imageSize);
				break;
			}
				
			case file::CompressionType_LZ4:
			case file::CompressionType_LZ4HC:
			{
				u32 bytesProcessed = LZ4_decompress_fast(reinterpret_cast<const char*>(m_compressedPixels),
					reinterpret_cast<char*>(m_pixels), m_imageSize);
				TT_ASSERT(bytesProcessed == m_compressedSize);
				break;
			}
			default:
				TT_PANIC("Unhandled compression type '%d'", p_type);
				break;
			}
		}
		
		mem::free(m_compressedPixels);
		m_compressedPixels = 0;
	}
}


void TextureBase::createMipVisualizationTexture()
{
	m_info.usage     = Usage_MipVisualization;
	m_info.format    = ImageFormat_ARGB8;
	m_info.width     = 32;
	m_info.height    = 32;
	m_info.mipLevels = 5;
	m_info.paintable = true;
	setMipmapFilter(FilterMode_Linear);

	// Allocate enough for all mip levels
	mem::free(m_pixels);
	m_pixels = static_cast<u8*>(mem::alloc(4096+1024+256+64+16+4, 4));

	ColorRGBA* pixel = reinterpret_cast<ColorRGBA*>(m_pixels);

	ColorRGBA mipColors[] = {
#if TT_PLATFORM_WIN
		// Swap R-B on Windows
		ColorRGBA(255,0,0,220),
		ColorRGBA(255,128,0,100),
		ColorRGBA(0,255,0,70),
		ColorRGBA(0,178,255,100),
		ColorRGBA(0,76,255,150),
		ColorRGBA(0,0,255,220)
#else
		ColorRGBA(0,0,255,204),
		ColorRGBA(0,128,255,102),
		ColorRGBA(255,255,255,0),
		ColorRGBA(255,178,0,51),
		ColorRGBA(255,76,0,153),
		ColorRGBA(255,0,0,204)
#endif
	};

	u32 mipSize = 32;
	for (u32 mip = 0; mip <= m_info.mipLevels; ++mip)
	{
		const u32 numPixels = mipSize * mipSize;
		for (u32 i = 0; i < numPixels; ++i)
		{
			*pixel = mipColors[mip];
			++pixel;
		}
		mipSize = mipSize / 2;
	}
}


void TextureBase::setAddressMode(AddressMode p_u, AddressMode p_v, AddressMode p_w)
{
	setAddressModeU(p_u);
	setAddressModeV(p_v);
	setAddressModeW(p_w);
}


void TextureBase::setAddressModeU(AddressMode p_mode)
{
	m_state.addressMode &= ~ADDRESS_U_MASK;
	m_state.addressMode |= p_mode;
}


void TextureBase::setAddressModeV(AddressMode p_mode)
{
	m_state.addressMode &= ~ADDRESS_V_MASK;
	m_state.addressMode |= (p_mode << ADDRESS_V_SHIFT);
}


void TextureBase::setAddressModeW(AddressMode p_mode)
{
	m_state.addressMode &= ~ADDRESS_W_MASK;
	m_state.addressMode |= (p_mode << ADDRESS_W_SHIFT);
}


void TextureBase::setFilterMode(FilterMode p_minFilter, FilterMode p_magFilter, FilterMode p_mipFilter)
{
	setMinificationFilter (p_minFilter);
	setMagnificationFilter(p_magFilter);
	setMipmapFilter       (p_mipFilter);
}


void TextureBase::setMinificationFilter(FilterMode p_filter)
{
	TT_ASSERT(isValidFilterMode(p_filter));
	if(p_filter == FilterMode_None)
	{
		TT_PANIC("Invalid filtermode 'FilterMode_None' for minification.");
		p_filter = FilterMode_Point;
	}

	m_state.filterMode &= ~FILTER_MIN_MASK;
	m_state.filterMode |= p_filter;
}


void TextureBase::setMagnificationFilter(FilterMode p_filter)
{
	TT_ASSERT(isValidFilterMode(p_filter));
	if(p_filter == FilterMode_None)
	{
		TT_PANIC("Invalid filtermode 'FilterMode_None' for magnification.");
		p_filter = FilterMode_Point;
	}

	m_state.filterMode &= ~FILTER_MAG_MASK;
	m_state.filterMode |= (p_filter << FILTER_MAG_SHIFT);
}


void TextureBase::setMipmapFilter(FilterMode p_filter)
{
	TT_ASSERT(isValidFilterMode(p_filter));
	if(p_filter > FilterMode_Linear)
	{
		TT_PANIC("Invalid filtermode '%s' for minification.", getFilterModeName(p_filter));
		p_filter = FilterMode_None;
	}

	m_state.filterMode &= ~FILTER_MIP_MASK;
	m_state.filterMode |= (p_filter << FILTER_MIP_SHIFT);
}


u32 TextureBase::getBytesPerPixel() const
{
	switch(m_info.format)
	{
	case ImageFormat_A8:
	case ImageFormat_I8:
	case ImageFormat_BC1: // FIXME: This is incorrect, should be 0.5
	case ImageFormat_BC2:
	case ImageFormat_BC3:
		return 1;

	case ImageFormat_RGBA8:
	case ImageFormat_ARGB8:
		return 4;

	default:
		TT_PANIC("Unsupported image format (%d)", m_info.format);
		return 4;
	}
}


s32 TextureBase::getMemSize() const
{
	s32 mipSize = m_info.width * m_info.height * getBytesPerPixel();
	s32 totalSize = mipSize; // mip level 0

	// Add size of mipmaps
	for(s32 i = 0; i < m_info.mipLevels; ++i)
	{
		mipSize = mipSize >= 4 ? mipSize / 4 : 1;
		totalSize += mipSize;
	}

	return totalSize;
}


math::Point2 TextureBase::getMinimalDimensions(const math::Point2& p_dimensions)
{
	const TextureHardware::Requirements& textureRequirements(TextureHardware::getRequirements());

	if(textureRequirements.fitsDimensions(p_dimensions))
	{
		return textureRequirements.correctDimension(p_dimensions);
	}

	// Texture is too big

	return textureRequirements.correctDimension(math::Point2(
		std::min(textureRequirements.getMaxDimension(), p_dimensions.x),
		std::min(textureRequirements.getMaxDimension(), p_dimensions.y)));
}


void TextureBase::checkDimensions(const std::string& p_info)
{
	// Dimension Check (This sort of platform constraint should be enforced in Asset Tool)
	(void)p_info;

#if !defined(TT_BUILD_FINAL)
	
	if (ms_enforceHardwareDimensions == false)
	{
		// Not allowed to complain
		return;
	}
	
	// Create information about the texture
	const std::string fileInfo("asset: '"     + m_id.getName()      + "' "
	                           "namespace: '" + m_id.getNamespace() + "' " + p_info);

	TextureHardware::getRequirements().checkDimension(m_info.width,  true, "width",  fileInfo);
	TextureHardware::getRequirements().checkDimension(m_info.height, true, "height", fileInfo);

#endif // #if !defined(TT_BUILD_FINAL)
}


// Namespace end
}
}
}
