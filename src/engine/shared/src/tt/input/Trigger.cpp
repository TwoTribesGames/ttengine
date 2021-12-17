#include <tt/code/bufferutils.h>
#include <tt/input/Trigger.h>


namespace tt {
namespace input {

Trigger::Trigger()
:
value(0)
{
}


void Trigger::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);

	code::bufferutils::put(value, p_context);
}


void Trigger::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	value = code::bufferutils::get<real>(p_context);
}


// Namespace end
}
}
