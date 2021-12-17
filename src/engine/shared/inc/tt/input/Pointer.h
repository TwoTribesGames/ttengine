#if !defined(INC_TT_INPUT_POINTER_H)
#define INC_TT_INPUT_POINTER_H

#include <tt/code/fwd.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace input {

struct Pointer : public tt::math::Point2
{
public:
	Pointer();
	explicit Pointer(const tt::math::Point2& p_location);
	
	bool valid;
	
	inline void reset() { setValues(-1,-1); valid = false; }
	
	void updateLocation(const tt::math::Point2& p_location, bool p_valid = true);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_POINTER_H)
