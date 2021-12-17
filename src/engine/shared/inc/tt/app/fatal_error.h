#if !defined(INC_TT_APP_FATAL_ERROR_H)
#define INC_TT_APP_FATAL_ERROR_H


#include <string>


namespace tt {
namespace app {

/*! \brief Reports a non-recoverable (fatal) error to the user (even in final builds) and exits the application. */
void reportFatalError(const std::string& p_message);

// Namespace end
}
}


#endif  // !defined(INC_TT_APP_FATAL_ERROR_H)
