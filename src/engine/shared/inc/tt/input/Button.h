#if !defined(INC_TT_INPUT_BUTTON_H)
#define INC_TT_INPUT_BUTTON_H

#include <tt/code/fwd.h>

namespace tt {
namespace input {

struct Button
{
	explicit Button(bool p_down = false);
	
	void update(bool p_down);
	
	inline bool hasChanged() const { return pressed || down; }
	void reset();
	void resetAndBlockUntilReleased();
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	bool down;     //!< Button is held down.
	bool pressed;  //!< Button has recently been pressed.
	bool released; //!< Button has recently been released.
	
private:
	bool blockedUntilReleased;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_BUTTON_H)
