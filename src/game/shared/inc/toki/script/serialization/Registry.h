#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_REGISTRY_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_REGISTRY_H

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


#include <unordered_map>
#include <vector>

namespace toki {
namespace script {
namespace serialization {

template <typename Type, typename Data>
class Registry
{
public:
	Registry()
	:
	m_storedData(),
	m_lookup()
	{
		m_storedData.reserve(reserveCount);
#if !defined(TT_PLATFORM_CAT)
		m_lookup.reserve(reserveCount);
#endif
	}
	
	inline bool contains(const Type& p_entry) const { return m_lookup.find(p_entry) != m_lookup.end(); }
	inline s32 indexOf(const Type& p_entry) const
	{
		typename Lookup::const_iterator it = m_lookup.find(p_entry);
		if (it != m_lookup.end())
		{
			return it->second;
		}
		
		return -1;
	}
	
	inline Type getLookup(s32 p_index) const
	{
		TT_ASSERT(p_index >= 0 && p_index < size());
		return m_storedData[p_index].second;
	}
	
	inline s32 add(const Type& p_entry)
	{
		return add(p_entry, Data());
	}
	
	inline s32 add(const Type& p_entry, const Data& p_data)
	{
		TT_ASSERT(contains(p_entry) == false);
		TT_ASSERT(m_lookup.size() == m_storedData.size());
		TT_ASSERT(m_lookup.size() <= reserveCount);
		
		s32 index = size();
		m_lookup[p_entry] = index;
		m_storedData.emplace_back(p_data, p_entry);
		return index;
	}
	
	inline s32 size() const { return static_cast<s32>(m_storedData.size()); }
	
	inline Data& operator[](s32 p_idx)
	{
		TT_ASSERT(p_idx >= 0 && p_idx < size());
		return m_storedData[static_cast<typename StoredData::size_type>(p_idx)].first;
	}
	
	inline const Data& operator[](s32 p_idx) const
	{
		TT_ASSERT(p_idx >= 0 && p_idx < size());
		return m_storedData[static_cast<typename StoredData::size_type>(p_idx)].first;
	}
	
	inline Data& operator[](const Type& p_entry)
	{
		return operator[](indexOf(p_entry));
	}
	
	inline void clear()
	{
		m_storedData.clear();
		m_lookup.clear();
	}
	
private:
	static const s32 reserveCount = 8192;
	
	typedef std::pair<Data, Type> DataLookupPair;
	typedef std::vector<DataLookupPair> StoredData;
	StoredData m_storedData;
	
	typedef std::unordered_map<Type, s32> Lookup;
	Lookup m_lookup;
};

// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_REGISTRY_H)
