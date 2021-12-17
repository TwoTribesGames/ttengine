#if !defined(INC_TT_ENGINE_RENDERER_BUFFERVTX_H)
#define INC_TT_ENGINE_RENDERER_BUFFERVTX_H

#include <tt/math/Vector2.h>
#include <tt/math/Vector3.h>
#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace engine {
namespace renderer {


class QuadBuffer;
class TriangleBuffer;
class TrianglestripBuffer;


/*! \brief Buffer vertex used in different TT primitive buffers (e.g. Quadbuffer, TrianglestripBuffer)
    \param UVSetCount the number of UV coordinate sets per vertex
    \note  PrimitiveCollectionBuffer uses assumptions  about the BufferVtxUV size difference per UVSetCount
    \note  The primitive buffers use overdraw magic which assumes variable member (in size) is declared last
    \note  The windows BufferVtx has swapped colors (RGBA = BGRA)
    \TODO: The RGBA type does not specify the swapped colors in its name */
template <s32 UVSetCount>
class BufferVtxUV
{
public:
	BufferVtxUV() {}
	
	/*! \brief Constructor which initializes 1 texture coordinate set */
	BufferVtxUV(const math::Vector3& p_pos, 
	            const ColorRGBA& p_col, 
	            const math::Vector2& p_tex1)
	:
	pos(p_pos),
	col(p_col)
	{ tex[0] = p_tex1; }
	
	~BufferVtxUV() { }
	
	inline void setPosition(const math::Vector3& p_pos)   { pos = p_pos;                  }
	inline void setPosition(real p_x, real p_y, real p_z) { pos.setValues(p_x, p_y, p_z); }
	inline void setPositionX(real p_x)                    { pos.x = p_x;                  }
	inline void setPositionY(real p_y)                    { pos.y = p_y;                  }
	inline void setPositionZ(real p_z)                    { pos.z = p_z;                  }
	
	inline void setColor(const ColorRGBA& p_color)        { col = p_color; }
	inline void setColor(u8 p_r, u8 p_g, u8 p_b, u8 p_a)  { col.setColor(p_r, p_g, p_b, p_a); }
	
	inline void setTexCoord(const math::Vector2& p_tex, s32 p_index = 0)
		{ TT_ASSERTMSG(p_index >= 0 && p_index < UVSetCount,
		  "texture coordinate index (%d) is out of bounds (max:%d)", p_index, UVSetCount);
		  tex[p_index] = p_tex; }
	inline void setTexCoord(real p_u, real p_v, s32 p_index = 0)
		{ setTexCoord(math::Vector2(p_u, p_v), p_index); }
		
	inline const math::Vector3& getPosition() const { return pos; }
	inline const ColorRGBA& getColor() const        { return col; }
	inline const math::Vector2& getTexCoord(s32 p_index = 0) const
		{ TT_ASSERTMSG(p_index >= 0 && p_index < UVSetCount,
		  "texture coordinate index (%d) is out of bounds (max:%d)", p_index, UVSetCount);
		  return tex[p_index]; }
	
private:
	math::Vector3 pos;
	ColorRGBA     col;
	math::Vector2 tex[UVSetCount];
	
	TT_STATIC_ASSERT(UVSetCount > 0);
	
	friend class QuadBuffer;
	friend class TriangleBuffer;
	friend class TrianglestripBuffer;
};

// client should create new types of BufferVtxUV<UVSetCount> when needed
typedef BufferVtxUV<1> BufferVtx; // type for backward compatibility


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_BUFFERVTX_H
