#include <tt/math/math.h>


namespace tt {
namespace math {

u32 countLeadingZeros(u32 p_x)
{
	// Use binary search to find where bit 0 end.
	u32 y = p_x >> 16;
	u32 n = 32;
	
	if (y != 0)
	{
		n -= 16;
		p_x = y;
	}
	y = p_x >> 8;
	if (y != 0)
	{
		n -= 8;
		p_x = y;
	}
	y = p_x >> 4;
	if (y != 0)
	{
		n -= 4;
		p_x = y;
	}
	y = p_x >> 2;
	if (y != 0)
	{
		n -= 2;
		p_x = y;
	}
	y = p_x >> 1;
	if (y != 0)
	{
		// p_x == 0b10 or 0b11 -> n -= 2
		n -= 2;
	}
	else
	{
		// p_x == 0b00 or 0b01 -> n -= p_x
		n -= p_x;
	}
	
	return n;
}

// namespace
}
}
