#include <tt/input/Touch.h>


namespace tt {
namespace input {


const Touch::ID Touch::invalidID = 0;


Touch::Touch()
:
Pointer(),
status(),
id(invalidID)
{
}
	
	
Touch::Touch(ID p_id, const tt::math::Point2& p_location)
:
Pointer(p_location),
status(true),
id(p_id)
{
}


void Touch::updateLocation(const tt::math::Point2& p_location)
{
	Pointer::updateLocation(p_location);
	status.update(true);
}


void Touch::updateNoTouch()
{
	if (status.down == false)
	{
		Pointer::updateLocation(tt::math::Point2(-1, -1), false);
	}
	status.update(false);
}


// Namespace end
}
}
