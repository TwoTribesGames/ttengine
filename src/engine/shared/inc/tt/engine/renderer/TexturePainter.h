#if !defined(TT_ENGINE_RENDERER_TEXTUREPAINTER_H)
#define TT_ENGINE_RENDERER_TEXTUREPAINTER_H


#include <tt/engine/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace engine {
namespace renderer {

/* \brief RAII class for locking and unlocking a texture
          and for making changes to the pixel data. */
class TexturePainter
{
public:
	TexturePainter(const TexturePainter& p_rhs);
	
	/*! \brief Destroying a TexturePainter. This will automatically unlock the texture. */
	~TexturePainter();
	
	/*! \brief Set a pixel to the specified color.
	    \param p_x X position of the pixel to set.
	    \param p_y Y position of the pixel to set.
	    \param p_color Color to fill the pixel with.
	    \return Boolean indicating success (true) or failure (false). */
	bool setPixel(s32 p_x, s32 p_y, ColorRGBA p_color);
	
	/*! \brief Get the color of the specified pixel.
	    \param p_x X position of the pixel to get.
	    \param p_y Y position of the pixel to get.
	    \param p_color Reference to color to put result in.
	    \return Boolean indicating success (true) or failure (false). */
	bool getPixel(s32 p_x, s32 p_y, ColorRGBA& p_color) const;
	
	// FIXME: Is this OSX-only function used anywhere?
	//bool setSubImage(s32 p_x, s32 p_y, s32 p_width, s32 p_height, u8* p_bytes);
	
	/*! \brief Fills the texture with data from a memory buffer.
	    \param p_buffer Pointer to the buffer that contains the pixel data
		\param p_bytes  Number of bytes to write. */
	// FIXME: Is this OSX-only function used anywhere?
	//void fillFromBuffer(const void* p_buffer, s32 p_bytes);
	
	/*! \brief Clear texture to black (+zero alpha) */
	void clear();
	
	/*! \brief Clear texture to the specified color. */
	void clear(const ColorRGBA& p_color);
	
	/*! \brief Clear the texture to white */
	void clearToWhite();
	
	
	/*! \brief Get the width of the locked texture.
	    \return Width of the locked texture in pixels. */
	inline s32 getTextureWidth()  const { return static_cast<s32>(m_width);  }
	
	/*! \brief Get the width of the locked texture.
	    \return Height of the locked texture in pixels. */
	inline s32 getTextureHeight() const { return static_cast<s32>(m_height); }
	
	void setRegion(s32 p_width, s32 p_height, s32 p_offsetX, s32 p_offsetY, const u8* p_pixelData);
	
private:
	/*! \brief Creating a TexturePainter will automatically lock the texture.
	           TexturePainter should be created by calling lock in Texture. */
	explicit TexturePainter(Texture* p_texture);
	
	// No assignment
	TexturePainter& operator=(const TexturePainter&);
	
	
	Texture* m_texture;
	s32      m_width;
	s32      m_height;
	s32      m_bufferSize;
	u32      m_texelSize;
	u8*      m_pixelData;
	bool     m_pixelDataChanged;
	
	
	friend class Texture;
};
	
// Namespace end
}
}
}


#endif  // !defined(TT_ENGINE_RENDERER_TEXTUREPAINTER_H)
