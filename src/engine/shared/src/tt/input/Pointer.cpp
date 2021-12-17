#include <tt/code/bufferutils.h>
#include <tt/input/Pointer.h>

namespace tt {
namespace input {

Pointer::Pointer()
:
Point2(0,0),
valid(false)
{
}


Pointer::Pointer(const tt::math::Point2& p_location)
:
Point2(p_location),
valid(true)
{
}


void Pointer::updateLocation(const tt::math::Point2& p_location, bool p_valid)
{
	setValues(p_location.x, p_location.y);
	valid = p_valid;
}


void Pointer::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	bu::put(x,     p_context); // s32
	bu::put(y,     p_context); // s32
	bu::put(valid, p_context); // bool
}


void Pointer::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	x     = bu::get<s32 >(p_context);
	y     = bu::get<s32 >(p_context);
	valid = bu::get<bool>(p_context);
}


// Namespace end
}
}
