#if !defined(INC_TT_CODE_HANDLEMGR_H)
#define INC_TT_CODE_HANDLEMGR_H

#include <tt/code/Handle.h>

namespace tt {
namespace code {

// Helper class to couple a Handle to a pointer of type <Type>.
// (Does not do resource managment of the pointer, this is only the coupling.)

template < class Type >
class HandleMgr
{
public:
	typedef Type              ValueType;
	typedef Handle<ValueType> HandleType;
	
	HandleMgr();
	
	void reset();
	HandleType add(ValueType* p_ptr);
	void update(const HandleType& p_handle, ValueType* p_ptr);
	void removeHandle(const HandleType& p_handle);
	
	ValueType* get(const HandleType& p_handle) const;
	bool get(const HandleType& p_handle, ValueType*& p_out) const;
	
	inline s32 getCount() const { return m_activeEntryCount; }
	
	/*! \brief Used to create for loading code.
	    \note Must call doInternalCleanup before using other handle functions after using this function. */
	bool addWithSpecificHandle(const HandleType& p_handle, ValueType* p_ptr);
	void doInternalCleanup();
	
private:
	struct HandleEntry
	{
		HandleEntry();
		explicit HandleEntry(u32 nextFreeIndex);
		
		u32 m_nextFreeIndex : HandleType::Constants_IndexSize;
		u32 m_counter       : HandleType::Constants_CounterSize;
		u32 m_active        : 1;
		ValueType* m_entry;
	};
	
	enum Constants
	{
		Constants_EntityMax = 1 << HandleType::Constants_IndexSize
	};
	
	inline void assertInternalCleanup() const
	{
#if !defined(TT_BUILD_FINAL)
		TT_ASSERTMSG(m_needInternalCleanup == false,
		             "HandleMgr needs internal cleanup! "
		             "(After calling 'addWithSpecificHandle()', 'doInternalCleanup()' must be called!)");
#endif
	}
	
	HandleEntry m_entries[Constants_EntityMax];
	
	s32 m_activeEntryCount;
	u32 m_firstFreeEntry;
	
#if !defined(TT_BUILD_FINAL)
	bool m_needInternalCleanup; // Flag to warn about missing internal cleanup.
#endif
};


// Namespace end
}
}

#include "HandleMgr.inl"

#endif  // !defined(INC_TT_CODE_HANDLEMGR_H)
