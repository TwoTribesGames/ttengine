#include <memory>

#include <tt/engine/renderer/Texture.h>

#include <tt/compression/png.h>
#include <tt/engine/renderer/directx.h>
#include <tt/engine/renderer/D3DResourceRegistry.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/TexturePainter.h>
#include <tt/engine/renderer/MultiTexture.h>
#include <tt/fs/WindowsFileSystem.h>
#include <tt/mem/mem.h>
#include <tt/mem/util.h>
#include <tt/str/common.h>


namespace tt {
namespace engine {
namespace renderer {

// Keep track of global sampler state (per channel)
TextureState Texture::ms_samplerState[Texture::maxAvailableSamplers];


//--------------------------------------------------------------------------------------------------
// Helper functions

static D3DFORMAT getD3DFormat(ImageFormat p_format)
{
	switch(p_format)
	{
	case ImageFormat_ARGB8: return D3DFMT_A8R8G8B8;
	case ImageFormat_RGBA8: return D3DFMT_A8R8G8B8;
	case ImageFormat_A8:    return D3DFMT_A8;
	case ImageFormat_I8:    return D3DFMT_L8;

	default:
		TT_PANIC("Unsupported texel format.");
	}

	return D3DFMT_A8R8G8B8;
}


static DWORD getD3DUsage(TextureUsage p_usage)
{
	switch(p_usage)
	{
	case Usage_Normal:           return 0;
	case Usage_MipVisualization: return 0;
	case Usage_Text:             return D3DUSAGE_AUTOGENMIPMAP;
	case Usage_RenderTarget:     return D3DUSAGE_RENDERTARGET;

	default:
		TT_PANIC("Invalid texture usage.");
	}

	return 0;
}


static D3DPOOL getPoolFromUsage(TextureUsage p_usage)
{
	switch(p_usage)
	{
	case Usage_Normal:           return D3DPOOL_MANAGED;
	case Usage_MipVisualization: return D3DPOOL_MANAGED;
	case Usage_Text:             return D3DPOOL_MANAGED;
	case Usage_RenderTarget:     return D3DPOOL_DEFAULT;

	default:
		TT_PANIC("Invalid texture usage.");
	}

	return D3DPOOL_MANAGED;
}


static D3DTEXTUREADDRESS getD3DAddressMode(AddressMode p_mode)
{
	TT_ASSERT(isValidAddressMode(p_mode));

	switch(p_mode)
	{
	case AddressMode_Clamp : return D3DTADDRESS_CLAMP;
	case AddressMode_Mirror: return D3DTADDRESS_MIRROR;
	case AddressMode_Wrap  : return D3DTADDRESS_WRAP;
	default                : return D3DTADDRESS_WRAP;
	}
}


static D3DTEXTUREFILTERTYPE getD3DFilterMode(FilterMode p_mode)
{
	TT_STATIC_ASSERT(D3DTEXF_NONE        == FilterMode_None);
	TT_STATIC_ASSERT(D3DTEXF_POINT       == FilterMode_Point);
	TT_STATIC_ASSERT(D3DTEXF_LINEAR      == FilterMode_Linear);
	TT_STATIC_ASSERT(D3DTEXF_ANISOTROPIC == FilterMode_Anisotropic);

	TT_ASSERT(isValidFilterMode(p_mode));

	return static_cast<D3DTEXTUREFILTERTYPE>(p_mode);
}

//--------------------------------------------------------------------------------------------------
// Public member functions

Texture::Texture(const EngineID& p_id, u32 p_flags)
:
TextureBase(p_id, p_flags),
m_filename(),
m_fileSystem(0),
m_pixelDataChanged(false),
m_texture(nullptr)
{
	// Register this resource
	D3DResourceRegistry::registerResource(this);
}


Texture::Texture(const TextureBaseInfo& p_info)
:
TextureBase(p_info),
m_filename(),
m_fileSystem(0),
m_pixelDataChanged(false),
m_texture(nullptr)
{
	if(m_info.usage != Usage_RenderTarget) checkDimensions();
	
	// Register this resource
	D3DResourceRegistry::registerResource(this);
	
	if (m_info.usage == Usage_Text)
	{
		mem::size_type imageSize = m_info.width * m_info.height * getBytesPerPixel();
		m_pixels = static_cast<u8*>(mem::alloc(imageSize));
		mem::zero8(m_pixels, imageSize);
		setMipmapFilter(FilterMode_Linear);
	}
}


Texture::~Texture()
{
	// Un-Register this resource
	D3DResourceRegistry::unregisterResource(this);
	Renderer::addToDeathRow(m_texture);
}


Texture* Texture::create(const fs::FilePtr&, const EngineID& p_id, u32 p_flags)
{
	return new Texture(p_id, p_flags);
}


TexturePtr Texture::createForText(s16 p_width, s16 p_height, bool p_usePremultipliedAlpha)
{
	TextureBaseInfo info;
	info.width         = p_width;
	info.height        = p_height;
	info.usage         = Usage_Text;
	info.premultiplied = p_usePremultipliedAlpha;
	return std::make_shared<Texture>(info);
}


TexturePtr Texture::createForRenderTarget(s16 p_width, s16 p_height)
{
	TextureBaseInfo info;
	info.width  = p_width;
	info.height = p_height;
	info.usage  = Usage_RenderTarget;
	return std::make_shared<Texture>(info);
}


TexturePtr Texture::createFromBuffer(const TextureBaseInfo& p_info, u8* p_imageData, s32 p_imageDataSize)
{
	TexturePtr texture = std::make_shared<Texture>(p_info);
	
	texture->m_pixels = static_cast<u8*>(mem::alloc(p_imageDataSize));
	mem::copy8(texture->m_pixels, p_imageData, p_imageDataSize);
	
	return texture;
}


TexturePainter Texture::lock()
{
	TT_ASSERTMSG(m_info.type == Type_Texture, "Can only lock regular textures for painting");
	
	return TexturePainter(this);
}


void Texture::select(u32 p_channel)
{
	if (getRenderDevice(true) == nullptr) return;
	
	Renderer::getInstance()->checkFromRenderThread();
	
	if (m_texture == nullptr)
	{
		deviceCreated();
		deviceReset();
	}
	
	if (m_pixelDataChanged)
	{
		switch (m_info.type)
		{
		case Type_Texture:       copyPixelsToTexture();       break;
		case Type_VolumeTexture: copyPixelsToVolumeTexture(); break;
		default:
			TT_PANIC("Unhandled texture type '%d'", m_info.type);
			break;
		}
		
		m_pixelDataChanged = false;
	}
	
	// Set sampler state
	if (m_state != ms_samplerState[p_channel])
	{
		updateD3DTextureState(p_channel);
	}
	
	if (MultiTexture::getActiveTexture(p_channel) != this)
	{
		if (SUCCEEDED(getRenderDevice()->SetTexture(p_channel, m_texture)))
		{
			// Cache active texture
			MultiTexture::setActiveTexture(this, p_channel);
			Renderer::getInstance()->setPremultipliedAlphaEnabled(m_info.premultiplied);
		}
		else
		{
			TT_PANIC("Unable to set texture");
		}
	}
}


bool Texture::load(const fs::FilePtr& p_file)
{
	if (m_texture != 0)
	{
		// Aparently we already had a texture, so this means that the pixeldata has changed
		m_pixelDataChanged = true;
	}
	const bool loadResult = TextureBase::loadPixelData(p_file, compression::Transform_SwapColorChannels);
	if (loadResult)
	{
		postload();
	}
	return loadResult;
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
		
		const bool loadOk = loadFromPNG(fs::open(p_filename, fs::OpenMode_Read, p_fileSystem), compression::Transform_SwapColorChannels);
		if (loadOk)
		{
			m_filename   = p_filename;
			m_fileSystem = p_fileSystem;
		}
		return loadOk;
	}

	TT_WARN("[DEPRECATED] This function should not be used anymore in production code! "
		"All texture loading should use the ETX format.");

	// Make sure the file exists
	TT_ASSERTMSG(p_fileSystem == 0, "Deprecated Texture loadFromFile code only works with the default file system.");
	TT_ASSERTMSG(fs::fileExists(p_filename), "File Not Found: %s\n", p_filename.c_str());
	fs::WindowsFileSystem::validatePathCasing(p_filename);

	// Make sure previous bindings are released
	safeRelease(m_texture);

	if(getRenderDevice(true) == nullptr) return false;

	D3DXIMAGE_INFO info;
	mem::zero8(&info, sizeof(info));
	
	// TODO: Remove dependency on D3DX
	IDirect3DTexture9* texture(0);

	if(checkD3DSucceeded(D3DXCreateTextureFromFileExA(
		getRenderDevice(), p_filename.c_str(),
		TextureBase::usePowerOfTwo() ? D3DX_DEFAULT : D3DX_DEFAULT_NONPOW2,
		TextureBase::usePowerOfTwo() ? D3DX_DEFAULT : D3DX_DEFAULT_NONPOW2,
		D3DX_FROM_FILE, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
	    D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR,
		0, &info, nullptr, &texture)) == false)
	{
		return false;
	}
	m_texture = texture;

	// Store to be able to recreate when device is re-created
	m_filename   = p_filename;
	m_fileSystem = 0;

	TT_ASSERT(info.ResourceType == D3DRTYPE_TEXTURE);
	TT_ASSERT(info.MipLevels >= 1);
	TT_ASSERT(info.Format == D3DFMT_A8R8G8B8);

	// Fill texture information
	m_info.width     = static_cast<u16>(info.Width);
	m_info.height    = static_cast<u16>(info.Height);
	m_info.depth     = 0;
	m_info.format    = ImageFormat_ARGB8;
	m_info.mipLevels = static_cast<u16>(info.MipLevels - 1);
	
	// Dimension Check
	checkDimensions();
	
	return true;
}


void Texture::postload()
{
	if (m_texture == nullptr)
	{
		deviceCreated();
		deviceReset();
	}
}


void Texture::resize(s32 p_width, s32 p_height)
{
	TT_ASSERT(m_info.usage == Usage_RenderTarget);
	m_info.width  = static_cast<s16>(p_width);
	m_info.height = static_cast<s16>(p_height);
	
	IDirect3DDevice9* device(getRenderDevice(true));
	if (device != nullptr)
	{
		createD3DTexture(*device);
	}
}


void Texture::forceMipmapLodBias(real p_bias)
{
	// NOTE: Only for channel 0 for now
	ms_forceMipmapLodBias = true;

	setMipmapLodBias(p_bias);
}


void Texture::restoreMipmapLodBias()
{
	// NOTE: Only for channel 0 for now
	ms_forceMipmapLodBias = false;

	setMipmapLodBias(0.0f);
}


//--------------------------------------------------------------------------------------------------
// D3DResourceRegistry overloads


void Texture::deviceCreated()
{
	// Only recreate textures from D3DPOOL_MANAGED here
	if(getPoolFromUsage(m_info.usage) != D3DPOOL_MANAGED) return;

	if(m_filename.empty() == false)
	{
		// Still need to do this here since D3DX combines loading with texture creation
		loadFromFile(m_filename, m_fileSystem);
	}
	else if(m_info.usage == Usage_Normal && m_pixels == 0)
	{
		fs::FilePtr file(file::FileUtils::getInstance()->getDataFile(
			getEngineID(), file::FileType_Texture));
		
		if (file != 0)
		{
			// Skip header
			file->seek(sizeof(file::ResourceHeader), fs::SeekPos_Cur);
			
			TextureBase::loadPixelData(file, compression::Transform_SwapColorChannels);
		}
		else
		{
			TT_PANIC("Failed to load file to recreate texture (%s)", getEngineID().toDebugString().c_str());
		}
	}

	// Device must be created at this point
	IDirect3DDevice9* device(getRenderDevice(true));
	if(device == nullptr) return;

	// Recreate
	if (m_info.type == Type_VolumeTexture)
	{
		if (createD3DVolumeTexture(*device))
		{
			copyPixelsToVolumeTexture();
		}
		else
		{
			TT_PANIC("Failed to (re-)create volume texture resource.");
		}
	}
	else
	{
		if(createD3DTexture(*device))
		{
			copyPixelsToTexture();
		}
		else
		{
			TT_PANIC("Failed to (Re-)create texture resource.");
		}
	}
}


void Texture::deviceReset()
{
	// Only handle textures from D3DPOOL_DEFAULT here
	if(getPoolFromUsage(m_info.usage) != D3DPOOL_DEFAULT) return;

	// Only if the texture needs to be created
	if(m_texture == nullptr)
	{
		// Device must be created at this point
		IDirect3DDevice9* device(getRenderDevice(true));
		if(device == nullptr) return;

		if(createD3DTexture(*device) == false)
		{
			TT_PANIC("Failed to (re-)construct render target with dimensions (%d,%d)\n",
				m_info.width, m_info.height);
		}
	}
}


void Texture::deviceLost()
{
	// Only handle textures from D3DPOOL_DEFAULT here
	if(getPoolFromUsage(m_info.usage) == D3DPOOL_DEFAULT)
	{
		safeRelease(m_texture);
	}
}


void Texture::deviceDestroyed()
{
	// Only handle textures from D3DPOOL_MANAGED here
	if (getPoolFromUsage(m_info.usage) == D3DPOOL_MANAGED)
	{
		safeRelease(m_texture);
	}
}


void Texture::resetSamplerState(u32 p_channel)
{
	static const u32 DirectXDefaultAddressMode = 0x2A; // 101010 => All Wrap
	static const u32 DirectXDefaultFilterMode  = 0x5;  // 000101 => None/Point/Point

	ms_samplerState[p_channel].addressMode = DirectXDefaultAddressMode;
	ms_samplerState[p_channel].filterMode  = DirectXDefaultFilterMode;
}


void Texture::resetAllSamplerStates()
{
	for(u32 i = 0; i < maxAvailableSamplers; ++i)
	{
		resetSamplerState(i);
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions


bool Texture::createD3DTexture(IDirect3DDevice9& p_device)
{
	D3DFORMAT texelFormat  = getD3DFormat    (m_info.format);
	DWORD     textureUsage = getD3DUsage     (m_info.usage);
	D3DPOOL   memoryPool   = getPoolFromUsage(m_info.usage);

	safeRelease(m_texture);

	IDirect3DTexture9* texture(0);

	if (checkD3DSucceeded(p_device.CreateTexture(
		m_info.width,         // Width of the texture
		m_info.height,        // Height of the texture
		m_info.mipLevels + 1, // Number of miplevels (includes level 0)
		textureUsage,         // Usage pattern
		texelFormat,          // Texel format
		memoryPool,           // Memory pool (VRAM / RAM)
		&texture,             // Resulting texture object
		nullptr)))            // Reserved (Create texture from system memory in Vista)
	{
		m_texture = texture;
		return true;
	}
	return false;
}


bool Texture::createD3DVolumeTexture(IDirect3DDevice9& p_device)
{
	D3DFORMAT texelFormat  = getD3DFormat    (m_info.format);
	DWORD     textureUsage = getD3DUsage     (m_info.usage);
	D3DPOOL   memoryPool   = getPoolFromUsage(m_info.usage);

	safeRelease(m_texture);

	IDirect3DVolumeTexture9* volumeTexture(0);

	if (checkD3DSucceeded(p_device.CreateVolumeTexture(
		m_info.width,         // Width of the texture
		m_info.height,        // Height of the texture
		m_info.depth,         // Depth of the texture
		m_info.mipLevels + 1, // Number of miplevels (includes level 0)
		textureUsage,         // Usage pattern
		texelFormat,          // Texel format
		memoryPool,           // Memory pool (VRAM / RAM)
		&volumeTexture,       // Resulting texture object
		nullptr)))            // Reserved (Create texture from system memory in Vista)
	{
		m_texture = volumeTexture;
		return true;
	}
	return false;
}


bool Texture::copyPixelsToTexture()
{
	if(m_pixels == nullptr)
	{
		// Nothing to copy
		return false;
	}

	D3DSURFACE_DESC    surfaceInfo;
	const u32          bytesPerPixel(getBytesPerPixel());
	const u8*          mipStart(m_pixels);

	mem::zero8(&surfaceInfo, sizeof(surfaceInfo));

	IDirect3DTexture9* texture(static_cast<IDirect3DTexture9*>(m_texture));

	// NOTE: Level count includes mip level 0 !!
	for(u32 i = 0; i < texture->GetLevelCount(); ++i)
	{
		// Get mipmap level i
		IDirect3DSurface9* mipLevelSurface(nullptr);
		if(FAILED(texture->GetSurfaceLevel(i, &mipLevelSurface)))
		{
			TT_PANIC("Failed to get surface level %d from texture.", i);
			return false;
		}
		
		checkD3DSucceeded( mipLevelSurface->GetDesc(&surfaceInfo) );
		
		D3DLOCKED_RECT lockedRect;
		checkD3DSucceeded( mipLevelSurface->LockRect(&lockedRect, nullptr, D3DLOCK_NOSYSLOCK) );
		
		const mem::size_type mipmapSize(surfaceInfo.Width * surfaceInfo.Height * bytesPerPixel);
		const u32 srcPitch = surfaceInfo.Width * bytesPerPixel;
		
		// Copy pixel data into current mipmap level, respecting pitch requirements
		mem::copyWithPitch(lockedRect.pBits, mipStart, srcPitch, lockedRect.Pitch, surfaceInfo.Height);
		
		mipStart += mipmapSize;
		
		checkD3DSucceeded( mipLevelSurface->UnlockRect() );
		mipLevelSurface->Release();
	}

	// Free image data
	if (m_info.paintable == false)
	{
		mem::free(m_pixels);
		m_pixels = nullptr;
	}

	// FIXME: Shouls unregister this as selected from correct channel, also if deleted
	MultiTexture::resetActiveTexture(0);

	return true;
}


bool Texture::copyPixelsToVolumeTexture()
{
	if(m_pixels == nullptr)
	{
		// Nothing to copy
		return false;
	}

	const u32 bytesPerPixel(getBytesPerPixel());

	IDirect3DVolumeTexture9* volumeTexture(static_cast<IDirect3DVolumeTexture9*>(m_texture));

	D3DLOCKED_BOX lockedBox;
	if (checkD3DSucceeded(volumeTexture->LockBox(0, &lockedBox, nullptr, D3DLOCK_NOSYSLOCK)) == false)
	{
		return false;
	}

	u8* dest = static_cast<u8*>(lockedBox.pBits);
	const u8* source = m_pixels;

	for (u32 slice = 0; slice < m_info.depth; ++slice)
	{
		// Copy single slice to destination
		const u32 srcPitch = bytesPerPixel * m_info.width;
		mem::copyWithPitch(dest, source, srcPitch, lockedBox.RowPitch, m_info.height);

		source += srcPitch * m_info.height;
		dest   += lockedBox.SlicePitch;
	}

	if (checkD3DSucceeded(volumeTexture->UnlockBox(0)) == false)
	{
		return false;
	}

	// Free image data
	mem::free(m_pixels);
	m_pixels = nullptr;

	// FIXME: Shouls unregister this as selected from correct channel, also if deleted
	MultiTexture::resetActiveTexture(0);

	return true;
}


bool Texture::updateD3DTextureState(u32 p_channel)
{
	IDirect3DDevice9* device(getRenderDevice(true));
	HRESULT hr;

	if(hasAddressModeChanged(ms_samplerState[p_channel], ADDRESS_U_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_ADDRESSU,
			getD3DAddressMode(getAddressMode(ADDRESS_U_MASK, 0))) );
	}

	if(hasAddressModeChanged(ms_samplerState[p_channel], ADDRESS_V_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_ADDRESSV, getD3DAddressMode(
			getAddressMode(ADDRESS_V_MASK, ADDRESS_V_SHIFT))) );
	}

	if(hasAddressModeChanged(ms_samplerState[p_channel], ADDRESS_W_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_ADDRESSW, getD3DAddressMode(
			getAddressMode(ADDRESS_W_MASK, ADDRESS_W_SHIFT))) );
	}

	if(hasFilterModeChanged(ms_samplerState[p_channel], FILTER_MIN_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_MINFILTER, getD3DFilterMode(
			getFilterMode(FILTER_MIN_MASK, 0))) );
	}

	if(hasFilterModeChanged(ms_samplerState[p_channel], FILTER_MAG_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_MAGFILTER, getD3DFilterMode(
			getFilterMode(FILTER_MAG_MASK, FILTER_MAG_SHIFT))) );
	}

	if(hasFilterModeChanged(ms_samplerState[p_channel], FILTER_MIP_MASK))
	{
		V( device->SetSamplerState(p_channel, D3DSAMP_MIPFILTER, getD3DFilterMode(
			getFilterMode(FILTER_MIP_MASK, FILTER_MIP_SHIFT))) );
	}

	// In sync again
	ms_samplerState[p_channel] = m_state;

	return true;
}


void Texture::setMipmapLodBias(real p_bias, u32 p_channel)
{
	IDirect3DDevice9* device = getRenderDevice();
	if(device == nullptr) return;

	device->SetSamplerState(p_channel, D3DSAMP_MIPMAPLODBIAS, *reinterpret_cast<DWORD*>(&p_bias));
}


void Texture::checkDimensions()
{
#if !defined(TT_BUILD_FINAL)
	std::string filenameInfo;

	if(m_filename.empty() == false)
	{
		filenameInfo = "Filename: " + m_filename;
	}
	TextureBase::checkDimensions(filenameInfo);

#endif // #if !defined(TT_BUILD_FINAL)
}


// Namespace end
}
}
}
