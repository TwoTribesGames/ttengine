#if !defined(INC_TT_MATH_HASH_NAMEDHASH_H)
#define INC_TT_MATH_HASH_NAMEDHASH_H

#include <tt/math/hash/Hash.h>


namespace tt {
namespace math {
namespace hash {

/*! \brief Same as Hash, but with the original string value stored (in non-final builds)
           This class should only be used by code that require the original name to be available
           for debugging purposes. Since it can involve strings, this class may perform
           significantly worse in non-final builds as compared to Hash.*/
template<int BitSize>
class NamedHash : public Hash<BitSize>
{
public:
	static const NamedHash invalid;

	explicit NamedHash(typename Hash<BitSize>::ValueType p_hashValue = Hash<BitSize>::INVALID_HASH)
	:
	Hash<BitSize>(p_hashValue)
#if !defined(TT_BUILD_FINAL)
	,
	m_name(new std::string("INVALID"))
#endif
	{ }
	
	explicit NamedHash(const std::string& p_string)
	:
	Hash<BitSize>(p_string)
#if !defined(TT_BUILD_FINAL)
	,
	m_name(new std::string(p_string))
#endif
	{ }
	
	inline const std::string& getName() const
	{
#if !defined(TT_BUILD_FINAL)
		return *m_name;
#else
		static std::string emptyString;
		return emptyString;
#endif
	}
	
	explicit NamedHash(const char* p_string)
	:
	Hash<BitSize>(p_string)
#if !defined(TT_BUILD_FINAL)
	,
	m_name(new std::string(p_string))
#endif
	{ }
	
private:
#if !defined(TT_BUILD_FINAL)
	typedef tt_ptr<std::string>::shared StringPtr;
	StringPtr m_name;
#endif
};

template<int BitSize>
const NamedHash<BitSize> NamedHash<BitSize>::invalid;

// Namespace end
}
}
}

#endif  // !defined(INC_TT_MATH_HASH_HASH_H)
