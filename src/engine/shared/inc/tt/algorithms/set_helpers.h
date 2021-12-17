#if !defined(INC_TT_ALGORITHMS_SET_HELPERS_H)
#define INC_TT_ALGORITHMS_SET_HELPERS_H

#include <set>
#include <tt/platform/tt_error.h>

namespace tt {
namespace algorithms {


template <class Type>
struct SetIntersectResult
{
	std::set<Type> onlyInFirst;
	std::set<Type> intersection;
	std::set<Type> onlyInSecond;
	
	inline SetIntersectResult()
	:
	onlyInFirst(),
	intersection(),
	onlyInSecond()
	{ }
};


template <class Type>
inline SetIntersectResult<Type> intersectSet(const std::set<Type>& p_first,
                                             const std::set<Type>& p_second)
{
	typedef std::set<Type> ContainerType;
	SetIntersectResult<Type> result;
	
	typename ContainerType::const_iterator firstIt  = p_first.begin();
	typename ContainerType::const_iterator secondIt = p_second.begin();
	
	while (firstIt != p_first.end() && secondIt != p_second.end())
	{
		if ((*firstIt) < (*secondIt))
		{
			result.onlyInFirst.insert((*firstIt));
			++firstIt;
		}
		else if ((*secondIt) < (*firstIt))
		{
			result.onlyInSecond.insert((*secondIt));
			++secondIt;
		}
		else
		{
			TT_ASSERT((*secondIt) == (*firstIt));
			result.intersection.insert((*firstIt));
			++secondIt;
			++firstIt;
		}
	}
	
	while (firstIt != p_first.end())
	{
		result.onlyInFirst.insert((*firstIt));
		++firstIt;
	}
	
	while (secondIt != p_second.end())
	{
		result.onlyInSecond.insert((*secondIt));
		++secondIt;
	}
	
	return result;
}

// End namespace
}
}


#endif // !defined(INC_TT_ALGORITHMS_SET_HELPERS_H)
