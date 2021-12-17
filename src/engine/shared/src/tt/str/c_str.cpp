#include <tt/str/c_str.h>


namespace tt {
namespace str {

bool equal(const char* p_a, const char* p_b, std::size_t p_max)
{
	for (size_t index = 0; index < p_max; ++p_a, ++p_b, ++index)
	{
		if (*p_a != *p_b)
		{
			return false;
		}
		
		if (*p_a == 0 || *p_b == 0)
		{
			break;
		}
	}
	
	return true;
}


// Namespace end
}
}
