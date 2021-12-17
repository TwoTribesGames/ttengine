#if !defined(INC_TT_ENGINE_RENDERER_COLORRGBA_H)
#define INC_TT_ENGINE_RENDERER_COLORRGBA_H

#include <ostream>

#include <tt/math/Vector4.h>
#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/ColorRGB.h>


namespace tt {
namespace engine {
namespace renderer {

struct ColorRGBA
{
	u8 r;
	u8 g;
	u8 b;
	u8 a;
	
	
	ColorRGBA()
	:
	r(255),
	g(255),
	b(255),
	a(255)
	{}
	
	ColorRGBA(u8 p_r, u8 p_g, u8 p_b, u8 p_a = 255)
	:
	r(p_r),
	g(p_g),
	b(p_b),
	a(p_a)
	{}
	
	ColorRGBA(const ColorRGB& p_rgb, u8 p_a = 255)
	:
	r(p_rgb.r),
	g(p_rgb.g),
	b(p_rgb.b),
	a(p_a)
	{}
	
	inline bool operator==(const ColorRGBA& p_rhs) const
	{
		return r == p_rhs.r &&
		       g == p_rhs.g &&
		       b == p_rhs.b &&
		       a == p_rhs.a;
	}
	
	inline bool operator!=(const ColorRGBA& p_rhs) const
	{
		return operator==(p_rhs) == false; 
	}
	
	inline void setColor(u8 p_r, u8 p_g, u8 p_b, u8 p_a = 255)
	{
		r = p_r;
		g = p_g;
		b = p_b;
		a = p_a;
	}
	
	inline ColorRGB rgb() const
	{
		return ColorRGB(r, g, b);
	}
	
	/*! \brief Convert color from 0-255 u8 to floating point 0.0-1.0 representation */
	inline math::Vector4 normalized() const
	{
		const float m = 1.0f / 255.0f;
		return math::Vector4(r * m, g * m, b * m, a * m);
	}
	
	/*! \brief Create color with 0-255 u8 from floating point 0.0-1.0 representation */
	static inline ColorRGBA createFromNormalized(tt::math::Vector4 p_normalizedColor)
	{
		p_normalizedColor *= 255.0f;
		
		tt::math::clamp(p_normalizedColor.x, 0.0f, 255.0f);
		tt::math::clamp(p_normalizedColor.y, 0.0f, 255.0f);
		tt::math::clamp(p_normalizedColor.z, 0.0f, 255.0f);
		tt::math::clamp(p_normalizedColor.w, 0.0f, 255.0f);
		
		return ColorRGBA(static_cast<u8>(p_normalizedColor.x),
		                 static_cast<u8>(p_normalizedColor.y),
		                 static_cast<u8>(p_normalizedColor.z),
		                 static_cast<u8>(p_normalizedColor.w));
	}
	
	inline void premultiply()
	{
		const float alpha = a * (1.0f / 255.0f);
		r = static_cast<u8>(alpha * r);
		g = static_cast<u8>(alpha * g);
		b = static_cast<u8>(alpha * b);
	}
};


inline std::ostream& operator<<(std::ostream& s, const ColorRGBA& p_col)
{
	return s <<  "(r: " << s32(p_col.r) << ", g: " << s32(p_col.g) << ", b: " << s32(p_col.b)
	         << ", a: " << s32(p_col.a)  << ")";
}


// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_RENDERER_COLORRGBA_H)
