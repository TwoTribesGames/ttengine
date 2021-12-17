#if !defined(TT_ENGINE_RENDERER_TEXTURE_H)
#define TT_ENGINE_RENDERER_TEXTURE_H


#include <string>

#include <tt/engine/cache/ResourceCache.h>
#include <tt/engine/fwd.h>
//#include <tt/engine/renderer/MultiTexture.h> // FIXME: Remove when inline code below is fixed.
#include <tt/engine/opengl_headers.h>
#include <tt/engine/renderer/TextureBase.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>
#include <tt/thread/Mutex.h>


namespace tt {
namespace engine {

// Forward declaration
namespace cache {
class FileTextureCache;
}
namespace renderer {

// Forward declarations
class Texture;
class TexturePainter;

// Resource Management
typedef cache::ResourceCache<Texture> TextureCache;


class Texture : public TextureBase
{
public:
	static const file::FileType fileType = file::FileType_Texture;
	static const bool hasResourceHeader = true;
	
	
	Texture(const TextureBaseInfo& p_info);
	
	virtual ~Texture();
	
	static Texture* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	
	/*! \return A texture suitable for rendering text into. */
	static TexturePtr createForText(s16 p_width, s16 p_height, bool p_usePremultipliedAlpha = false);
	
	inline static TexturePtr createForText(const tt::math::Point2& p_size, bool p_usePremultipliedAlpha = false)
	{ return createForText(static_cast<s16>(p_size.x), static_cast<s16>(p_size.y), p_usePremultipliedAlpha); }
	
	static TexturePtr createForRenderTarget(s16 p_width, s16 p_height);
	
	static TexturePtr createFromBuffer(const TextureBaseInfo& p_info, u8* p_imageData, s32 p_imageDataSize);
	
#if defined(TT_PLATFORM_OSX_IPHONE)
	// iPhone-specific
	static TexturePtr createFromUIImage(void* p_uiImage);
#endif
	
	/*! \brief Lock texture.
	    \return TexturePainter which will unlock the Texture when destroyed. */
	TexturePainter lock();
	
	/*! \brief Use this texture for rendering.
	    \param p_channel The stage in which to load the texture. */
	void select(u8 p_channel = 0);
	
	/* \brief Load texture from file */
	bool load(const fs::FilePtr& p_file);
	
	/* \brief Load texture from file */
	bool loadFromFile(const std::string& p_filename, fs::identifier p_fileSystem = 0);
	
	void preload();
	
	// Mipmap LOD bias override functions
	static void forceMipmapLodBias  (real p_bias);
	static void restoreMipmapLodBias();
	
	
	//////////////////////////////////
	// OpenGL specific
	
	inline GLuint getGLName() const { return m_textureName; }
	
	
	static void setActiveChannel      (s32 p_channel);
	static void setActiveClientChannel(s32 p_channel);
	static void unbindTexture         (s32 p_channel = 0);
	
	static void clearDeathRow();
	
	
private:
	Texture(const EngineID& p_id);
	
	// No copy & assignment
	Texture(const Texture& p_texture);
	Texture& operator=(const Texture&);
	
	void createGLTexture();
	void bindTexture  ();
	void deleteTexture();
	void updateGLSamplerState();
	
	void checkDimensions();
	
	void copyPixelsToTexture();
	void copyPixelsToVolumeTexture();
	
	// OpenGL specific
	GLuint m_textureName;
	GLenum m_target; // 2D / 3D
	bool   m_pixelDataChanged;
	
	typedef std::vector<GLuint> DeathRow;
	static DeathRow      ms_deathRow;
	static thread::Mutex ms_deathRowMutex;
	
	static s32     ms_activeChannel;
	static s32     ms_activeClientChannel;
	
	friend class cache::FileTextureCache;
	friend class TexturePainter;
};

// Namespace end
}
}
}


#endif  // !defined(TT_ENGINE_RENDERER_TEXTURE_H)
