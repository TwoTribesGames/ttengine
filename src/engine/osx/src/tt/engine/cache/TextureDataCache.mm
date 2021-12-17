#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>

#include <memory>
#include <libpng/png.h>

#include <tt/engine/cache/TextureDataCache.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>

#if !defined(TT_BUILD_FINAL)
	#include <tt/system/Time.h>
#endif


namespace tt {
namespace engine {
namespace cache {

TextureDataCache::TextureContainer TextureDataCache::ms_textures;


ConstTextureDataPtr TextureDataCache::get(const std::string& p_filename)
{
	ConstTextureDataPtr result = find(p_filename);
	if (result != 0)
	{
		return result;
	}
	return load(p_filename);
}


ConstTextureDataPtr TextureDataCache::find(const std::string& p_filename)
{
	TextureData::IDType id(tt::str::toLower(p_filename));
	TextureContainer::iterator it = ms_textures.find(id);
	if (it != ms_textures.end())
	{
		return it->second.lock();
	}
	
	// Not found, return null pointer.
	return TextureDataPtr();
}


void TextureDataCache::dump()
{
	TT_Printf("TextureDataCache::dump -- START --\n");
	for (TextureContainer::iterator it = ms_textures.begin(); it != ms_textures.end(); ++it)
	{
		TextureDataWeakPtr ptr(it->second);
		TT_Printf("TextureDataCache::dump: id: %u, name: '%s'\n"
				  "TextureData expired: %d, use cout: %u, ptr: %p\n",
				  it->first.getValue(),
#if !defined(TT_BUILD_FINAL)
				  it->first.getName().c_str(),
#else
				  "",
#endif
				  ptr.expired(), ptr.use_count(), ptr.lock().get());
	}
	TT_Printf("TextureDataCache::dump -- END --\n");
}


//--------------------------------------------------------------------------------------------------
// Private functions

ConstTextureDataPtr TextureDataCache::load(const std::string& p_filename)
{
#if defined(TT_TEXTURE_TIMER_ON) && !defined(TT_BUILD_FINAL)
	u64 loadStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif //#if defined(TT_TEXTURE_TIMER_ON) && !defined(TT_BUILD_FINAL)
	TextureData::IDType id(tt::str::toLower(p_filename));
	
	// Get filename using NSBundle.
	
	// Split p_filename in a path and filename string.
	std::string normalizedFilename(p_filename);
	str::replace(normalizedFilename, "\\", "/");
	
	std::string subPathStr;
	std::string filenameStr(normalizedFilename);
	
	std::string::size_type idx = normalizedFilename.rfind("/");
	if (idx != std::string::npos && idx + 1 < normalizedFilename.length())
	{
		subPathStr  = normalizedFilename.substr(0, idx);
		filenameStr = normalizedFilename.substr(idx + 1);
	}
	// Make NSString from std::strings.
	NSString* nsPath = [[[NSString alloc] initWithCString:subPathStr.c_str() encoding:NSASCIIStringEncoding] autorelease];
	NSString* nsName = [[[NSString alloc] initWithCString:filenameStr.c_str() encoding:NSASCIIStringEncoding] autorelease];
	
	// Use NSBundle to get the proper path. (bundle resource path, localized files, etc.)
	NSString* filename = [[NSBundle mainBundle] pathForResource: nsName ofType:@"" inDirectory: nsPath];
	TT_ASSERTMSG(filename != 0,
	             "Unable to get the path for resource name:'%s' with path: '%s' from mainBundle",
	             filenameStr.c_str(), subPathStr.c_str());
	
	
	// Preparation to load a png file.
	FILE* file = fopen([filename UTF8String], "r");
	if (file == 0) 
	{
		return TextureDataPtr();
	}
	
	char sig[8] = { 0 };
	fread(sig, 1, 8, file);
	
	if (png_check_sig((unsigned char*) sig, 8) == false) 
	{
		fclose(file);
		return TextureDataPtr();
	}
	
	png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (pngPtr == 0) // out of memory 
	{
		fclose(file);
		return TextureDataPtr();    
	}
	
	png_infop infoPtr = png_create_info_struct(pngPtr);
	if (infoPtr == 0) // out of memory  
	{
		png_destroy_read_struct(&pngPtr, (png_infopp) NULL, (png_infopp) NULL);
		fclose(file);
		return TextureDataPtr();    
	}	
	
#if defined(PNG_SETJMP_SUPPORTED)
	if (setjmp(png_jmpbuf(pngPtr))) 
	{
		png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
		fclose(file);
		return TextureDataPtr();
	}
#endif
	
	png_init_io(pngPtr, file);
	
	png_set_sig_bytes(pngPtr, 8);
	
	// Read all the info up to the image data.
	png_read_info(pngPtr, infoPtr);
	
	// Get some information about the png.
	png_uint_32 width;
	png_uint_32 height;
	int bitDepth;
	int colorType;
	png_get_IHDR(pngPtr, infoPtr, &width, &height, &bitDepth, 
				 &colorType, NULL, NULL, NULL);
	
	int bytesPerPixel = 4;
	GLint pixelFormat = GL_RGBA;
	
	if (bitDepth > 8)
	{
		TT_PANIC("Converting texture '%s' from 16 to 8 bit/channel", p_filename.c_str());
		png_set_strip_16(pngPtr);
	}
	
	if (colorType & PNG_COLOR_TYPE_RGB)
	{
		png_set_add_alpha(pngPtr, 0xFF, PNG_FILLER_AFTER);
	}
	
	if (colorType == PNG_COLOR_TYPE_GRAY)
	{
		pixelFormat = GL_LUMINANCE;
		bytesPerPixel = 1;
	}
	else if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		TT_PANIC("Converting texture '%s' type: PNG_COLOR_TYPE_GRAY_ALPHA to RGBA", p_filename.c_str());
		png_set_gray_to_rgb(pngPtr);
	}
	else if (colorType == PNG_COLOR_TYPE_PALETTE)
	{
		TT_PANIC("Converting texture '%s' type: PNG_COLOR_TYPE_PALETTE to RGBA", p_filename.c_str());
		png_set_palette_to_rgb(pngPtr);
	}
	
	png_read_update_info(pngPtr, infoPtr);
	
	// Allocate memory for the data.
	tt_ptr<TextureData>::unique rawPtr(new TextureData(id, width, height, pixelFormat));
	
	png_bytepp rowPointers = new png_bytep[height * sizeof(png_bytep)];
	
	// Set the individual row_pointers to point at the correct offsets.
	png_size_t rowbytes = png_get_rowbytes(pngPtr, infoPtr);
	for (png_size_t i = 0;  i < height;  ++i)
	{
		rowPointers[i] = rawPtr->getData() + i * (rowbytes);
	}	
	
	png_read_image(pngPtr, rowPointers);
	
	// Clean up.
	delete[] rowPointers;
	png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
	fclose(file);
	
	// Create smart pointer and add to cache.
	TextureDataPtr texture(rawPtr.release(), remove);
	
	typedef std::pair<TextureContainer::iterator, bool> insertType;
	insertType result = ms_textures.insert(std::make_pair(id, texture));
	TT_ASSERTMSG(result.second, "Already had a texture with this ID in the cache (insert failed).");
	
#if defined(TT_TEXTURE_TIMER_ON) && !defined(TT_BUILD_FINAL)
	u64 loadEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("TextureDataCache::load for texture: '%s' took: %u ms.\n",
	          p_filename.c_str(), u32(loadEnd - loadStart));
#endif //#if defined(TT_TEXTURE_TIMER_ON) && !defined(TT_BUILD_FINAL)
	
	return texture;
}


void TextureDataCache::remove(TextureData* p_texture)
{
	if (p_texture == 0)
	{
		return;
	}
	
	TextureData::IDType id(p_texture->getID());
	TextureContainer::iterator it = ms_textures.find(id);
	if (it != ms_textures.end())
	{
		ms_textures.erase(it);
		delete p_texture;
		return;
	}
	
	TT_PANIC("Couldn't find TextureData %p (ID: %u, name: '%s'.) for removal.",
			 p_texture, id.getValue(), id.getName().c_str());
}

// Namespace end
}
}
}
