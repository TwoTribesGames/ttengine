#include <tt/code/bufferutils_get.h>
#include <tt/fileformat/cursor/CursorData.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace fileformat {
namespace cursor {

struct BitmapInfoHeader
{
	enum BitCount
	{
		BitCount_Zero       = 0,
		BitCount_Monochrome = 1,
		BitCount_4bit       = 4,
		BitCount_8bit       = 8,
		BitCount_32bit      = 32
	};
	
	enum Compression
	{
		Compression_RGB       = 0,
		Compression_RLE8      = 1,
		Compression_RLE4      = 2,
		Compression_Bitfields = 3,
		Compression_JPEG      = 4,
		Compression_PNG       = 5
	};
	
	u32         size;
	s32         width;
	s32         height;
	u16         planes;
	BitCount    bitCount;
	Compression compression;
	u32         sizeImage;
	s32         xPelsPerMeter;
	s32         yPelsPerMeter;
	u32         colorsUsed;
	u32         colorsImportant;
};

//--------------------------------------------------------------------------------------------------
// Public member functions

CursorData* CursorData::createFromData(const HeaderInfo&        p_header,
                                       code::BufferReadContext* p_imageDataContext)
{
	TT_NULL_ASSERT(p_imageDataContext);
	if (p_imageDataContext == 0) return 0;
	
	namespace bu = code::bufferutils;
	
	// Load the image data header
	BitmapInfoHeader bih;
	bih.size            = bu::get<u32>(p_imageDataContext);
	bih.width           = bu::get<s32>(p_imageDataContext);
	bih.height          = bu::get<s32>(p_imageDataContext) / 2;  // NOTE: height is combined height of XOR and AND data
	bih.planes          = bu::get<u16>(p_imageDataContext);
	bih.bitCount        = bu::getEnum<u16, BitmapInfoHeader::BitCount   >(p_imageDataContext);
	bih.compression     = bu::getEnum<u32, BitmapInfoHeader::Compression>(p_imageDataContext);
	bih.sizeImage       = bu::get<u32>(p_imageDataContext);
	bih.xPelsPerMeter   = bu::get<s32>(p_imageDataContext);
	bih.yPelsPerMeter   = bu::get<s32>(p_imageDataContext);
	bih.colorsUsed      = bu::get<u32>(p_imageDataContext);
	bih.colorsImportant = bu::get<u32>(p_imageDataContext);

	// Translate the loaded image data into RGBA data
	CursorData* cursorData = new CursorData(p_header);

	if (bih.bitCount == BitmapInfoHeader::BitCount_32bit)
	{
		s32 dataSize = (bih.width * bih.height) * (bih.bitCount / 8);
		u8* data = new u8[dataSize];
		u8* color = data;
		bu::get(data, dataSize, p_imageDataContext);

		s32 dataANDSize = (bih.width * bih.height) / 8;
		u8* dataAND = new u8[dataANDSize];
		bu::get(dataAND, dataANDSize, p_imageDataContext);

		u8* pixel = cursorData->m_imageData;
		for (s32 y = cursorData->m_height - 1; y >= 0; --y)
		{
			for (s32 x = 0; x < cursorData->m_width; ++x, pixel += 4, color += 4)
			{
				const s32 index = ((y * bih.width) + x) * 4;
				const u8* c = data +index;
				pixel[0] = c[2];
				pixel[1] = c[1];
				pixel[2] = c[0];
				pixel[3] = c[3];
				
				// Apply the transparency mask
				const s32 pixelIndex = (y * bih.width) + x;
				const s32 andIndex = pixelIndex / 8;
				const s32 andBit   = pixelIndex % 8;
				if ((dataAND[andIndex] & (1 << (7 - andBit))) != 0)
				{
					pixel[3] = 0;
				}
			}
		}

		delete [] dataAND;
		delete [] data;
	}
	else if (bih.bitCount == BitmapInfoHeader::BitCount_8bit)
	{
		// Load the color table for the image
		s32 colorsToLoad = 0;
		switch (bih.bitCount)
		{
		case BitmapInfoHeader::BitCount_Monochrome: colorsToLoad = 2;   break;
		case BitmapInfoHeader::BitCount_4bit:       colorsToLoad = 16;  break;
		case BitmapInfoHeader::BitCount_8bit:       colorsToLoad = 256; break;
		default:                                    colorsToLoad = 0;   break; // no color table for this bit depth
		}

		u8* cursorColors = new u8[colorsToLoad * 3];
		{
			u8* color = cursorColors;
			for (s32 i = 0; i < colorsToLoad; ++i, color += 3)
			{
				// NOTE: Colors are stored in BGR format in the file (but we want them as RGB)
				color[2] = bu::get<u8>(p_imageDataContext); // b
				color[1] = bu::get<u8>(p_imageDataContext); // g
				color[0] = bu::get<u8>(p_imageDataContext); // r
				const u8 reserved = bu::get<u8>(p_imageDataContext);
				TT_ASSERTMSG(reserved == 0,
							 "Reserved field of cursor color should be 0 "
							 "(but is %d for color %d in the color table).",
							 s32(reserved), i);
			}
		}

		// Load the XOR and AND image data
		// (the meaning of XOR data depends on the image format)
		s32 byteSizeXOR = 0;
		s32 byteSizeAND = (bih.width * bih.height) / 8;  // AND mask is always one bit per pixel
		switch (bih.bitCount)
		{
		case BitmapInfoHeader::BitCount_Monochrome:
			byteSizeXOR = (bih.width * bih.height) / 8;
			break;

		case BitmapInfoHeader::BitCount_4bit:
			byteSizeXOR = (bih.width * bih.height) / 2;
			break;

		case BitmapInfoHeader::BitCount_8bit:
			byteSizeXOR = (bih.width * bih.height);
			break;

		case BitmapInfoHeader::BitCount_Zero:
			byteSizeXOR = (bih.width * bih.height) * 4;
			break;

		default:
			byteSizeXOR = (bih.width * bih.height) * (bih.bitCount / 8);
			break;
		}

		u8* dataXOR = new u8[byteSizeXOR];
		u8* dataAND = new u8[byteSizeAND];
		bu::get(dataXOR, byteSizeXOR, p_imageDataContext);
		bu::get(dataAND, byteSizeAND, p_imageDataContext);

		u8* pixel = cursorData->m_imageData;
		for (s32 y = cursorData->m_height - 1; y >= 0; --y)
		{
			for (s32 x = 0; x < cursorData->m_width; ++x, pixel += 4)
			{
				// Determine the color for the pixel
				if (bih.bitCount == BitmapInfoHeader::BitCount_8bit)
				{
					const s32 xorIndex = (y * cursorData->m_width) + x;
					const u8* color = cursorColors + (dataXOR[xorIndex] * 3);
					pixel[0] = color[0];
					pixel[1] = color[1];
					pixel[2] = color[2];
				}
				else
				{
					// FIXME: Haven't done any work to parse other bit depths yet
					pixel[0] = 255;
					pixel[1] = 0;
					pixel[2] = 255;
				}

				// Apply the transparency mask
				const s32 pixelIndex = (y * bih.width) + x;
				const s32 andIndex = pixelIndex / 8;
				const s32 andBit   = pixelIndex % 8;
				if ((dataAND[andIndex] & (1 << (7 - andBit))) != 0)
				{
					pixel[3] = 0;
				}
				else
				{
					pixel[3] = 255;
				}
			}
		}

		// Clean up temporary data used for loading
		delete[] dataXOR;
		delete[] dataAND;
		delete[] cursorColors;
	}
	else
	{
		TT_ASSERTMSG(false, "The cursor loading code can't handle any other image formats than 8 and 32-bit color yet.");	
	}

	return cursorData;
}


CursorData::~CursorData()
{
	delete[] m_imageData;
}


CursorData* CursorData::clone() const
{
	return new CursorData(*this);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CursorData::CursorData(const HeaderInfo& p_header)
:
m_width    (p_header.width),
m_height   (p_header.height),
m_hotSpot  (p_header.hotSpot),
m_imageData(new u8[p_header.width * p_header.height * 4])
{
}


CursorData::CursorData(const CursorData& p_rhs)
:
m_width    (p_rhs.m_width),
m_height   (p_rhs.m_height),
m_hotSpot  (p_rhs.m_hotSpot),
m_imageData(new u8[p_rhs.m_width * p_rhs.m_height * 4])
{
	mem::copy8(m_imageData, p_rhs.m_imageData, m_width * m_height * 4);
}

// Namespace end
}
}
}
