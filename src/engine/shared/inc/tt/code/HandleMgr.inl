#include <tt/code/HandleMgr.h>


namespace tt {
namespace code {

//--------------------------------------------------------------------------------------------------
// Public member functions

template < class Type >
HandleMgr<Type>::HandleMgr()
:
m_activeEntryCount(0),
m_firstFreeEntry(0)
#if !defined(TT_BUILD_FINAL)
,
m_needInternalCleanup(true)
#endif
{
	reset();
}


template < class Type >
void HandleMgr<Type>::reset()
{
	m_activeEntryCount    = 0;
	m_firstFreeEntry      = 0;
#if !defined(TT_BUILD_FINAL)
	m_needInternalCleanup = false;
#endif
	
	for (u32 i = 0; i < Constants_EntityMax - 1; ++i)
	{
		m_entries[i] = HandleEntry(i + 1);
	}
	m_entries[Constants_EntityMax - 1] = HandleEntry();
}


template < class Type >
typename HandleMgr<Type>::HandleType HandleMgr<Type>::add(ValueType* p_ptr)
{
	TT_ASSERT(m_activeEntryCount < Constants_EntityMax - 1);
	assertInternalCleanup();
	
	const u32 newIndex = m_firstFreeEntry;
	TT_ASSERT(newIndex < Constants_EntityMax);
	TT_ASSERT(m_entries[newIndex].m_active == false);
	
	m_firstFreeEntry = m_entries[newIndex].m_nextFreeIndex;
	m_entries[newIndex].m_nextFreeIndex = 0;
	m_entries[newIndex].m_counter = m_entries[newIndex].m_counter + 1;
	if (m_entries[newIndex].m_counter == 0)
	{
		m_entries[newIndex].m_counter = 1;
	}
	m_entries[newIndex].m_active = true;
	m_entries[newIndex].m_entry = p_ptr;
	
	++m_activeEntryCount;
	
	return HandleType(newIndex, m_entries[newIndex].m_counter);
}


template < class Type >
void HandleMgr<Type>::update(const HandleType& p_handle, ValueType* p_ptr)
{
	const u32 index = p_handle.m_index;
	TT_ASSERT(m_entries[index].m_counter == p_handle.m_counter);
	TT_ASSERT(m_entries[index].m_active == true);  // yes, == true... to prevent error about "sizeof() applied to bit-field"
	
	m_entries[index].m_entry = p_ptr;
}


template < class Type >
void HandleMgr<Type>::removeHandle(const HandleType& p_handle)
{
	const u32 index = p_handle.m_index;
	TT_ASSERT(m_entries[index].m_counter == p_handle.m_counter);
	TT_ASSERT(m_entries[index].m_active == true);  // yes, == true... to prevent error about "sizeof() applied to bit-field"
	assertInternalCleanup();
	
	m_entries[index].m_nextFreeIndex = m_firstFreeEntry;
	m_entries[index].m_active        = 0;
	m_firstFreeEntry                 = index;
	
	--m_activeEntryCount;
}


template < class Type >
typename HandleMgr<Type>::ValueType* HandleMgr<Type>::get(const HandleType& p_handle) const
{
	ValueType* ptr;
	if (get(p_handle, ptr) == false)
	{
		return 0;
	}
	return ptr;
}


template < class Type >
bool HandleMgr<Type>::get(const HandleType& p_handle, ValueType*& p_out) const
{
	const HandleEntry& entry = m_entries[p_handle.m_index];
	p_out = entry.m_entry;
	return entry.m_counter == p_handle.m_counter && entry.m_active;
}


template < class Type >
bool HandleMgr<Type>::addWithSpecificHandle(const HandleType& p_handle, ValueType* p_ptr)
{
	TT_ASSERT(m_activeEntryCount < Constants_EntityMax - 1);
	
	const u32 index = p_handle.m_index;
	
	TT_ASSERT(m_entries[index].m_active == false); // Handle already used.
	
	if (index >= Constants_EntityMax || // Too large
	    m_entries[index].m_active    || // This index is already used. (index collision)
	    p_handle.m_counter == 0)        // Handle is empty.
	{
		return false;
	}
	
#if !defined(TT_BUILD_FINAL)
	m_needInternalCleanup = true;
#endif
	
	m_entries[index].m_nextFreeIndex = 0;
	m_entries[index].m_counter       = p_handle.m_counter;
	m_entries[index].m_active        = true;
	m_entries[index].m_entry         = p_ptr;
	
	++m_activeEntryCount;
	return true;
}


template < class Type >
void HandleMgr<Type>::doInternalCleanup()
{
	s32 activeEntriesFound = 0;
	u32 nextFreeIndex      = 0;
	
	for (u32 i = Constants_EntityMax - 1; /*break inside*/; --i)
	{
		if (m_entries[i].m_active == false)
		{
			m_entries[i].m_nextFreeIndex = nextFreeIndex;
			nextFreeIndex                = i;
		}
		else
		{
			++activeEntriesFound;
		}
		
		if (i == 0)
		{
			break;
		}
	}
	TT_ASSERT(m_activeEntryCount == activeEntriesFound);
	m_firstFreeEntry = nextFreeIndex;
	
#if !defined(TT_BUILD_FINAL)
	m_needInternalCleanup = false;
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions


template < class Type >
HandleMgr<Type>::HandleEntry::HandleEntry() 
:
m_nextFreeIndex(0),
m_counter(1),
m_active(0),
m_entry(0)
{}


template < class Type >
HandleMgr<Type>::HandleEntry::HandleEntry(u32 nextFreeIndex)
:
m_nextFreeIndex(nextFreeIndex),
m_counter(1),
m_active(0),
m_entry(0)
{}


// Namespace end
}
}
