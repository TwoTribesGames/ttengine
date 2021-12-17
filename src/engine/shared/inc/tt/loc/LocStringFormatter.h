#ifndef NO_WIDESTRINGSTREAMS

#ifndef INC_TT_LOC_LOC_STRING_FORMATTER_H
#define INC_TT_LOC_LOC_STRING_FORMATTER_H

#include <tt/loc/LocStr.h>
#include <tt/str/StringFormatter.h>


namespace tt {
namespace loc {

/*! \brief Provides basic stream-based string formatting based on localization strings. */
class LocStringFormatter : public str::StringFormatter
{
public:
	LocStringFormatter(LocStr*            p_locSheet,
	                   const std::string& p_localizationID,
	                   bool               p_numberedParams = true);
	virtual ~LocStringFormatter();
	
private:
	// No copying or assigning
	LocStringFormatter(const LocStringFormatter&);
	const LocStringFormatter& operator=(const LocStringFormatter&);
};

// Namespace end
}
}

#endif  // !defined(INC_TT_LOC_LOC_STRING_FORMATTER_H)
#endif
