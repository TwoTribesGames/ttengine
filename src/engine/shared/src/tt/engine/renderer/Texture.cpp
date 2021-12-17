#include <tt/engine/renderer/Texture.h>

#include <tt/compression/png.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/str/common.h>
#include <tt/thread/CriticalSection.h>

#if defined(TT_PLATFORM_OSX) && !defined(TT_PLATFORM_SDL)
#define TT_USE_TEXTURE_CACHE
#endif

#ifdef TT_USE_TEXTURE_CACHE
#include <tt/engine/cache/TextureDataCache.h>
#endif


namespace tt {
namespace engine {
namespace renderer {
	
	
Texture::DeathRow Texture::ms_deathRow;
thread::Mutex     Texture::ms_deathRowMutex;
s32               Texture::ms_activeChannel       = 0;
s32               Texture::ms_activeClientChannel = 0;


//--------------------------------------------------------------------------------------------------
// Helper functions

static GLint getGLAddressMode(AddressMode p_mode)
{
	TT_ASSERT(isValidAddressMode(p_mode));
	//TT_Printf("Setting address mode to: %s\n", getAddressModeName(p_mode));
	
	switch(p_mode)
	{
		case AddressMode_Clamp : return GL_CLAMP_TO_EDGE;
		case AddressMode_Mirror:
#if defined(TT_PLATFORM_OSX_IPHONE)
			return GL_MIRRORED_REPEAT_OES;
#else
			return GL_MIRRORED_REPEAT;
#endif
		case AddressMode_Wrap  : return GL_REPEAT;
		default                : return GL_REPEAT;
	}
}


static GLint getGLMagFilter(FilterMode p_mode)
{
	TT_ASSERT(isValidFilterMode(p_mode));
	
	switch(p_mode)
	{
		case FilterMode_None  : return GL_NEAREST;
		case FilterMode_Point : return GL_NEAREST;
		case FilterMode_Linear: return GL_LINEAR;
		default               : return GL_NEAREST;
	}
}
	
	
static GLint getGLMinFilter(FilterMode p_minFilter, FilterMode p_mipFilter)
{
	TT_ASSERT(isValidFilterMode(p_minFilter));
	TT_ASSERT(isValidFilterMode(p_mipFilter));
	
	switch(p_mipFilter)
	{
		case FilterMode_None:
			return (p_minFilter == FilterMode_Linear) ? GL_LINEAR : GL_NEAREST;

		case FilterMode_Point:
			return (p_minFilter == FilterMode_Linear) ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;

		case FilterMode_Linear:
			return (p_minFilter == FilterMode_Linear) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
			
		default:
			return GL_NEAREST;
	}
}
	

static GLenum getGLFormat(ImageFormat p_format)
{
	switch(p_format)
	{
		case ImageFormat_ARGB8: return GL_BGRA;
		case ImageFormat_RGBA8: return GL_RGBA;
		case ImageFormat_A8   : return GL_ALPHA;
		case ImageFormat_I8   : return GL_LUMINANCE;
		default:
			TT_PANIC("Image format (%d) not supported", p_format);
			return GL_RGBA;
	}
}
	
	
//--------------------------------------------------------------------------------------------------
// Public member functions

Texture::Texture(const EngineID& p_id)
:
TextureBase(p_id),
m_textureName(0),
m_target(GL_TEXTURE_2D),
m_pixelDataChanged(false)
{
}


Texture::Texture(const TextureBaseInfo& p_info)
:
TextureBase(p_info),
m_textureName(0),
m_target(GL_TEXTURE_2D),
m_pixelDataChanged(false)
{
	if(m_info.usage != Usage_RenderTarget) checkDimensions();
	
	if(m_info.usage == Usage_Text)
	{
		// Allocate memory for pixel data (used for TexturePainter)
		mem::size_type imageSize = m_info.width * m_info.height * getBytesPerPixel();
		m_pixels = static_cast<u8*>(mem::alloc(imageSize));
		mem::zero8(m_pixels, imageSize);
		setMipmapFilter(FilterMode_Linear);
	}
}
	

Texture::~Texture()
{
	deleteTexture();
}
	
	
Texture* Texture::create(const fs::FilePtr&, const EngineID& p_id, u32)
{
	return new Texture(p_id);
}


TexturePtr Texture::createForText(s16 p_width, s16 p_height, bool p_usePremultipliedAlpha)
{
	TextureBaseInfo info;
	info.width         = p_width;
	info.height        = p_height;
	info.usage         = Usage_Text;
	info.premultiplied = p_usePremultipliedAlpha;
	return TexturePtr(new Texture(info));
}


TexturePtr Texture::createForRenderTarget(s16 p_width, s16 p_height)
{
	TextureBaseInfo info;
	info.width  = p_width;
	info.height = p_height;
	info.usage  = Usage_RenderTarget;
	return TexturePtr(new Texture(info));
}
	
	
TexturePtr Texture::createFromBuffer(const TextureBaseInfo& p_info, u8* p_imageData, s32 p_imageDataSize)
{
	TexturePtr texture(new Texture(p_info));
	
	texture->m_pixels = static_cast<u8*>(mem::alloc(p_imageDataSize));
	
	mem::copy8(texture->m_pixels, p_imageData, p_imageDataSize);
	
	return texture;
}


TexturePainter Texture::lock()
{	
	return TexturePainter(this);
}
	
	
void Texture::select(u8 p_channel)
{
	if(m_textureName == 0)
	{
		createGLTexture();
	}
	
	setActiveChannel(p_channel);
	bindTexture();
	
	if (m_pixelDataChanged)
	{
		TT_ASSERT(m_info.type == Type_Texture && m_info.usage == Usage_Text);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_info.width, m_info.height, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
		m_pixelDataChanged = false;
	}
	
	if (m_state != m_hwState)
	{
		updateGLSamplerState();
	}
}
	

bool Texture::load(const fs::FilePtr& p_file)
{
	if (TextureBase::loadPixelData(p_file, compression::Transform_SwapColorChannels) == false)
	{
		return false;
	}
	
	m_target = (m_info.type == Type_VolumeTexture) ? GL_TEXTURE_3D : GL_TEXTURE_2D;
	
	return true;
}


bool Texture::loadFromFile(const std::string& p_filename, fs::identifier p_fileSystem)
{
	if (str::endsWith(p_filename, ".png"))
	{
		if (fs::fileExists(p_filename, p_fileSystem) == false)
		{
			TT_PANIC("Cannot load file '%s' as texture: file does not exist.", p_filename.c_str());
			return false;
		}
		
		return loadFromPNG(fs::open(p_filename, fs::OpenMode_Read, p_fileSystem), compression::Transform_SwapColorChannels);
	}
	// FIXME: Provide old implementation
	
	TT_ASSERTMSG(p_fileSystem == 0, "Deprecated Texture loadFromFile code only works with the default file system.");
	TT_WARN("[DEPRECATED] This function should not be used anymore in production code! "
			"All texture loading should use the ETX format.");
	
#ifdef TT_USE_TEXTURE_CACHE
	cache::ConstTextureDataPtr data = cache::TextureDataCache::get(p_filename);
	
	if (data == 0)
	{
		// Loading for pixel data failed.
		return false;
	}
	
	m_info.width  = data->getWidth();
	m_info.height = data->getHeight();
	
	// FIXME: Determine format
	TT_ASSERTMSG(data->getBytesPerPixel() == 4, "Only RGBA textures are supported");
	
	// Copy image data
	mem::size_type imageSize = data->getWidth() * data->getHeight() * data->getBytesPerPixel();
	m_pixels = static_cast<u8*>(mem::alloc(imageSize));
	mem::copy8(m_pixels, data->getData(), imageSize);
	
	// If the TextureDataCache was not set to correct texture dimensions (in data)
	// checkDimensions could give a warning and could alter m_width and m_height (with un-altered data)
	checkDimensions();
	
	return true;
#else
	return false;
#endif
}
	
	
void Texture::preload()
{
	if (m_textureName == 0)
	{
		createGLTexture();
	}
}
	
	
void Texture::forceMipmapLodBias(real p_bias)
{
	ms_forceMipmapLodBias = true;
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, p_bias);
}
	
	
void Texture::restoreMipmapLodBias()
{
	ms_forceMipmapLodBias = false;
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, 0.0f);
}
	
	
//--------------------------------------------------------------------------------------------------
// OpenGL specific functions
	

void Texture::setActiveChannel(s32 p_channel)
{
	if (p_channel != ms_activeChannel)
	{
		glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + p_channel));
		ms_activeChannel = p_channel;
	}
	
	TT_CHECK_OPENGL_ERROR();
}

	
void Texture::setActiveClientChannel(s32 p_channel)
{
	if (p_channel != ms_activeClientChannel)
	{
		glClientActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + p_channel));
		ms_activeClientChannel = p_channel;
	}
	
	TT_CHECK_OPENGL_ERROR();
}
	

void Texture::unbindTexture(s32 p_channel)
{
	setActiveChannel(p_channel);
	if(MultiTexture::getActiveTexture(p_channel) != 0)
	{
		Texture *old = MultiTexture::getActiveTexture(p_channel);
		glBindTexture(old->m_target, 0);
		MultiTexture::setActiveTexture(0, p_channel);
	}
	
	TT_CHECK_OPENGL_ERROR();
}


void Texture::clearDeathRow()
{
	thread::CriticalSection criticalSection(&ms_deathRowMutex);
	glDeleteTextures(static_cast<GLsizei>(ms_deathRow.size()), &ms_deathRow[0]);
	ms_deathRow.clear();
	
	TT_CHECK_OPENGL_ERROR();
}


//--------------------------------------------------------------------------------------------------
// Private member functions
	
static const u32 OpenGLDefaultAddressMode = 0x2A;
static const u32 OpenGLDefaultFilterMode  = 0x29;
	

void Texture::createGLTexture()
{
	Renderer::getInstance()->checkFromRenderThread();
	
	if(m_textureName != 0)
	{
		deleteTexture();
	}
	
	glGenTextures(1, &m_textureName);
	bindTexture();
	
	// Set hardware state (default OpenGL)
	m_hwState.addressMode = OpenGLDefaultAddressMode;
	m_hwState.filterMode  = OpenGLDefaultFilterMode;
	
	if (m_info.type == Type_VolumeTexture)
	{
		copyPixelsToVolumeTexture();
	}
	else
	{
		copyPixelsToTexture();
	}
	
	TT_CHECK_OPENGL_ERROR();
	
	if (m_info.paintable == false)
	{
		// Release image data
		mem::free(m_pixels);
		m_pixels = 0;
	}
}


void Texture::bindTexture()
{
	TT_ASSERTMSG(m_textureName != 0, "No texture data, texture id is 0");
	
	if(MultiTexture::getActiveTexture(ms_activeChannel) != this)
	{
		Texture *old = MultiTexture::getActiveTexture(ms_activeChannel);
		if (old && old->m_target != m_target) {
			glBindTexture(old->m_target, 0);
		}
		MultiTexture::setActiveTexture(this, ms_activeChannel);
		glBindTexture(m_target, m_textureName);

		Renderer::getInstance()->setPremultipliedAlphaEnabled(m_info.premultiplied);
	}
	
	TT_CHECK_OPENGL_ERROR();
}


void Texture::deleteTexture()
{
	if (!Renderer::hasInstance())
	{
		// the renderer has been shutdown so don't bother cleaning up
		return;
	}
	for (u32 i = 0; i < MultiTexture::getChannelCount(); ++i)
	{
		if (MultiTexture::getActiveTexture(i) == this)
		{
			MultiTexture::setActiveTexture(0, i);
		}
	}
	
	if(m_textureName != 0)
	{
		//glDeleteTextures(1, &m_textureName);
		thread::CriticalSection criticalSection(&ms_deathRowMutex);
		ms_deathRow.push_back(m_textureName);
		
		m_textureName = 0;
	}
}

	
void Texture::updateGLSamplerState()
{
	// NOTE: Texture must be bound!
	
	if(hasAddressModeChanged(m_hwState, ADDRESS_U_MASK))
	{
		glTexParameteri(m_target, GL_TEXTURE_WRAP_S, getGLAddressMode(getAddressMode(ADDRESS_U_MASK, 0)));
	}
	
	if(hasAddressModeChanged(m_hwState, ADDRESS_V_MASK))
	{
		glTexParameteri(m_target, GL_TEXTURE_WRAP_T, getGLAddressMode(getAddressMode(ADDRESS_V_MASK, ADDRESS_V_SHIFT)));
	}
	
	if(hasAddressModeChanged(m_hwState, ADDRESS_W_MASK))
	{
		glTexParameteri(m_target, GL_TEXTURE_WRAP_R, getGLAddressMode(getAddressMode(ADDRESS_W_MASK, ADDRESS_W_SHIFT)));
	}
	
	if(hasFilterModeChanged(m_hwState, FILTER_MAG_MASK))
	{
		glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, getGLMagFilter(getFilterMode(FILTER_MAG_MASK, FILTER_MAG_SHIFT)));
	}
	
	if(hasFilterModeChanged(m_hwState, FILTER_MIN_MASK) || hasFilterModeChanged(m_hwState, FILTER_MIP_MASK))
	{
		glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, getGLMinFilter(
			getFilterMode(FILTER_MIN_MASK, 0), getFilterMode(FILTER_MIP_MASK, FILTER_MIP_SHIFT)));
	}
	
	TT_CHECK_OPENGL_ERROR();
	
	// In sync again
	m_hwState = m_state;
}

	
void Texture::checkDimensions()
{
	TextureBase::checkDimensions();
}
	
	
void Texture::copyPixelsToTexture()
{
	// Load mip levels (0 = original texture)
	GLsizei mipWidth    = static_cast<GLsizei>(m_info.width);
	GLsizei mipHeight   = static_cast<GLsizei>(m_info.height);
	GLenum  pixelFormat = getGLFormat(m_info.format);
	const u32 bytesPerPixel = getBytesPerPixel();
	const u8* mipStart(m_pixels);
	
	// FIXME: Proper internal format for alpha textures??
	
	for(GLint mipLevel = 0; mipLevel <= m_info.mipLevels; ++mipLevel)
	{
		glTexImage2D(GL_TEXTURE_2D,     // 2D Texture
					 mipLevel,          // Mipmap level
					 GL_RGBA,           // Internal format (number of color components)
					 mipWidth,          // Width of current mip level
					 mipHeight,         // Height of current mip level
					 0,                 // No border
					 pixelFormat,       // Format of the texel data
					 GL_UNSIGNED_BYTE,  // Type of single texel
					 mipStart);         // Texel data
		
		// Compute size of this mipmap
		u32 mipmapSize(mipWidth * mipHeight * bytesPerPixel);
		
		// Compute settings for next mipmap
		mipStart += mipmapSize;
		mipWidth  = std::max(mipWidth  / 2, 1);
		mipHeight = std::max(mipHeight / 2, 1);
	}
	
	if (m_info.usage != Usage_Text)
	{
		// Set maximum available mipmap level
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_info.mipLevels);
	} else {
		// Generate mipmaps for textures containing text
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	
	//TT_Printf("Created texture of size %dx%d with %d mipmaps\n", m_info.width, m_info.height, m_info.mipLevels);
}
	
	
void Texture::copyPixelsToVolumeTexture()
{
	glTexImage3D(m_target,                   // 3D Texture
				 0,                          // Top-level mipmap (base image)
				 GL_RGBA,                    // Internal format
				 m_info.width,               // Width of texture
				 m_info.height,              // Height of texture
				 m_info.depth,               // Depth of texture
				 0,                          // No border
				 getGLFormat(m_info.format), // Format of the texel data
				 GL_UNSIGNED_BYTE,           // Type of single texel
				 m_pixels);                  // Texel data
}

	
	
}
}
}
