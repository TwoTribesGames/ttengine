#if !defined(INC_TT_INPUT_TRIGGER_H)
#define INC_TT_INPUT_TRIGGER_H


#include <tt/code/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace input {

struct Trigger
{
public:
	Trigger();
	
	
	real value;
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
};

// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_TRIGGER_H)
