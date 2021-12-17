#include <tt/audio/xact/utils.h>


namespace tt {
namespace audio {
namespace xact {

math::Random& getXactRandom()
{
	static math::Random xactRNG;
	return xactRNG;
}

// Namespace end
}
}
}
