#if !defined(INC_TT_ENGINE_RENDERER_COLORRGB_H)
#define INC_TT_ENGINE_RENDERER_COLORRGB_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace renderer {

struct ColorRGB
{
	u8 r;
	u8 g;
	u8 b;
	
	static const ColorRGB white;
	static const ColorRGB black;
	static const ColorRGB red;
	static const ColorRGB green;
	static const ColorRGB blue;
	static const ColorRGB yellow;  // red + green
	static const ColorRGB magenta; // red + blue
	static const ColorRGB cyan;    // blue + green
	static const ColorRGB gray;
	static const ColorRGB darkgray;
	static const ColorRGB orange;
	
	
	ColorRGB()
	:
	r(255),
	g(255),
	b(255)
	{}
	
	ColorRGB(u8 p_r, u8 p_g, u8 p_b)
	:
	r(p_r),
	g(p_g),
	b(p_b)
	{}
	
	inline bool operator==(const ColorRGB& p_rhs) const
	{
		return r == p_rhs.r &&
		       g == p_rhs.g &&
		       b == p_rhs.b;
	}
	
	inline bool operator!=(const ColorRGB& p_rhs) const
	{ 
		return operator==(p_rhs) == false;
	}
	
	/*! \brief mix performs a linear interpolation between p_a and p_b.
	    \return p_a * ( 1 - p_i ) + p_b * p_i. */
	static ColorRGB mix(const ColorRGB& p_a, const ColorRGB& p_b, real p_i);
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_RENDERER_COLORRGB_H
