#if !defined(INC_TT_CODE_HANDLEARRAYMGR_H)
#define INC_TT_CODE_HANDLEARRAYMGR_H

#include <tt/code/HandleMgr.h>
#include <tt/code/Handle.h>

namespace tt {
namespace code {

/* \brief Helper class which extends HandleMgr to also store the objects of type <Type>.
          Create will also 'allocate' the resource for the object (using placement new).

This class imposes the following requirements on Type:
- Needs to be copy constructable (however, see the point below!).

- Copy constructor and assignment operator will only ever be called in a "swap" context:
  swap places of two Type instances. These operators are therefore expected to perform a
  shallow copy (do not perform a full copy with separate resource allocation etc).
  Take extra care with raw pointers in this situation: avoid double deletion!

- Constructor and destructor are used in a typical RAII manner: an instance is only created
  or destroyed when requested. If create() is never called, no Type constructor will ever be called.

- Type must have an inner type named ConstructorParamType. An instance of this type
  is passed to the Type constructor upon creation.

- The constructor of Type must take two parameters:
   * The first is Type::ConstructorParamType: client-specified initialization data.
   * The second is the Handle to this newly created object.

- Type must have a getHandle() function, which returns the Handle<Type> to itself.
*/
template < class Type >
class HandleArrayMgr : public HandleMgr<Type>
{
public:
	typedef typename HandleMgr<Type>::HandleType HandleType;
	
	explicit HandleArrayMgr(s32 p_reserveCount);
	~HandleArrayMgr();
	
	HandleType create(typename Type::ConstructorParamType p_constructionParam);
	Type* createWithSpecificHandle(typename Type::ConstructorParamType p_constructionParam, const HandleType& p_handle); //<! \brief Create object with specific handle, used for loading code.
	bool addWithSpecificHandle(const Type& p_value, const HandleType& p_handle); //<! \brief Add value with specific handle, used for loading code.
	void destroy(const HandleType& p_handle);
	
	/*! \brief Swaps the two objects referred to by their handle.
	           The handles keep pointing to the same objects,
	           but the objects' locations in the array are swapped. */
	void swapObjectsInArray(HandleType p_a, HandleType p_b);
	
	template<typename Compare>
	void sort(Compare p_comparator);
	
	void reset();
	
	inline s32 getActiveCount() const { return m_activeCount; }
	inline s32 getMax()         const { return m_max;         }
	
	inline const Type* getFirst() const { return reinterpret_cast<const Type*>(m_objects); }
	inline       Type* getFirst()       { return reinterpret_cast<      Type*>(m_objects); }
	
private:
	void destroy(Type* p_objectToDestroy);
	u8*  getStoragePtr(s32 p_objectIndex);
	
	inline static void swap(Type& p_a, Type& p_b)
	{
		Type tmp = p_a;
		p_a = p_b;
		p_b = tmp;
		tmp.invalidateTempCopy();  // invalidate the temporary copy, so that no regular cleanup is attempted
	}
	
	u8* m_objects;     // Raw storage for m_max * Type objects (objects will be allocated in this buffer using placement new)
	s32 m_max;         // Max number of objects. (Size of array)
	s32 m_activeCount; // Count of the number of active objects. (Created / Alive)
};

// Namespace end
}
}

#include "HandleArrayMgr.inl"

#endif  // !defined(INC_TT_CODE_HANDLEARRAYMGR_H)
