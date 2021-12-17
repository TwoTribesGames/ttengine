#include <tt/engine/cache/FileTextureCache.h>
#include <tt/engine/debug/DebugRenderer.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/Texture.h>

#include <tt/platform/tt_printf.h>
#include <tt/str/str.h>
#include <tt/fs/File.h>
#include <tt/fs/utils/utils.h>
#include <tt/system/Language.h>
#include <tt/system/Time.h>

namespace tt {
namespace engine {
namespace cache {


FileTextureCache::TextureContainer FileTextureCache::ms_textures;


renderer::TexturePtr FileTextureCache::get(const std::string& p_filename, fs::identifier p_fileSystem)
{
	renderer::TexturePtr result = find(p_filename, p_fileSystem);
	if (result != 0)
	{
		return result;
	}
	return load(p_filename, p_fileSystem);
}


renderer::TexturePtr FileTextureCache::find(const std::string& p_filename, fs::identifier p_fileSystem)
{
	EngineID id(str::toStr(p_fileSystem) + "|" + p_filename, "dummy");
	TextureContainer::iterator it = ms_textures.find(id);
	if (it != ms_textures.end())
	{
		return it->second.lock();
	}
	
	// Not found, return null pointer.
	return renderer::TexturePtr();
}


void FileTextureCache::dump()
{
	TT_Printf("FileTextureCache::dump -- START --\n");
	
	u32 totalSize = 0;
	u32 totalTextures = 0;
	
	typedef std::multimap<s32, std::string> SizeNameMap;
	SizeNameMap sizeNameMap;
	
	for (TextureContainer::iterator it = ms_textures.begin();
	     it != ms_textures.end(); ++it)
	{
		tt::engine::renderer::TextureWeakPtr ptr(it->second);
		
		tt::engine::renderer::TexturePtr texPtr(it->second.lock());
		++totalTextures;
		totalSize += texPtr->getMemSize();
		
		sizeNameMap.insert(std::make_pair(texPtr->getMemSize(), 
#if !defined(TT_BUILD_FINAL)
		                   it->first.getName()
#else
		          ""
#endif
		          ));
		
		TT_Printf("FileTextureCache::dump: size: %10d KB, id crc1: %u, crc2: %u, name: '%s'\n"
		          "Texture expired: %s, use count: %u, ptr: %p\n",
		          (texPtr->getMemSize() / 1024),
		          it->first.crc1, it->first.crc2,
#if !defined(TT_BUILD_FINAL)
		          it->first.getName().c_str(),
#else
		          "",
#endif
		          ptr.expired() ? "yes" : "no ", ptr.use_count(), ptr.lock().get());
	}
	
	TT_Printf("FileTextureCache::dump -- File sorted Textures: --\n");
	
	
	for (SizeNameMap::iterator it = sizeNameMap.begin(); it != sizeNameMap.end(); ++it)
	{
		TT_Printf("FileTextureCache::dump: size: %10d KB, name: '%s'\n",
		          (it->first / 1024), it->second.c_str() );
	}
	
	
	TT_Printf("\nTotal textures: %d\n", totalTextures);
	TT_Printf("Total texture memory in use: %.3f MB\n", (totalSize/(1024.0f*1024.0f)));
	TT_Printf("FileTextureCache::dump -- END --\n");
}


void FileTextureCache::dumpToFile(const std::string& p_filename, bool p_append)
{
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	if (ms_textures.empty())
	{
		// no data; early abort
		return;
	}
	
	tt::fs::FilePtr f;
	
	if (p_append)
	{
		f = tt::fs::open(p_filename, tt::fs::OpenMode_Append);
	}
	else
	{
		f = tt::fs::open(p_filename, tt::fs::OpenMode_Write);
	}
	
	if (f == 0)
	{
		TT_PANIC("Cannot open log file '%s'", p_filename.c_str());
		return;
	}
	
	char buf[1024];
	
	sprintf(buf, "-- FILETEXTURECACHE DUMP START (%s) --\r\n", 
		system::Time::getInstance()->getNowAsString().c_str());
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	u32 totalSize = 0;
	u32 totalResources = 0;
	
	typedef std::multimap<int, std::string> SizeMap;
	SizeMap sortedMap;
	
	for (TextureContainer::iterator it = ms_textures.begin();
	     it != ms_textures.end(); ++it)
	{
		tt::engine::renderer::TexturePtr ptr(it->second.lock());
		
		++totalResources;
		totalSize += ptr->getMemSize();
		sortedMap.insert(std::make_pair((ptr->getMemSize() / 1024), it->first.getName()));
	}
	
	// output statistics
	sprintf(buf, "\r\nTotal resources: %d\r\n", totalResources);
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	
	sprintf(buf, "Total resource memory in use: %.3f MB\r\n\r\n", (totalSize/(1024.0f*1024.0f)));
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	
	for (SizeMap::const_reverse_iterator it = sortedMap.rbegin(); it != sortedMap.rend(); ++it)
	{
		sprintf(buf, "\tsize: %6d KB\tname: '%s'\r\n", (*it).first, (*it).second.c_str());
	
		f->write(buf, static_cast<fs::size_type>(strlen(buf)));
	}
	
	sprintf(buf, "\r\n-- FILETEXTURECACHE DUMP END --\r\n\r\n");
	f->write(buf, static_cast<fs::size_type>(strlen(buf)));
#else // #if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	(void)p_filename;
	(void)p_append;
#endif // #else //#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
}


s32 FileTextureCache::getTotalMemory()
{
	s32 size = 0;
	for (TextureContainer::iterator it = ms_textures.begin(); it != ms_textures.end(); ++it)
	{
		tt::engine::renderer::TexturePtr ptr(it->second.lock());
		if (ptr != 0)
		{
			size += ptr->getMemSize();
		}
	}
	
	return size;
}


//--------------------------------------------------------------------------
// Private functions

renderer::TexturePtr FileTextureCache::load(const std::string& p_filename, fs::identifier p_fileSystem)
{
#if !defined(TT_BUILD_FINAL)
	{
		// verify whether data has correct path
		std::string file(tt::fs::utils::compactPath(p_filename, "/\\", p_fileSystem));
		tt::str::replace(file, "\\", "/");
		TT_ASSERTMSG(p_filename == file, "Path mismatch: '%s' should be '%s'", 
			p_filename.c_str(), file.c_str());
	}
#endif
	
	std::string filename(p_filename);
	if (tt::fs::fileExists(filename, p_fileSystem) == false)
	{
		// not found, try to load language specific texture
		static const std::string langRoot("lang/");
		
		filename = langRoot + tt::system::Language::getLanguage() + "/" + p_filename;
		if (fs::fileExists(filename, p_fileSystem) == false)
		{
			// fall back on 'en'. (FIXME: Remove hardcoded fall back languages
			filename = langRoot + "en/" + p_filename;
			
			if (fs::fileExists(filename, p_fileSystem) == false)
			{
				TT_WARN("File does not exist: '%s', also checked language fallback paths", p_filename.c_str());
				return renderer::Renderer::getInstance()->getDebug()->getDummyTexture();
			}
		}
	}
	
	tt::engine::EngineID id(str::toStr(p_fileSystem) + "|" + filename, "dummy");
	
	// Load the specified texture
	tt_ptr<renderer::Texture>::unique rawPtr(new renderer::Texture(id));
	
	if (rawPtr->loadFromFile(filename, p_fileSystem) == false)
	{
		TT_WARN("Failed to load texture: '%s'\n", filename.c_str());
		
		// Failed to load.
		// Return default texture
		return renderer::Renderer::getInstance()->getDebug()->getDummyTexture();
	}
	
	// Create smart pointer and add to cache.
	renderer::TexturePtr texture(rawPtr.release(), remove);
	
	typedef std::pair<TextureContainer::iterator, bool> insertType;
	insertType result = ms_textures.insert(std::make_pair(id, texture));
	(void)result;
	
#if !defined(TT_BUILD_FINAL)
	TT_ASSERTMSG(result.second, "Already had a texture with this id '%s' in the cache. (Insert failed).",
		id.getName().c_str());
#endif
	
	// Return the texture
	return texture;
}


void FileTextureCache::remove(renderer::Texture* p_texture)
{
	if (p_texture == 0 || ms_textures.empty())
	{
		return;
	}
	
	tt::engine::EngineID id(p_texture->getEngineID());
	TextureContainer::iterator it = ms_textures.find(id);
	if (it != ms_textures.end())
	{
		ms_textures.erase(it);
		delete p_texture;
		return;
	}
	
#if !defined(TT_BUILD_FINAL)
	TT_PANIC("Couldn't find Texture %p (ID 1: %u, 2: %u, name: '%s'.) for removal.\n",
	         p_texture, id.crc1, id.crc2, id.getName().c_str());
#endif
}


// Namespace end
}
}
}
