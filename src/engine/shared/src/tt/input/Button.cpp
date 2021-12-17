#include <tt/code/bufferutils.h>
#include <tt/input/Button.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace input {


Button::Button(bool p_down)
:
down(p_down),
pressed(p_down),
released(false),
blockedUntilReleased(false)
{
}


void Button::update(bool p_down)
{
	if (blockedUntilReleased)
	{
		if (p_down)
		{
			return;
		}
		else
		{
			blockedUntilReleased = false;
		}
	}
	
	pressed  = ((down == false) && p_down);
	released = (down            && (p_down == false));
	down     = p_down;
}


void Button::reset()
{
	pressed              = false;
	released             = false;
	down                 = false;
	blockedUntilReleased = false;
}


void Button::resetAndBlockUntilReleased()
{
	pressed              = false;
	released             = false;
	down                 = false;
	blockedUntilReleased = true;
}


enum ButtonFlag
{
	ButtonFlag_Pressed              = 0x1 << 0,
	ButtonFlag_Released             = 0x1 << 1,
	ButtonFlag_Down                 = 0x1 << 2,
	ButtonFlag_BlockedUntilReleased = 0x1 << 3
};


void Button::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);

	u8 buttonState(0);
	if(pressed)              buttonState |= ButtonFlag_Pressed;
	if(released)             buttonState |= ButtonFlag_Released;
	if(down)                 buttonState |= ButtonFlag_Down;
	if(blockedUntilReleased) buttonState |= ButtonFlag_BlockedUntilReleased;

	code::bufferutils::put<u8>(buttonState, p_context);
}


void Button::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);

	u8 buttonState = code::bufferutils::get<u8>(p_context);
	
	pressed              = ((buttonState & ButtonFlag_Pressed             ) == ButtonFlag_Pressed             );
	released             = ((buttonState & ButtonFlag_Released            ) == ButtonFlag_Released            );
	down                 = ((buttonState & ButtonFlag_Down                ) == ButtonFlag_Down                );
	blockedUntilReleased = ((buttonState & ButtonFlag_BlockedUntilReleased) == ButtonFlag_BlockedUntilReleased);
}

// Namespace end
}
}
