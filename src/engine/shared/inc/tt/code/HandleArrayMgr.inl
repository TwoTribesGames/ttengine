#include <tt/code/HandleMgr.h>
#include <tt/mem/mem.h>


namespace tt {
namespace code {

//--------------------------------------------------------------------------------------------------
// Public member functions

template < class Type >
HandleArrayMgr<Type>::HandleArrayMgr(s32 p_reserveCount)
:
m_objects(reinterpret_cast<u8*>(mem::alloc(p_reserveCount * sizeof(Type)))),
m_max(p_reserveCount),
m_activeCount(0)
{
}


template < class Type >
HandleArrayMgr<Type>::~HandleArrayMgr()
{
	// Call destructor on all allocated objects
	Type* objectToDestroy = getFirst();
	for (s32 i = 0; i < m_activeCount; ++i, ++objectToDestroy)
	{
		objectToDestroy->~Type();
	}
	
	mem::free(m_objects);
}


template < class Type >
typename HandleArrayMgr<Type>::HandleType HandleArrayMgr<Type>::create(
		typename Type::ConstructorParamType p_constructionParam)
{
	if (m_activeCount + 1 > m_max)
	{
		TT_PANIC("Can't create any more objects. Max (%d) number are active (%d)!",
		         m_max, m_activeCount);
		return HandleType();
	}
	
	// Get the storage space pointer for the new object
	u8* storagePtr = getStoragePtr(m_activeCount);
	
	// Register a new handle for the new object (do this first, so that we have access to a Handle)
	HandleType newHandle = HandleMgr<Type>::add(reinterpret_cast<Type*>(storagePtr));
	
	// Create a new object using placement new
	Type* newObject = new (storagePtr) Type(p_constructionParam, newHandle);
	(void)newObject; // NOTE: Pointer value not actually needed: this address was already registered with a new handle
	
	++m_activeCount;
	
	return newHandle;
}


template < class Type >
Type* HandleArrayMgr<Type>::createWithSpecificHandle(typename Type::ConstructorParamType p_constructionParam,
                                                     const typename HandleArrayMgr<Type>::HandleType& p_handle)
{
	if (m_activeCount + 1 > m_max)
	{
		TT_PANIC("Can't create any more objects. Max (%d) number are active (%d)!",
		         m_max, m_activeCount);
		return 0;
	}
	
	// Get the storage space pointer for the new object
	u8* storagePtr = getStoragePtr(m_activeCount);
	
	// Add a explicit handle for the new object (do this first, so we know if it fails)
	if (HandleMgr<Type>::addWithSpecificHandle(p_handle, reinterpret_cast<Type*>(storagePtr)) == false)
	{
		return 0;
	}
	
	// Create a new object using placement new
	Type* newObject = new (storagePtr) Type(p_constructionParam, p_handle);
	
	++m_activeCount;
	
	return newObject;
}


template < class Type >
bool HandleArrayMgr<Type>::addWithSpecificHandle(const Type&                                      p_value,
                                                 const typename HandleArrayMgr<Type>::HandleType& p_handle)
{
	if (m_activeCount + 1 > m_max)
	{
		TT_PANIC("Can't create any more objects. Max (%d) number are active (%d)!",
		         m_max, m_activeCount);
		return false;
	}
	
	// Get the storage space pointer for the new object
	u8* storagePtr = getStoragePtr(m_activeCount);
	
	// Add a explicit handle for the new object (do this first, so we know if it fails)
	if (HandleMgr<Type>::addWithSpecificHandle(p_handle, reinterpret_cast<Type*>(storagePtr)) == false)
	{
		return false;
	}
	
	// Create a new object using placement new
	Type* newObject = new (storagePtr) Type(p_value); // Copy constructor.
	(void)newObject; // NOTE: Pointer value not actually needed: this address was already registered with a new handle
	
	++m_activeCount;
	
	return true;
}


template < class Type >
void HandleArrayMgr<Type>::destroy(const HandleType& p_handle)
{
	// Silently accept destruction of null handles (same behavior as deletion of null pointers)
	if (p_handle.isEmpty())
	{
		return;
	}
	destroy(HandleMgr<Type>::get(p_handle));  // Cleanup object.
	HandleMgr<Type>::removeHandle(p_handle);  // Cleanup handle.
}


template < class Type >
void HandleArrayMgr<Type>::swapObjectsInArray(HandleType p_a, HandleType p_b)
{
	Type* a;
	Type* b;
	
	if (p_a == p_b || HandleMgr<Type>::get(p_a, a) == false || HandleMgr<Type>::get(p_b, b) == false)
	{
		TT_PANIC("Invalid objects to swap: either A or B handle is invalid or both handles are the same.");
		return;
	}
	
	swap(*a, *b);
	
	HandleMgr<Type>::update(p_a, b);
	HandleMgr<Type>::update(p_b, a);
}

template < class Type >
template < typename Compare >
void HandleArrayMgr<Type>::sort(Compare p_comparator)
{
	Type* a = getFirst();
	s32 i = 0;
	s32 j = 0;
	
	// Insertion sort; which is slow the first time, but expected to be much faster later.
	// Because generally we don't expect the sorted array to change much afterwards.
	// If it does change a lot and you see performance problems here, use quicksort instead.
	for (i = 1; i < m_activeCount; ++i) 
	{
		j = i;
		while (j > 0 && p_comparator(a[j-1], a[j]) == false)
		{
			swapObjectsInArray(a[j].getHandle(), a[j-1].getHandle());
			--j;
		}
	}
}


template < class Type >
void HandleArrayMgr<Type>::reset()
{
	// Call destructor on all allocated objects
	Type* objectToDestroy = getFirst();
	for (s32 i = 0; i < m_activeCount; ++i, ++objectToDestroy)
	{
		// Invalidate (internal) handle before destroying object.
		HandleMgr<Type>::update(objectToDestroy->getHandle(), 0);
		objectToDestroy->~Type();
	}
	
	HandleMgr<Type>::reset();
	m_activeCount = 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

template < class Type >
void HandleArrayMgr<Type>::destroy(Type* p_objectToRemove)
{
	if (p_objectToRemove == 0)
	{
		// Already gone. ( Do we want to panic here?)
		return;
	}
	
	if (m_activeCount > 1)
	{
		// Override the 'to delete'-object with the info of the last active object and update handle with new location.
		Type* endObject = reinterpret_cast<Type*>(getStoragePtr(m_activeCount - 1));
		
		if (p_objectToRemove != endObject)
		{
			// Swap with last object in array.
			swap(*p_objectToRemove, *endObject);
			
			// Update the handle of the end so it points to the new pointer.
			HandleMgr<Type>::update(p_objectToRemove->getHandle(), p_objectToRemove);
		}
	}
	
	// Destroy the removed object
	{
		Type* removedObject = reinterpret_cast<Type*>(getStoragePtr(m_activeCount - 1));
		removedObject->~Type();
		
		// Sometimes removedObject is seen as unused by the compiler, hence the fix below
		(void)removedObject;
	}
	
	TT_ASSERT(m_activeCount > 0);
	--m_activeCount;
}


template < class Type >
u8* HandleArrayMgr<Type>::getStoragePtr(s32 p_objectIndex)
{
	TT_ASSERT(p_objectIndex >= 0 && p_objectIndex < m_max);
	return m_objects + (p_objectIndex * sizeof(Type));
}

// Namespace end
}
}
