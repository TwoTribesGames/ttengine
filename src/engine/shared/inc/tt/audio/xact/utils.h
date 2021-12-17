#ifndef INC_TT_AUDIO_XACT_UTILS_H
#define INC_TT_AUDIO_XACT_UTILS_H


#include <tt/math/Random.h>


namespace tt {
namespace audio {
namespace xact {

/*! \brief Returns the random number generator that the XACT clone implementation should use for its random numbers. */
math::Random& getXactRandom();

// Namespace end
}
}
}


#endif  // !defined(INC_TT_AUDIO_XACT_UTILS_H)
