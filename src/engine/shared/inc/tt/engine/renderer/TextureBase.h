#if !defined(INC_TT_ENGINE_RENDERER_TEXTUREBASE_H)
#define INC_TT_ENGINE_RENDERER_TEXTUREBASE_H

#include <tt/platform/tt_types.h>
#include <tt/compression/image.h>
#include <tt/engine/EngineID.h>
#include <tt/engine/file/TextureHeader.h>
#include <tt/engine/renderer/enums.h>
#include <tt/fs/types.h>
#include <tt/math/Point2.h>


namespace tt {
namespace engine {
namespace renderer {


enum TextureType
{
	Type_Texture,
	Type_VolumeTexture,
	Type_CubeTexture
};


enum TextureUsage
{
	Usage_Normal,        // Regular Texture
	Usage_Text,          // If going to write to it using TexturePainter
	Usage_RenderTarget,  // Render target
	Usage_MipVisualization
};


// Texture Sampler State
struct TextureState
{
	u8  addressMode;
	u8  filterMode;

	TextureState() : addressMode(0), filterMode(0) { }
};

inline bool operator==(const TextureState& p_lhs, const TextureState& p_rhs)
{
	return (p_lhs.addressMode == p_rhs.addressMode && p_lhs.filterMode == p_rhs.filterMode);
}

inline bool operator!=(const TextureState& p_lhs, const TextureState& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}

// Pack address and filter modes in bitset
// 2 bits per setting (4 variations)
static const u32 ADDRESS_U_MASK   = 0x03;
static const u32 ADDRESS_V_MASK   = 0x0C;
static const u32 ADDRESS_W_MASK   = 0x30;
static const u32 ADDRESS_UVW_MASK = ADDRESS_U_MASK|ADDRESS_V_MASK|ADDRESS_W_MASK;
static const u32 ADDRESS_V_SHIFT  = 2;
static const u32 ADDRESS_W_SHIFT  = 4;

static const u32 FILTER_MIN_MASK  = 0x03;
static const u32 FILTER_MAG_MASK  = 0x0C;
static const u32 FILTER_MIP_MASK  = 0x30;
static const u32 FILTER_MAG_SHIFT = 2;
static const u32 FILTER_MIP_SHIFT = 4;


struct TextureBaseInfo
{
	u16          width;
	u16          height;
	u16          depth;
	u16          mipLevels;
	ImageFormat  format;
	TextureType  type;
	TextureUsage usage;
	bool         premultiplied;
	bool         paintable;

	TextureBaseInfo()
	:
	width(0),
	height(0),
	depth(1),
	mipLevels(0),
	format(ImageFormat_RGBA8),
	type(Type_Texture),
	usage(Usage_Normal),
	premultiplied(false),
	paintable(false)
	{ }
};


/*! \brief Base class for engine textures, contains common functionality and data */
class TextureBase
{
public:
	explicit TextureBase(const EngineID& p_id, u32 p_flags = 0);
	explicit TextureBase(const TextureBaseInfo& p_info);
	virtual ~TextureBase();

	inline EngineID getEngineID() const {return m_id;}

	/*! \brief Retrieve the width of the current texture */
	s32 getWidth()  const { return m_info.width; }
	
	/*! \brief Retrieve the height of the current texture */
	s32 getHeight() const { return m_info.height; }

	/*! \brief Get texture information */
	const TextureBaseInfo& getInfo() const { return m_info; }

	/*! \brief Load pixels from ETX file */
	bool loadPixelData(const fs::FilePtr& p_file, u32 p_pngTransforms = 0, bool p_decompress = true);

	/*! \brief Load pixels from PNG file */
	bool loadFromPNG(const fs::FilePtr& p_file, u32 p_pngTransforms = 0);

	/*! \ brief Load platform specific pixel data */
	virtual bool loadPlatformData(const fs::FilePtr& p_file);

	void decompressPixelData(file::CompressionType p_type);

	void createMipVisualizationTexture();

	// Setting Address Mode
	void setAddressMode(AddressMode p_u, AddressMode p_v, AddressMode p_w = AddressMode_Clamp);
	void setAddressModeU(AddressMode p_mode);
	void setAddressModeV(AddressMode p_mode);
	void setAddressModeW(AddressMode p_mode);


	// Setting Filtering Mode
	void setFilterMode(FilterMode p_minFilter,
	                   FilterMode p_magFilter,
	                   FilterMode p_mipFilter = FilterMode_None);
	void setMinificationFilter (FilterMode p_filter);
	void setMagnificationFilter(FilterMode p_filter);
	void setMipmapFilter       (FilterMode p_filter);

	u32 getBytesPerPixel() const;

	/*! \brief get size in bytes in memory */
	s32 getMemSize() const;

	inline bool isPremultiplied() { return m_info.premultiplied; }

	inline void setPaintable(bool p_paintable) { m_info.paintable = p_paintable; }
	inline bool isPaintable() const            { return m_info.paintable; }

	static inline bool isMipmapLodBiasForced() { return ms_forceMipmapLodBias; }

	// Hardware limitations

	/*! \brief Calculates the minimal texture dimensions for a given dimension.
	    \param p_dimensions The requested minimal texture size.
	    \return The smallest texture size that will fit p_dimensions.*/
	static math::Point2 getMinimalDimensions(const math::Point2& p_dimensions);

	static inline math::Point2 getMinimalDimensions(s32 p_x, s32 p_y) { return getMinimalDimensions(math::Point2(p_x, p_y)); }
	
	static inline void setEnforceHardwareDimensions(bool p_enforce) { ms_enforceHardwareDimensions = p_enforce; }
	static inline bool doesEnforceHardwareDimensions() { return ms_enforceHardwareDimensions; }
	
	static inline void setForcedPowerOfTwoOnly(bool p_forcedPow2Only)    { ms_forcedPow2Only = p_forcedPow2Only; }
	static inline bool getForcedPowerOfTwoOnly()                         { return ms_forcedPow2Only; }
	static inline bool usePowerOfTwo()                                   { return ms_deviceSupportsNonPow2 == false || ms_forcedPow2Only; }
	static inline void setDeviceSupportsNonPowerOfTwo(bool p_useNonPow2) { ms_deviceSupportsNonPow2 = p_useNonPow2; }

	void checkDimensions(const std::string& p_info = std::string());

protected:
	inline bool hasAddressModeChanged(TextureState p_hardware, u32 p_mask)
	{
		return (m_state.addressMode & p_mask) != (p_hardware.addressMode & p_mask);
	}
	inline bool hasFilterModeChanged(TextureState p_hardware, u32 p_mask)
	{
		return (m_state.filterMode & p_mask) != (p_hardware.filterMode & p_mask);
	}

	inline AddressMode getAddressMode(u32 p_mask, u32 p_shift)
	{
		return static_cast<AddressMode>((m_state.addressMode & p_mask) >> p_shift);
	}

	inline FilterMode getFilterMode(u32 p_mask, u32 p_shift)
	{
		return static_cast<FilterMode>((m_state.filterMode & p_mask) >> p_shift);
	}

	TextureBaseInfo  m_info;
	TextureState     m_state;
	TextureState     m_hwState;
	u8*              m_pixels;
	u8*              m_compressedPixels;
	u32              m_imageSize;
	u32              m_compressedSize;

	// Mipmap LOD bias forced (override normal sampler settings)
	static bool ms_forceMipmapLodBias;

private:
	EngineID m_id;

	static bool ms_enforceHardwareDimensions; // Whether the code should complain about incorrect texture dimensions
	static bool ms_deviceSupportsNonPow2;     // Set by renderer based on if device supports non power or 2.
	static bool ms_forcedPow2Only;            // Set by client code to force power of 2 textures.
};


// Namespace end
}
}
}

#endif // INC_TT_ENGINE_RENDERER_TEXTUREBASE_H
