#include <tt/engine/renderer/ColorRGB.h>
#include <tt/math/math.h>

namespace tt {
namespace engine {
namespace renderer {


const ColorRGB ColorRGB::white   (255, 255, 255);
const ColorRGB ColorRGB::black   (  0,   0,   0);
const ColorRGB ColorRGB::red     (255,   0,   0);
const ColorRGB ColorRGB::green   (  0, 255,   0);
const ColorRGB ColorRGB::blue    (  0,   0, 255);
const ColorRGB ColorRGB::yellow  (255, 255,   0); // red + green
const ColorRGB ColorRGB::magenta (255,   0, 255); // red + blue
const ColorRGB ColorRGB::cyan    (  0, 255, 255); // blue + green
const ColorRGB ColorRGB::gray    (128, 128, 128);
const ColorRGB ColorRGB::darkgray( 96,  96,  96);
const ColorRGB ColorRGB::orange  (255, 128,   0);


ColorRGB ColorRGB::mix(const ColorRGB& p_a, const ColorRGB& p_b, real p_i)
{
	const real oneMinusI = 1.0f - p_i;
	
	// Use s32 so we can clamp to u8 range.
	s32 newR = static_cast<s32>( (p_a.r * oneMinusI) + p_b.r * p_i );
	s32 newG = static_cast<s32>( (p_a.g * oneMinusI) + p_b.g * p_i );
	s32 newB = static_cast<s32>( (p_a.b * oneMinusI) + p_b.b * p_i );
	
	math::clamp(newR, s32(0), s32(255));
	math::clamp(newG, s32(0), s32(255));
	math::clamp(newB, s32(0), s32(255));
	
	return ColorRGB(static_cast<u8>(newR), static_cast<u8>(newG), static_cast<u8>(newB));
}

// Namespace end
}
}
}

