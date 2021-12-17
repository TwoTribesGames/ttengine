#if !defined(INC_TT_HTTP_HELPERS_H)
#define INC_TT_HTTP_HELPERS_H


#include <map>
#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace http {

/*! \brief Parses raw data (received from an HTTP request for example) as a narrow string. */
std::string parseDataAsString(const u8* p_data, std::size_t p_dataLength);

std::string urlDecode(const std::string& p_encodedData);
std::string urlEncode(const std::string& p_rawData);

std::string base64Encode(const std::string& p_rawData);

/*! \brief Returns a string representation of a sequence of bytes individually converted to their hex equivalent.
    \param p_bytes The bytes to convert.
    \param p_count The number of bytes in the p_bytes array.
    \param p_lowerCase Whether the string representation should use lowercase or uppercase hex digits. */
std::string bytesToHex(const u8* p_bytes, s32 p_count, bool p_lowerCase = true);


typedef std::map<std::string, std::string> QueryParameters;

/*! \brief Parses the query part of a URL into key/value pairs.
    \param p_query Query part of the URL (e.g. 'param1=value1&param2=value2', without leading '?')
    \note Does not perform any URL encoding or decoding. */
QueryParameters parseQuery(const std::string& p_query);

/*! \brief Builds a query string for a URL.
    \param p_params The query parameters for the URL.
    \param p_urlEncode Whether to URL encode the parameter values. */
std::string buildQuery(const QueryParameters& p_params, bool p_urlEncode = true);


/*! \brief Builds a string from a set of key/value pairs.
    \param p_keyValueContainer The container of key/value pairs (map, vector of pairs).
    \param p_keyValueSeparator What to separate the key and value with (e.g., "=" for "key=value").
    \param p_itemSeparator What to separate each key/value pair with (e.g., "," for "pair,pair").
    \return Key/value pairs as a string. */
template<typename T>
inline std::string implodeKeyValue(const T&           p_keyValueContainer,
                                   const std::string& p_keyValueSeparator,
                                   const std::string& p_pairSeparator,
                                   bool               p_appendTrailingPairSeparator = false)
{
	std::string result;
	
	for (typename T::const_iterator it = p_keyValueContainer.begin(); it != p_keyValueContainer.end(); )
	{
		result += (*it).first + p_keyValueSeparator + (*it).second;
		
		++it;
		if (p_appendTrailingPairSeparator || it != p_keyValueContainer.end())
		{
			result += p_pairSeparator;
		}
	}
	
	return result;
}

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HELPERS_H)
