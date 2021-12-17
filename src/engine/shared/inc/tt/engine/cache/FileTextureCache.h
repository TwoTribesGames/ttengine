#if !defined(INC_TT_ENGINE_CACHE_FILETEXTURECACHE_H)
#define INC_TT_ENGINE_CACHE_FILETEXTURECACHE_H


#include <map>
#include <string>

#include <tt/engine/EngineID.h>
#include <tt/engine/fwd.h>
#include <tt/fs/types.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace cache {


// Use this to load and cache Texture directly from a file.


class FileTextureCache
{
public:
	static renderer::TexturePtr get(const std::string& p_filename, fs::identifier p_fileSystem = 0);
	
	static renderer::TexturePtr find(const std::string& p_filename, fs::identifier p_fileSystem = 0);
	
	static void dump();
	static void dumpToFile(const std::string& p_filename, bool p_append = true);
	static s32 getTotalMemory();
	
private:
	// Static class
	FileTextureCache() {}
	~FileTextureCache() {}
	
	static renderer::TexturePtr load(const std::string& p_filename, fs::identifier p_fileSystem);
	static void remove(renderer::Texture* p_texture);
	
	typedef std::map<EngineID, renderer::TextureWeakPtr, EngineIDLess> TextureContainer;
	static TextureContainer ms_textures;
};


// Namespace end
}
}
}


#endif // !defined(INC_TT_ENGINE_CACHE_FILETEXTURECACHE_H)
