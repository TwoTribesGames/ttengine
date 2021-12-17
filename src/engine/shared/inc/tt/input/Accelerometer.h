#if !defined(INC_TT_INPUT_ACCELEROMETER_H)
#define INC_TT_INPUT_ACCELEROMETER_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

struct Accelerometer
{
public:
	Accelerometer();
	
	
	real x;
	real y;
	real z;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_ACCELEROMETER_H)
