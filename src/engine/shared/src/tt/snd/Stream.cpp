#include <tt/snd/Stream.h>

namespace tt {
namespace snd {


void Stream::deleteStream(Stream* p_stream)
{
	delete p_stream;
}


// namespace end
}
}
