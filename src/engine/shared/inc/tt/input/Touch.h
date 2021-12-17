#if !defined(INC_TT_INPUT_TOUCH_H)
#define INC_TT_INPUT_TOUCH_H

#include <tt/input/Pointer.h>
#include <tt/input/Button.h>

namespace tt {
namespace input {

struct Touch : public Pointer
{
public:
#if defined(TT_PLATFORM_OSX_IPHONE)
	typedef void* ID;
#else
	typedef u32 ID;
#endif
	
	static const ID invalidID;
	static inline bool isValidID(ID p_id) { return p_id != 0; }
	
	Touch();
	Touch(ID p_id, const tt::math::Point2& p_location);
	
	Button status;
	ID id;
	
	inline void reset() { Pointer::reset(); status.reset(); }
	
	void updateLocation(const tt::math::Point2& p_location);
	void updateNoTouch();
	inline void updateRelease() { status.update(false); }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_TOUCH_H)
