#if !defined(INC_TT_ENGINE_RENDERER_QUADSPRITE_H)
#define INC_TT_ENGINE_RENDERER_QUADSPRITE_H


#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/Quad2D.h>
#include <tt/math/Matrix44.h>
#include <tt/math/Vector3.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {


class QuadSprite
{
public:
	enum Flag
	{
		Flag_NeedUpdate     = (1 << 0),  // Transform requires updating
		Flag_Visible        = (1 << 1),
		Flag_FadingIn       = (1 << 2),
		Flag_FadingOut      = (1 << 3),
		Flag_FlipHorizontal = (1 << 4),
		Flag_FlipVertical   = (1 << 5),
		Flag_NeedQuadUpdate = (1 << 6),  // Quad data requires updating
		Flag_Premultiply    = (1 << 7),
		Flag_WorldSpace     = (1 << 8)   // Do not flip Y coordinates on quadsprite
	};
	
	QuadSprite(const QuadSprite& p_rhs);
	
	/*! \brief Sets a specific flag.
	    \param flag - The flag to set */
	inline void setFlag(Flag p_flag)
	{
		TT_ASSERTMSG(p_flag != Flag_FlipHorizontal && p_flag != Flag_FlipVertical,
		             "QuadSprite error: do not set the flip flags, use the methods instead!");
		m_flags |= p_flag;
	}
	inline void setFlags(u32 p_flags)
	{
		TT_ASSERTMSG((p_flags & Flag_FlipHorizontal) == 0 && (p_flags & Flag_FlipVertical) == 0,
		             "QuadSprite error: do not set the flip flags, use the methods instead!");
		m_flags |= p_flags;
	}
	
	/*! \brief Resets a specific flag 
	    \param flag - The flag to reset */
	inline void resetFlag(Flag p_flag) { m_flags &= ~p_flag; }
	
	/*! \brief Checks if a flag is currently set
	    \param flag - The flag to check
	    \return True if the flag is set else false */
	inline bool checkFlag(Flag p_flag) const { return ((m_flags & p_flag) != 0); }
	
	inline void setPosition(const math::Vector2& p_pos)
	{
		m_position.x = p_pos.x;
		m_position.y = p_pos.y;
		setFlag(Flag_NeedUpdate);
	}
	inline void setPosition(const math::Vector3& p_pos)
	{
		m_position = p_pos;
		setFlag(Flag_NeedUpdate);
	}
	inline void setPosition(real p_x, real p_y, real p_z)
	{
		m_position.setValues(p_x, p_y, p_z);
		setFlag(Flag_NeedUpdate);
	}
	inline void setPositionX(real p_x)
	{
		m_position.x = p_x;
		setFlag(Flag_NeedUpdate);
	}
	inline void setPositionY(real p_y)
	{
		m_position.y = p_y;
		setFlag(Flag_NeedUpdate);
	}
	inline void setPositionZ(real p_z)
	{
		m_position.z = p_z;
		setFlag(Flag_NeedUpdate);
	}
	
	inline const math::Vector3& getPosition() const { return m_position; }
	
	void setTexture(const TexturePtr& p_texture);
	const TexturePtr& getTexture() const;
	
	void setColor(const ColorRGBA& p_color);
	void setColor(const ColorRGB&  p_color);
	void setOpacity(u8 p_opacity);
	
	inline void setWidth (real p_width)
	{
		m_width = p_width / (Quad2D::quadSize * 2);
		setFlag(Flag_NeedUpdate);
	}
	inline void setHeight(real p_height)
	{
		m_height = p_height / (Quad2D::quadSize * 2);
		setFlag(Flag_NeedUpdate);
	}
	
	inline real getWidth()  const { return m_width  * (Quad2D::quadSize * 2); }
	inline real getHeight() const { return m_height * (Quad2D::quadSize * 2); }
	
	inline void setRotation(real p_angleRad)
	{
		m_rotation = p_angleRad;
		setFlag(Flag_NeedUpdate);
	}
	inline real getRotation() const { return m_rotation; }
	
	inline void setScale(real p_scale)
	{
		m_scale = p_scale;
		setFlag(Flag_NeedUpdate);
	}
	inline real getScale() const { return m_scale; }
	
	inline void setRotationOffset(const math::Vector3& p_offset) // Sets the rotation offset
	{
		m_offset = p_offset;
		setFlag(Flag_NeedUpdate);
	}
	inline const math::Vector3& getRotationOffset() { return m_offset; }
	
	inline void setTransform(const math::Matrix44& p_transform)
	{
		m_transform = p_transform;
	}
	inline const math::Matrix44& getTransform() const { return m_transform; }
	
	void fadeIn(real p_time, u8 p_opacity = 255);
	void fadeOut(real p_time);
	
	/*! \brief flipping quad
	    \param flip - Do or do not flip
	    \return nothing */
	void setFlippedHorizontal(bool p_flip);
	void setFlippedVertical(bool p_flip);
	
	/*! \brief check if quad is flipped
	    \return true if flipped, false otherwise */
	inline bool isFlippedHorizontal() const { return checkFlag(Flag_FlipHorizontal); }
	inline bool isFlippedVertical()   const { return checkFlag(Flag_FlipVertical);   }
	
	bool update();
	bool render();
	
	void setFrame(s32 p_frameWidth, s32 p_frameHeight);
	inline const MaterialPtr& getMaterial() const { return m_material; }
	
	// QuadSprite creation (Preferred method)
	static QuadSpritePtr createQuad(real p_width, real p_height,   u32 p_vertexType);
	static QuadSpritePtr createQuad(const TexturePtr& p_texture,   u32 p_vertexType);
	//static QuadSpritePtr createQuad(const std::string& p_filename, u32 p_vertexType);
	
	// Backwards compatibility
	static QuadSpritePtr createQuad(real p_width, real p_height, const ColorRGBA& p_color);
	static QuadSpritePtr createQuad(const TexturePtr& p_texture, const ColorRGBA& p_color);
	static QuadSpritePtr createQuad(const TexturePtr& p_texture);
	
private:
	QuadSprite(real p_width, real p_height, u32 p_vertexType);
	QuadSprite(const TexturePtr& p_texture, u32 p_vertexType, const ColorRGBA& p_color = ColorRGB::white);
	
	// Internal flipping
	void updateTextureCoordsForFlip();
	
	
	math::Matrix44 m_transform;
	math::Vector3  m_position;
	math::Vector3  m_offset;
	real           m_rotation;
	real           m_scale;
	real           m_width;
	real           m_height;
	
	MaterialPtr m_material;
	
	u16 m_flags;
	
	u8   m_maxAlpha;
	real m_fadeStart;
	real m_fadeEnd;
	
	ColorRGBA m_color;
	Quad2D    m_quad;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_QUADSPRITE_H)
