#include <tt/settings/mac.h>


namespace tt {
namespace settings {

std::string formatMacAddress(const u8* p_address)
{
	static const char* hex = "0123456789abcdef";
	std::string ret;
	ret.reserve(6 * 2 + 5);
	for (int i = 0; i < 6; ++i)
	{
		u8 b = p_address[i];
		ret.push_back(hex[b >> 4]);
		ret.push_back(hex[b & 0x0F]);
		if (i != 5)
		{
			ret.push_back('-');
		}
	}
	
	return ret;
}

// namespace end
}
}
