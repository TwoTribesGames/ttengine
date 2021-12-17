#import <QuartzCore/QuartzCore.h>

#include <tt/compression/pvrtc.h>

#include <tt/platform/tt_error.h>

namespace tt {
namespace compression {
	
// Struct used for the PVRTC textures. This struct should not be changed.
struct CompressedTextureHeader
{
	u32 headerSize;
	u32 height;
	u32 width;
	u32 mipmapLevels;
	u32 pixelFormatFlags;
	u32 surfaceDataSize;
	u32 bitsPerPixel;
	u32 redMask;
	u32 greenMask;
	u32 blueMask;
	u32 alphaMask;
	u32 PVRId;
	u32 numberOfSurfaces;
};
static const u32 pvrtcHeaderSize = sizeof(CompressedTextureHeader);
		
		
bool decompressPVRTC(const std::string& p_filename, PixelData& p_out)
{

	(void)p_out;
	
	NSString* fileName = [[NSBundle mainBundle] bundlePath];
	fileName = [fileName stringByAppendingString:@"/"];
	fileName = [fileName stringByAppendingString:[NSString stringWithUTF8String:p_filename.c_str()]];
	NSString* extension = [[fileName pathExtension] lowercaseString];
	
	NSData* compressedData = nil;
	
	if ([extension isEqualToString:@"png"])
	{
		NSString* compressedFileName = [fileName stringByReplacingOccurrencesOfString:@"png" withString:@"pvr" 
										options:NSCaseInsensitiveSearch range:NSMakeRange([fileName length] - 5, 5)];
		compressedData = [NSData dataWithContentsOfFile:compressedFileName];
	}
	// Check to see if it's a compressed texture without header.
	else if ([extension isEqualToString:@"pvrtc"])
	{
		// These are compressed textures without any information.
		return false;
	}
	else if ([extension isEqualToString:@"pvr"])
	{
		// pvrtc are compressed textures. They are stored on the disk as raw data which we can provide to openGL.
		compressedData = [NSData dataWithContentsOfFile:fileName];
		if (compressedData == nil)
		{
			return false;
		}
	}
	
		
	if (compressedData != nil)
	{
		// Image needs at least a header and two pixels
		if ([compressedData length] < (pvrtcHeaderSize + 1))
		{
			TT_PANIC("Compressed texture is too small to be valid.");
			return false;
		}
		
		// Read the header data for information about the image
		NSData* headerData = [NSData dataWithBytes:[compressedData bytes] length:pvrtcHeaderSize];
		const CompressedTextureHeader* headerStruct = 
			reinterpret_cast<const CompressedTextureHeader*>([headerData bytes]);
		
		if (headerStruct->width != headerStruct->height)
		{
			TT_PANIC("Compressed textures must be square (width = height).");
			return false;
		}
		
		p_out.width  = headerStruct->width;
		p_out.height = headerStruct->height;
		
		if (headerStruct->bitsPerPixel == 2)
		{
			if (headerStruct->pixelFormatFlags & 0x8000)
			{
				// Image has 2 bits per pixel and alpha.
				p_out.format = ImageFormat_PVRTC_2BPP_RGBA;
			}
			else
			{
				// Image has 2 bits per pixel but no alpha.
				p_out.format = ImageFormat_PVRTC_2BPP_RGB;
			}
		}
		else if (headerStruct->bitsPerPixel == 4)
		{
			if (headerStruct->pixelFormatFlags & 0x8000)
			{
				// Image has 4 bits per pixel and alpha.
				p_out.format = ImageFormat_PVRTC_4BPP_RGBA;
			}
			else
			{
				// Image has 4 bits per pixel but no alpha.
				p_out.format = ImageFormat_PVRTC_4BPP_RGB;
			}
		}
		else
		{
			return false;
		}
	
		// Get a pointer to the imagedata
		// FIXME: Copy data to pixels
		//p_out.pixels = mem::alloc();
		//p_out.pixels = reinterpret_cast<u8*>([compressedData bytes]) + headerStruct->headerSize;
		
		TT_PANIC("Not implemented.");

		return false;
	}
	
	// ****** Not a compressed texture ******
	
	return false;
}
		
		
		
// Namespace end
}
}
