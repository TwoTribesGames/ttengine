#include <tt/compression/png.h>

#include <tt/code/helpers.h>
#include <tt/code/Buffer.h>
#include <tt/fs/File.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace compression {


// PNG custom function declarations
void      __cdecl pngReadData    (png_structp p_pngPtr, png_bytep p_targetData, png_size_t p_requestedLength);
void      __cdecl pngWriteData   (png_structp p_pngPtr, png_bytep p_dataToWrite, png_size_t p_dataLength);
png_voidp __cdecl pngMalloc      (png_structp p_pngPtr, png_size_t p_size);
void      __cdecl pngFree        (png_structp p_pngPtr, png_voidp p_ptr);
void      __cdecl pngReadError   (png_structp p_pngPtr, png_const_charp p_errorMessage);
void      __cdecl pngReadWarning (png_structp p_pngPtr, png_const_charp p_errorMessage);
void      __cdecl pngWriteError  (png_structp p_pngPtr, png_const_charp p_errorMessage);
void      __cdecl pngWriteWarning(png_structp p_pngPtr, png_const_charp p_errorMessage);


// PNG Header information
struct PNGHeader
{
	PNGHeader()
	:
	width(0),
	height(0),
	bitDepth(0),
	colorType(0)
	{ }

	png_uint_32 width;
	png_uint_32 height;
	int         bitDepth;
	int         colorType;
};



//--------------------------------------------------------------------------------------------------
// PNG Decompression


bool decompressPNG(const fs::FilePtr&  p_file,
	               PixelData&          p_out,
	               u32                 p_transforms,
	               const math::Point2& p_maxDimensions)
{
	// Get file buffer information
	code::BufferPtr buffer(p_file->getContent());
	u32             offset(p_file->getPosition());

	// Get pointer to file buffer at current position
	const u8* fileStart(reinterpret_cast<const u8*>(buffer->getData()) + offset);

	// Get size of remaining buffer
	u32 fileSize(buffer->getSize() - offset);

	// Add filename for error checking
	const char* filename = "";
#if !defined(TT_BUILD_FINAL)
	filename = p_file->getPath();
#endif

	// Build source structure
	PNGSourceData source(fileStart, fileSize, filename);
	source.transforms    = p_transforms;
	source.maxDimensions = p_maxDimensions;

	// Decompress PNG data
	return decompressPNG(source, p_out);
}


bool decompressPNG(PNGSourceData& p_source, PixelData& p_out)
{
	// Check the PNG signature
	if (png_check_sig((png_bytep)p_source.data, 8) == false)
	{
		TT_PANIC("Data is not valid PNG format.");
		return false;
	}

	png_structp pngPtr = png_create_read_struct_2(
			PNG_LIBPNG_VER_STRING,     // Library version
			0,                         // User error pointer
			pngReadError,              // User error function
			pngReadWarning,            // User warning function
			0,                         // User mem pointer
			pngMalloc,                 // User malloc() function
			pngFree);                  // User free() function

	if (pngPtr == 0) // out of memory
	{
		TT_PANIC("Creating PNG read struct failed.");
		return false;
	}
	
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (infoPtr == 0) // out of memory
	{
		TT_PANIC("Creating PNG info struct failed.");
		png_destroy_read_struct(&pngPtr, 0, 0);
		return false;
	}

	// Set image limits
	if(p_source.maxDimensions != math::Point2::zero)
	{
		TT_ASSERTMSG(p_source.maxDimensions.x > 0 && p_source.maxDimensions.y > 0,
			"Invalid image limits provided for file '%s' limits set to (%d, %d).",
			p_source.filename, p_source.maxDimensions.x, p_source.maxDimensions.y);

		png_set_user_limits(pngPtr, p_source.maxDimensions.x, p_source.maxDimensions.y);
	}

#if defined(PNG_SETJMP_SUPPORTED)
	if (setjmp(png_jmpbuf(pngPtr)))
	{
		TT_PANIC("Unknown PNG loading error.");
		png_destroy_read_struct(&pngPtr, &infoPtr, 0);
		return false;
	}
#endif

	// Set up read function
	png_set_read_fn(pngPtr, &p_source, pngReadData);

	// Read all the info up to the image data.
	png_read_info(pngPtr, infoPtr);
	
	// Get some information about the png.
	PNGHeader header;
	png_get_IHDR(pngPtr, infoPtr,
	             &header.width, &header.height, &header.bitDepth, &header.colorType, 0, 0, 0);

	// FIXME: Always want RGBA output??
	modifySettingsForRGBA(pngPtr, header, p_source);
	
	png_read_update_info(pngPtr, infoPtr);

	// Allocate memory for return data
	u32 bytesPerPixel(4); // FIXME: Compute from color channels & bitdepth
	mem::size_type imageSize = static_cast<mem::size_type>(header.width * header.height * bytesPerPixel);
	u8* imageData = static_cast<u8*>(mem::alloc(imageSize));
	png_bytepp rowPointers = new png_bytep[header.height];

	// Set the individual row_pointers to point at the correct offsets
	png_size_t rowbytes = png_get_rowbytes(pngPtr, infoPtr);
	for (png_uint_32 i = 0; i < header.height; ++i)
	{
		rowPointers[i] = imageData + (i * rowbytes);
	}

	png_read_image(pngPtr, rowPointers);
	
	// Clean up
	code::helpers::safeDeleteArray(rowPointers);
	png_destroy_read_struct(&pngPtr, &infoPtr, 0);

	// Return information in output struct
	// FIXME: set correct format + bit depth after transformations
	p_out.pixels   = imageData;
	p_out.format   = ImageFormat_ARGB8;
	p_out.bitDepth = header.bitDepth;
	p_out.width    = header.width;
	p_out.height   = header.height;

	return true;
}


bool compressPNG(const fs::FilePtr&  p_outputFile,
                 const PixelData&    p_pixelData,
                 u32                 p_transforms)
{
	PNGTargetData target(p_outputFile, p_transforms);
	return compressPNG(target, p_pixelData);
}


bool compressPNG(PNGTargetData& p_target, const PixelData& p_pixelData)
{
	TT_NULL_ASSERT(p_target.outFile);
	TT_NULL_ASSERT(p_pixelData.pixels);
	if (p_target.outFile   == 0 ||
	    p_pixelData.pixels == 0)
	{
		return false;
	}
	
	// Determine the color type and transforms to use for writing the PNG
	// (do this first, so that no cleanup is needed if an unsupported format passed)
	int colorType     = PNG_COLOR_TYPE_RGB;
	int transforms    = PNG_TRANSFORM_IDENTITY;
	s32 pixelChannels = 4;  // number of channels per pixel (e.g. 4 for RGBA)
	
	static const u32 unsupportedTransformsMask = ~(u32(Transform_SwapAlphaChannel) |
	                                               u32(Transform_SwapColorChannels));
	if ((p_target.transforms & unsupportedTransformsMask) != 0)
	{
		TT_WARN("Unsupported transforms were specified. All unsupported transforms are ignored.");
	}
	
	if ((p_target.transforms & Transform_SwapAlphaChannel) != 0)
	{
		transforms |= PNG_TRANSFORM_SWAP_ALPHA;
	}
	
	if ((p_target.transforms & Transform_SwapColorChannels) != 0)
	{
		transforms |= PNG_TRANSFORM_BGR;
	}
	
	switch (p_pixelData.format)
	{
	case ImageFormat_ARGB8:
		colorType     = PNG_COLOR_TYPE_RGBA;
		transforms   ^= PNG_TRANSFORM_BGR;  // toggle the color swap bit
		pixelChannels = 4;
		break;
		
	case ImageFormat_RGBA8:
		colorType     = PNG_COLOR_TYPE_RGBA;
		pixelChannels = 4;
		break;
		
	case ImageFormat_I8:
		colorType     = PNG_COLOR_TYPE_GRAY;
		pixelChannels = 1;
		break;
		
	case ImageFormat_IA8:
		colorType     = PNG_COLOR_TYPE_GRAY_ALPHA;
		pixelChannels = 2;
		break;
		
	default:
		TT_PANIC("Unsupported pixel format: %d. Cannot write this as PNG.", p_pixelData.format);
		return false;
	}
	
	const s32 bytesPerPixel = (pixelChannels * p_pixelData.bitDepth) / 8;
	
	// Set up the various PNG structs and function pointers
	png_structp pngPtr = png_create_write_struct_2(
			PNG_LIBPNG_VER_STRING,     // Library version
			0,                         // User error pointer
			pngWriteError,             // User error function
			pngWriteWarning,           // User warning function
			0,                         // User mem pointer
			pngMalloc,                 // User malloc() function
			pngFree);                  // User free() function
	
	if (pngPtr == 0) // out of memory
	{
		TT_PANIC("Creating PNG write struct failed.");
		return false;
	}
	
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (infoPtr == 0) // out of memory
	{
		TT_PANIC("Creating PNG info struct failed.");
		png_destroy_write_struct(&pngPtr, 0);
		return false;
	}
	
#if defined(PNG_SETJMP_SUPPORTED)
	if (setjmp(png_jmpbuf(pngPtr)))
	{
		TT_PANIC("Unknown PNG writing error.");
		png_destroy_write_struct(&pngPtr, &infoPtr);
		return false;
	}
#endif
	
	png_set_write_fn(pngPtr, &p_target, pngWriteData, 0);
	
	// Set the image header
	png_set_IHDR(pngPtr,
	             infoPtr,
	             static_cast<png_uint_32>(p_pixelData.width),
	             static_cast<png_uint_32>(p_pixelData.height),
	             static_cast<int        >(p_pixelData.bitDepth),
	             colorType,
	             PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT,
	             PNG_FILTER_TYPE_DEFAULT);
	
	// Tell libpng where to find the image data
	png_bytepp rowPointers = new png_bytep[p_pixelData.height];
	for (s32 y = 0; y < p_pixelData.height; ++y)
	{
		rowPointers[y] = p_pixelData.pixels + (y * p_pixelData.width * bytesPerPixel);
	}
	
	// Write the image data
	png_set_rows (pngPtr, infoPtr, rowPointers);
	png_write_png(pngPtr, infoPtr, transforms, 0);
	
	// Clean up
	code::helpers::safeDeleteArray(rowPointers);
	png_destroy_write_struct(&pngPtr, &infoPtr);
	
	return true;
}


void modifySettingsForRGBA(png_structp p_png, const PNGHeader& p_header, const PNGSourceData& p_src)
{
	if (p_header.bitDepth > 8)
	{
		TT_PANIC("Converting image '%s' from 16 to 8 bit/channel", p_src.filename);
		png_set_strip_16(p_png);
	}

	// Get information from color type
	bool hasAlphaChannel((p_header.colorType & PNG_COLOR_MASK_ALPHA)   != 0);
	bool hasColor       ((p_header.colorType & PNG_COLOR_MASK_COLOR)   != 0);
	bool hasPalette     ((p_header.colorType & PNG_COLOR_MASK_PALETTE) != 0);
	
	// Add alpha channel if needed
	if ((p_src.transforms & Transform_AddAlphaChannel) != 0 && hasAlphaChannel == false)
	{
		png_set_add_alpha(p_png, 0xFF, PNG_FILLER_AFTER);
	}

	// Remove alpha channel if needed
	if ((p_src.transforms & Transform_StripAlphaChannel) != 0 && hasAlphaChannel)
	{
		png_set_strip_alpha(p_png);
	}

	// Swap alpha channel to get ARGB
	if ((p_src.transforms & Transform_SwapAlphaChannel) != 0 && hasAlphaChannel)
	{
		png_set_swap_alpha(p_png);
	}

	// Swap color channels to get BGR
	if ((p_src.transforms & Transform_SwapColorChannels) != 0 && hasColor)
	{
		png_set_bgr(p_png);
	}
	
	// Convert greyscale => RGB
	if ((p_src.transforms & Transform_ConvertToRGB) != 0 && hasColor == false)
	{
		png_set_gray_to_rgb(p_png);
	}

	// Convert RGB => greyscale
	if ((p_src.transforms & Transform_ConvertToLuminance) != 0 && hasColor)
	{
		static const int doSilentConversion = 1; // No error if source data is not grey

		png_set_rgb_to_gray(p_png, doSilentConversion, -1, -1);
	}

	// Convert palletted to RGB
	if (hasPalette)
	{
		TT_WARN("Converting image '%s' from PNG_COLOR_TYPE_PALETTE to RGBA", p_src.filename);
		png_set_palette_to_rgb(p_png);
	}
}


//--------------------------------------------------------------------------------------------------
// PNG Custom function implementations

void __cdecl pngReadData(png_structp p_pngPtr, png_bytep p_targetData, png_size_t p_requestedLength)
{
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr == 0)
	{
		png_error(p_pngPtr, "Invalid PNG I/O pointer.");
		return;
	}
	
	PNGSourceData* sourceData = reinterpret_cast<PNGSourceData*>(ioPtr);
	
	s32 remainingBytes = sourceData->length - sourceData->currentReadByteIndex;
	if (p_requestedLength > static_cast<png_size_t>(remainingBytes))
	{
		png_error(p_pngPtr, "Not enough data remaining in memory buffer.");
		return;
	}
	
	tt::mem::copy8(p_targetData, sourceData->data + sourceData->currentReadByteIndex,
	               static_cast<tt::mem::size_type>(p_requestedLength));
	sourceData->currentReadByteIndex += static_cast<s32>(p_requestedLength);
}


void __cdecl pngWriteData(png_structp p_pngPtr, png_bytep p_dataToWrite, png_size_t p_dataLength)
{
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr == 0)
	{
		png_error(p_pngPtr, "Invalid PNG I/O pointer.");
		return;
	}
	
	PNGTargetData* targetData = reinterpret_cast<PNGTargetData*>(ioPtr);
	
	const fs::size_type bytesToWrite = static_cast<fs::size_type>(p_dataLength);
	if (targetData->outFile->write(p_dataToWrite, bytesToWrite) != bytesToWrite)
	{
		png_error(p_pngPtr, "Could not write data to file.");
	}
}


png_voidp __cdecl pngMalloc(png_structp p_pngPtr, png_size_t p_size)
{
	if (p_pngPtr == 0 || p_size == 0)
	{
		return 0;
	}
	return static_cast<png_voidp>(mem::alloc(static_cast<mem::size_type>(p_size)));
}


void __cdecl pngFree(png_structp p_pngPtr, png_voidp p_ptr)
{
	if (p_pngPtr != 0 && p_ptr != 0)
	{
		mem::free(static_cast<void*>(p_ptr));
	}
}


void __cdecl pngReadError(png_structp p_pngPtr, png_const_charp p_errorMessage)
{
#if !defined(TT_BUILD_FINAL)
	const char* filename = "";
	
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr != 0)
	{
		PNGSourceData* sourceData = reinterpret_cast<PNGSourceData*>(ioPtr);
		filename = sourceData->filename;
	}
	
	TT_PANIC("PNG READ ERROR:\n%s\nFilename: %s", p_errorMessage, filename);
#endif
	(void)p_pngPtr;
	(void)p_errorMessage;
}


void __cdecl pngReadWarning(png_structp p_pngPtr, png_const_charp p_errorMessage)
{
#if !defined(TT_BUILD_FINAL)
	const char* filename = "";
	
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr != 0)
	{
		PNGSourceData* sourceData = reinterpret_cast<PNGSourceData*>(ioPtr);
		filename = sourceData->filename;
	}
	
	TT_PANIC("PNG READ WARNING:\n%s\nFilename: %s", p_errorMessage, filename);
#endif
	(void)p_pngPtr;
	(void)p_errorMessage;
}


void __cdecl pngWriteError(png_structp p_pngPtr, png_const_charp p_errorMessage)
{
#if !defined(TT_BUILD_FINAL)
	const char* filename = "";
	
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr != 0)
	{
		PNGTargetData* targetData = reinterpret_cast<PNGTargetData*>(ioPtr);
		filename = targetData->outFile->getPath();
	}
	
	TT_PANIC("PNG WRITE ERROR:\n%s\nFilename: %s", p_errorMessage, filename);
#endif
	(void)p_pngPtr;
	(void)p_errorMessage;
}


void __cdecl pngWriteWarning(png_structp p_pngPtr, png_const_charp p_errorMessage)
{
#if !defined(TT_BUILD_FINAL)
	const char* filename = "";
	
	void* ioPtr = png_get_io_ptr(p_pngPtr);
	if (ioPtr != 0)
	{
		PNGTargetData* targetData = reinterpret_cast<PNGTargetData*>(ioPtr);
		filename = targetData->outFile->getPath();
	}
	
	TT_PANIC("PNG WRITE WARNING:\n%s\nFilename: %s", p_errorMessage, filename);
#endif
	(void)p_pngPtr;
	(void)p_errorMessage;
}

// Namespace end
}
}
