#include <tt/snd/Buffer.h>

namespace tt {
namespace snd {


void Buffer::deleteBuffer(Buffer* p_buffer)
{
	delete p_buffer;
}


// namespace end
}
}
