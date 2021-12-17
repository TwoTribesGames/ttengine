#if !defined(INC_TT_CODE_HELPERS_H)
#define INC_TT_CODE_HELPERS_H


#include <algorithm>
#include <vector>


namespace tt {
namespace code {
namespace helpers {

//--------------------------------------------------------------------------------------------------
// Resource deallocation helpers

/*! \brief Performs a fast erase on an element in a container, without moving all the elements. Doesn't preserve order.
    \param p The container to erase from. 
    \param p The iterator to erase. Cannot be const_iterator. */
template<typename T>
inline typename std::vector<T>::iterator unorderedErase(std::vector<T>& p_container,
                                                        typename std::vector<T>::iterator p_it)
{
	if (p_it != p_container.end() - 1)
	{
		std::swap(*p_it, p_container.back());
		p_container.pop_back();
		return p_it;
	}
	// else
	p_container.pop_back();
	return p_container.end();
}


/*! \brief Deletes memory and sets the pointer to 0.
    \param p The pointer to the memory to delete. */
template<typename T>
inline void safeDelete(T*& p)
{
	delete p;
	p = 0;
}


/*! \brief Deletes an array and sets the pointer to 0.
    \param p The pointer to the array to delete. */
template<typename T>
inline void safeDeleteArray(T*& p)
{
	delete[] p;
	p = 0;
}


/*! \brief Performs a swap with an empty container.
    \param p The container to clear. */
template<typename T>
inline void freeContainer(T& p_container)
{
	T empty;
	using std::swap;
	swap(p_container, empty);
}


/*! \brief Deletes every pointer in a container and clears the container.
    \param p The container to clear. */
template<typename T>
inline void freePointerContainer(T& p_container)
{
	for (typename T::iterator it = p_container.begin();
	     it != p_container.end();
	     ++it)
	{
		delete *it;
	}
	
	freeContainer(p_container);
}


/*! \brief Deletes the second value of each pair in a container and clears
           the container.
    \param p The container to clear. */
template<typename T>
inline void freePairSecondContainer(T& p_container)
{
	for (typename T::iterator it = p_container.begin();
	     it != p_container.end();
	     ++it)
	{
		delete (*it).second;
	}
	
	freeContainer(p_container);
}


/*! \brief Finds a value that is the 'second' member of a pair container (such as map).
    \param p_begin Iterator to the begin position for the search (e.g. map.begin()).
    \param p_end   Iterator to one past the end of the range to search (e.g. map.end()).
    \param p_value The value to search for.
    \return Iterator to the pair that has the search value as 'second', or p_end if not found. */
template<typename InIt, typename Type>
inline InIt findPairSecondValue(InIt p_begin, InIt p_end, const Type& p_value)
{
	for ( ; p_begin != p_end; ++p_begin)
	{
		if (p_begin->second == p_value)
		{
			break;
		}
	}
	
	return p_begin;
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_CODE_HELPERS_H)
