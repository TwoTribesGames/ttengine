#if !defined(INC_TT_ENGINE_CACHE_TEXTUREDATACACHE_H)
#define INC_TT_ENGINE_CACHE_TEXTUREDATACACHE_H

#include <map>
#include <string>

#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/math/hash/Hash.h>
#include <tt/platform/tt_error.h>

#if !defined(TT_BUILD_FINAL)
	//#define TT_TEXTURE_TIMER_ON
#endif //#if !defined(TT_BUILD_FINAL)


namespace tt {
namespace engine {
namespace cache {

/*
OSX only class.

This helper class can be used to store texture data (The pixel data) in memory.
Use this to store/cache texture data without using up VRAM.
Before loading a file from disk and decompressing a png the Texture load code will check this cache.
 
OSX ONLY!
*/
	
class TextureDataCache;

class TextureData
{
public:
	typedef tt::math::hash::Hash<32> IDType;
	
	inline IDType         getID()     const { return m_id;     }
	inline GLint          getFormat() const { return m_format; }
	inline GLsizei        getWidth()  const { return m_width;  }
	inline GLsizei        getHeight() const { return m_height; }
	inline const GLubyte* getData()   const { return m_data;   }
	inline       GLubyte* getData()         { return m_data;   }
	inline s32            getBytesPerPixel() const { return m_bytesPerPixel; }
	
	inline ~TextureData() { delete[] m_data; }
	
private:
	IDType   m_id;
	GLubyte* m_data;
	GLsizei  m_width;
	GLsizei  m_height;
	//bool     m_alpha;
	GLint    m_format; // Texture format and internal format are the same in openGL ES.
	
	s32      m_bytesPerPixel;
	s32      m_size;
	
	
	TextureData(IDType p_id, GLsizei p_width, GLsizei p_height, GLint p_format)
	:
	m_id(p_id),
	m_data(0),
	m_width(p_width),
	m_height(p_height),
	//m_alpha(false),
	m_format(p_format),
	m_bytesPerPixel(0),
	m_size(0)
	{
		TT_ASSERT(p_width  > 0);
		TT_ASSERT(p_height > 0);
		
		if (m_format == GL_RGBA)
		{
			m_bytesPerPixel = 4; // RGBA
		}
		else
		{
			m_bytesPerPixel = 1; // Luminance
			TT_ASSERTMSG(m_format == GL_LUMINANCE, "Unsupported texture format: %d", p_format);
		}
		
		TT_ASSERT(m_bytesPerPixel != 0);
		
		m_size = m_width * m_height * m_bytesPerPixel;
		m_data = new GLubyte[m_size];
	}
	
	// No copying
	TextureData(const TextureData&);
	TextureData& operator=(const TextureData&);
	
	friend class TextureDataCache;
};


typedef tt_ptr<const TextureData>::shared ConstTextureDataPtr;
typedef tt_ptr<      TextureData>::shared      TextureDataPtr;
typedef tt_ptr<      TextureData>::weak        TextureDataWeakPtr;

class TextureDataCache
{
public:
	static ConstTextureDataPtr get(const std::string& p_filename);
	static ConstTextureDataPtr find(const std::string& p_filename);
	
	static void dump();
	
private:
	TextureDataCache();  // Static class. Not implemented.
	~TextureDataCache(); // Static class. Not implemented.
	
	static ConstTextureDataPtr load(const std::string& p_filename);
	static void remove(TextureData* p_textureData);
	
	
	typedef std::map<TextureData::IDType, TextureDataWeakPtr> TextureContainer;
	static TextureContainer ms_textures;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_CACHE_TEXTUREDATACACHE_H)
