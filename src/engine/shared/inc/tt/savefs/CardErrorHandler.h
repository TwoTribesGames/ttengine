#ifndef INC_TT_SAVEFS_CARDERRORHANDLER_H
#define INC_TT_SAVEFS_CARDERRORHANDLER_H

#include <tt/savefs/CardError.h>

namespace tt {
namespace savefs {

class CardErrorHandler
{
public:
	virtual void handleError(CardError p_error) = 0;
	virtual bool handleRemoval() = 0;
	
};

// Namespace end
}
}


#endif  // !defined(INC_TT_SAVEFS_CARDERRORHANDLER_H)
