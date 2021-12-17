#include <sstream>
#include <iomanip>

#include <tt/http/helpers.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>
#include <tt/str/str.h>


namespace tt {
namespace http {

inline static char charFromHex(const std::string& p_hex)
{
	TT_ASSERT(p_hex.length() == 2);
	std::istringstream buff(p_hex);
	s32 out = 0;
	buff >> std::hex >> out;
	
	return static_cast<char>(out);
}


inline static std::string char2hex(char p_dec)
{
	s32 dig1 = static_cast<s32>((p_dec & 0xF0) >> 4);
	s32 dig2 = static_cast<s32>(p_dec & 0x0F);
	if ( 0 <= dig1 && dig1 <=  9) dig1 += '0';
	if (10 <= dig1 && dig1 <= 15) dig1 += 'A' - 10;
	if ( 0 <= dig2 && dig2 <=  9) dig2 += '0';
	if (10 <= dig2 && dig2 <= 15) dig2 += 'A' - 10;
	
	std::string r(1, static_cast<std::string::value_type>(dig1));
	r += static_cast<std::string::value_type>(dig2);
	return r;
}



std::string parseDataAsString(const u8* p_data, std::size_t p_dataLength)
{
	if (p_dataLength == 0)
	{
		return std::string();
	}
	
	std::string result;
	result.resize(static_cast<std::string::size_type>(p_dataLength));
	for (std::size_t i = 0; i < p_dataLength; ++i, ++p_data)
	{
		result[i] = static_cast<std::string::value_type>(*p_data);
	}
	
	return result;
}


std::string urlDecode(const std::string& p_encodedData)
{
	std::string outUrl(p_encodedData);
	std::string::size_type pos;
	while (std::string::npos != (pos = outUrl.find('%')))
	{
		outUrl.replace(pos, 3, 1, charFromHex(outUrl.substr(pos + 1, 2)));
	}
	return outUrl;
}


std::string urlEncode(const std::string& p_rawData)
{
	std::string escaped;
	escaped.reserve(p_rawData.length()); // reserve size for at least the original string
	
	for (std::string::const_iterator it = p_rawData.begin(); it != p_rawData.end(); ++it)
	{
		// List of unreserved characters from: http://en.wikipedia.org/wiki/Percent_encoding
		
		// Check if the current character is *unreserved* (anything else has to be encoded)
		if ((*it >= '0' && *it <= '9') ||
		    (*it >= 'a' && *it <= 'z') ||
		    (*it >= 'A' && *it <= 'Z') ||
		    *it == '-' || *it == '_' ||*it == '.' || *it == '~')
		{
			// Unreserved character; add as-is
			escaped += *it;
		}
		else
		{
			// Not an unreserved character, so has to be percent-encoded
			// (convert numeric byte value to hexadecimal representation)
			escaped += "%" + char2hex(*it);
		}
	}
	
	return escaped;
}


std::string base64Encode(const std::string& p_rawData)
{
	static const char base64_table[] =
	{
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"
	};
	
	std::string dst;
	dst.reserve(((p_rawData.length() + 2) / 3) * 4);
	
	for (u32 srcPos = 0; srcPos < static_cast<u32>(p_rawData.length()); srcPos += 3)
	{
		const u32 filling = static_cast<u32>((srcPos + 3) > p_rawData.length() ? p_rawData.length() - srcPos : 3);
		
		dst += base64_table[u32(p_rawData[srcPos + 0] & 0xFC) >> 2]; // FC = 11111100
		if (filling == 1)
		{
			dst += base64_table[((p_rawData[srcPos + 0] & 0x03) << 4)]; // 03 = 11
			dst += '=';
			dst += '=';
		}
		else if (filling == 2)
		{
			dst += base64_table[((p_rawData[srcPos + 0] & 0x03) << 4) | (u32(p_rawData[srcPos + 1] & 0xF0) >> 4)]; // 03 = 11
			dst += base64_table[((p_rawData[srcPos + 1] & 0x0F) << 2) | (u32(p_rawData[srcPos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
			dst += '=';
		}
		else
		{
			TT_ASSERT(filling == 3);
			dst += base64_table[((p_rawData[srcPos + 0] & 0x03) << 4) | (u32(p_rawData[srcPos + 1] & 0xF0) >> 4)]; // 03 = 11
			dst += base64_table[((p_rawData[srcPos + 1] & 0x0F) << 2) | (u32(p_rawData[srcPos + 2] & 0xC0) >> 6)]; // 0F = 1111, C0=11110
			dst += base64_table[p_rawData[srcPos + 2] & 0x3F]; // 3F = 111111
		}
	}
	
	return dst;
}


std::string bytesToHex(const u8* p_bytes, s32 p_count, bool p_lowerCase)
{
	TT_NULL_ASSERT(p_bytes);
	TT_ASSERTMSG(p_count > 0,
	             "Invalid byte count specified: %d (must be a non-zero positive integer).",
	             p_count);
	if (p_bytes == 0 || p_count <= 0)
	{
		return std::string();
	}
	
	std::string            ret(static_cast<std::string::size_type>(p_count * 2), ' ');
	std::string::size_type retIdx = 0;
	
	const std::string::value_type hexA = (p_lowerCase ? 'a' : 'A');
	
	for (s32 i = 0; i < p_count; ++i, ++p_bytes, retIdx += 2)
	{
		s32 dig1 = static_cast<s32>((*p_bytes & 0xF0) >> 4);
		s32 dig2 = static_cast<s32>(*p_bytes & 0x0F);
		if (dig1 >= 0  && dig1 <=  9) dig1 += '0';
		if (dig1 >= 10 && dig1 <= 15) dig1 += hexA - 10;
		if (dig2 >= 0  && dig2 <=  9) dig2 += '0';
		if (dig2 >= 10 && dig2 <= 15) dig2 += hexA - 10;
		
		ret[retIdx    ] = static_cast<std::string::value_type>(dig1);
		ret[retIdx + 1] = static_cast<std::string::value_type>(dig2);
	}
	
	return ret;
}


QueryParameters parseQuery(const std::string& p_query)
{
	if (p_query.empty())
	{
		return QueryParameters();
	}
	
	// No leading question mark should be present in the query
	if (p_query.at(0) == '?')
	{
		TT_PANIC("Query '%s' contains a leading question mark. This is not part of the query.",
		         p_query.c_str());
	}
	
	// Split the query up into key-value pair parts that are separated by &
	tt::str::Strings queryParts(tt::str::explode(p_query, "&"));
	
	// Split each part up into separate key and value strings
	QueryParameters params;
	
	for (tt::str::Strings::iterator it = queryParts.begin(); it != queryParts.end(); ++it)
	{
		std::string key;
		std::string value;
		if (tt::str::split(*it, '=', key, value) == false)
		{
			TT_PANIC("Invalid query parameter block: '%s'", (*it).c_str());
			return QueryParameters();
		}
		
		std::pair<QueryParameters::iterator, bool> result(params.insert(std::make_pair(key, value)));
		if (result.second == false)
		{
			TT_WARN("Query '%s' contains multiple definitions of key '%s'. Overwriting existing value.",
			        p_query.c_str(), key.c_str());
			(*result.first).second = value;
		}
	}
	
	return params;
}


std::string buildQuery(const QueryParameters& p_params, bool p_urlEncode)
{
	if (p_params.empty())
	{
		return std::string();
	}
	
	std::string query;
	
	for (QueryParameters::const_iterator it = p_params.begin(); it != p_params.end(); )
	{
		query += (*it).first + "=";
		if (p_urlEncode)
		{
			query += urlEncode((*it).second);
		}
		else
		{
			query += (*it).second;
		}
		
		++it;
		if (it != p_params.end())
		{
			query += "&";
		}
	}
	
	return query;
}

// Namespace end
}
}
