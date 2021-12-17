#if !defined(INC_TT_CODE_STATEID_H)
#define INC_TT_CODE_STATEID_H


#include <tt/code/Identifier.h>


namespace tt {
namespace code {

enum StateIDDummy { };

/*! \brief Type-safe state identifier. */
typedef Identifier<StateIDDummy> StateID;

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_STATEID_H)
