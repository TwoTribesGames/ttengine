#include <tt/asset/NameHashFunc.h>


namespace tt {
namespace asset {


/// \brief Convert string to hash - enlarges and calls wide version
/// \return a 64 bit value, but the hashing function is not really 64 bit 
AssetIdType strToHash( const std::string& p_strkey )
{
	std::wstring widekey(p_strkey.begin(), p_strkey.end());
	return strToHash(widekey);
}



/// \brief Convert wide string to hash
/// \return a 64 bit value, but the hashing function is not really 64 bit 
AssetIdType strToHash( const std::wstring& p_strkey )
{
	u32 b = 378551;
	u32 a = 63689;

	u32 h=0;

	for ( u32 c =0; c < p_strkey.length(); ++c )
	{
		h = (h * a) + p_strkey[c];
		a = a * b;
	}
	return (h & 0x7FFFFFFF);
}



/// \brief Convert arbitrary bytestream to hash
/// \return a 64 bit value, but the hashing function is not really 64 bit 
AssetIdType chunkToHash( u8* p_chunk, std::size_t p_length )
{
	u32 b = 378551;
	u32 a = 63689;

	u32 h=0;

	for ( u32 c =0; c < p_length; ++c )
	{
		h = (h * a) + p_chunk[c];
		a = a * b;
	}
	return (h & 0x7FFFFFFF);

}



}
}

