#ifndef NO_WIDESTRINGSTREAMS

#include <tt/loc/LocStringFormatter.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace loc {


//------------------------------------------------------------------------------
// Public member functions

LocStringFormatter::LocStringFormatter(LocStr*            p_locSheet,
                                       const std::string& p_localizationID,
                                       bool               p_numberedParams)
:
str::StringFormatter(p_locSheet->getString(p_localizationID), p_numberedParams)
{
	TT_ASSERTMSG(
		p_locSheet->getString(p_localizationID) != p_locSheet->getErrorString(),
		"Localization ID '%s' is not present in localization sheet '%s'.",
		p_localizationID.c_str(),
		p_locSheet->getFileName().c_str());
}


LocStringFormatter::~LocStringFormatter()
{
}

// Namespace end
}
}

#endif  // !defined(NO_WIDESTRINGSTREAMS)
