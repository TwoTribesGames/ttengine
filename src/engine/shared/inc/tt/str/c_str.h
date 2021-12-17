#ifndef INC_TT_STR_C_STR_H
#define INC_TT_STR_C_STR_H


#include <cstddef>


namespace tt {
namespace str {

/*! \brief Indicates whether two strings are equal.
           (With specified max number of characters, to guard against buffer overruns). */
bool equal(const char* p_a, const char* p_b, std::size_t p_max);

// Namespace end
}
}


#endif  // !defined(INC_TT_STR_C_STR_H)
