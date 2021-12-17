#if !defined(INC_TT_ENGINE_RENDERER_TEXTURE_H)
#define INC_TT_ENGINE_RENDERER_TEXTURE_H


#include <string>

#define NOMINMAX
#include <d3dx9.h>

#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/D3DResource.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/TextureBase.h>
#include <tt/engine/cache/ResourcePersistentCache.h>
#include <tt/engine/cache/FileTextureCache.h>
#include <tt/math/Point2.h>


namespace tt {
namespace engine {
namespace renderer {

// Forward declaration
class Renderer;

// Resource Management
typedef cache::ResourcePersistentCache<Texture> TextureCache;


class Texture : public TextureBase, public D3DResource
{
public:
	static const file::FileType fileType = file::FileType_Texture;
	static const bool hasResourceHeader = true;
	
	Texture(const TextureBaseInfo& p_info);
	
	virtual ~Texture();
	
	static Texture* create(const fs::FilePtr& p_file, const EngineID& p_id, u32 p_flags);
	
	static TexturePtr createForText(s16 p_width, s16 p_height, bool p_usePremultipliedAlpha = false);
	
	inline static TexturePtr createForText(const tt::math::Point2& p_size, bool p_usePremultipliedAlpha = false)
	{ return createForText(static_cast<s16>(p_size.x), static_cast<s16>(p_size.y), p_usePremultipliedAlpha); }
	
	static TexturePtr createForRenderTarget(s16 p_width, s16 p_height);
	
	static TexturePtr createFromBuffer(const TextureBaseInfo& p_info, u8* p_imageData, s32 p_imageDataSize);
	
	/*! \brief Lock texture.
	    \return TexturePainter which will unlock the Texture when destroyed. */
	TexturePainter lock();
	
	/* \brief Use this texture for rendering */
	void select(u32 p_channel = 0);
	
	/* \brief Load texture from file */
	bool load(const fs::FilePtr& p_file);
	
	bool loadFromFile(const std::string& p_filename, fs::identifier p_fileSystem = 0);
	
	void postload();
	
	void resize(s32 p_width, s32 p_height);
	
	// Mipmap LOD bias override functions
	static void forceMipmapLodBias  (real p_bias);
	static void restoreMipmapLodBias();
	
	
	///////////////////////////////
	// Windows Specific
	
	inline IDirect3DBaseTexture9* getD3DTexture() { return m_texture; }
	
	virtual void deviceCreated();
	virtual void deviceReset();
	virtual void deviceLost();
	virtual void deviceDestroyed();

	// Call this when 3rd party code changed states or
	static void resetSamplerState(u32 p_channel);
	static void resetAllSamplerStates();
	
private:
	Texture(const EngineID& p_id, u32 p_flags = 0);
	
	// Disable copy and assigment.
	Texture(const Texture& p_texture);
	const Texture& operator=(const Texture& p_rhs);
	
	bool createD3DTexture(IDirect3DDevice9& p_device);
	bool createD3DVolumeTexture(IDirect3DDevice9& p_device);
	bool copyPixelsToTexture();
	bool copyPixelsToVolumeTexture();
	bool updateD3DTextureState(u32 p_channel);
	
	static void setMipmapLodBias(real p_bias, u32 p_channel = 0);
	
	void checkDimensions();
	
	
	std::string    m_filename;
	fs::identifier m_fileSystem;
	bool           m_pixelDataChanged;
	
	// DirectX specific
	IDirect3DBaseTexture9* m_texture;
	
	static const u32 maxAvailableSamplers = 16;
	static TextureState ms_samplerState[maxAvailableSamplers];
	
	friend class cache::FileTextureCache;
	friend class Renderer; // For setDeviceSupportsNonPowerOfTwo.
	friend class TexturePainter;
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_TEXTURE_H
