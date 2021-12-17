#if !defined(INC_TT_CODE_HANDLE_H)
#define INC_TT_CODE_HANDLE_H

#include <ostream>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_error.h>
#include <tt/math/math.h>

namespace tt {
namespace code {


// Forward declaration
template< class MgrType >
class HandleMgr;

template< class MgrType >
class HandleArrayMgr;

class HandleBase
{
public:
	enum Constants
	{
		// Bit sizes
		Constants_IndexSize   = 12,
		Constants_CounterSize = 15
	};
	TT_STATIC_ASSERT(Constants_IndexSize + Constants_CounterSize <= 32);
	
	/* \returns True if this handle is empty (default constructed) or, false if it's bound to a pointer. */
	inline bool isEmpty() const { return m_counter == 0;  }
	inline void invalidate()    { (*this) = HandleBase(); }
	
	inline u32 getValue() const { return m_counter << (Constants_IndexSize) | m_index; }
	
protected:
	inline HandleBase()
	:
	m_index(0),
	m_counter(0)
	{}
	
	inline HandleBase(u32 p_index, u32 p_counter)
	:
	m_index(p_index),
	m_counter(p_counter)
	{
		TT_ASSERTMSG(p_index   <= (1 << Constants_IndexSize),
		             "Handle Index:   %u is too large for only %d bits. (Max is %d.)",
		             p_index,   Constants_IndexSize, (1 << Constants_IndexSize));
		TT_ASSERTMSG(p_counter <= (1 << Constants_CounterSize),
		             "Handle Counter: %u is too large for only %d bits. (Max is %d.)",
		             p_counter, Constants_CounterSize, (1 << Constants_CounterSize));
	}
	
	u32 m_index   : Constants_IndexSize;
	u32 m_counter : Constants_CounterSize;
};


template < class Type >
class Handle : public HandleBase
{
public:
	typedef Type ValueType;
	
	inline Handle() : HandleBase() { }
	
	static inline Handle createFromRawValue(u32 p_value)
	{
		// inverse of return m_counter << (Constants_IndexSize) | m_index;
		const u32 indexMask   = (1 << HandleBase::Constants_IndexSize  ) - 1;
		const u32 counterMask = (1 << HandleBase::Constants_CounterSize) - 1;
		
		const u32 index   = (p_value                                   ) & indexMask;
		const u32 counter = (p_value >> HandleBase::Constants_IndexSize) & counterMask;
		return Handle(index, counter);
	}
	static inline Handle createForTesting(u32 p_index, u32 p_counter) { return Handle(p_index, p_counter); }
	
	// Overload so Handle to a Base class can be created from a Handle to a derived class
	template < class DerivedType >
	explicit inline Handle(const Handle<DerivedType>& p_derivedHandle)
	:
	HandleBase(p_derivedHandle, DerivedType::handleType)
	{
#if !defined(TT_BUILD_FINAL)
		// Make sure derivedType ptrs can be assigened to the Type ptrs.
		DerivedType* derivedPtr = 0;
		Type* basePtr = derivedPtr;
		(void)basePtr;
#endif
	}
	
	inline ValueType* getPtr() const { return isEmpty() ? 0 : ValueType::getPointerFromHandle(*this); }
	
private:
	// Actual creation, can only be created by HandleMgr of <ValueType>.
	inline Handle(u32 p_index, u32 p_counter)
	:
	HandleBase(p_index, p_counter)
	{ }
	
	template <class MgrType> friend class HandleMgr;
	template <class MgrType> friend class HandleArrayMgr;
};


template < class Type >
bool operator==(const Handle<Type>& p_lhs, const Handle<Type>& p_rhs)
{
	return p_lhs.getValue() == p_rhs.getValue();
}


template < class Type >
bool operator!=(const Handle<Type>& p_lhs, const Handle<Type>& p_rhs)
{
	return (p_lhs == p_rhs) == false;
}


template < class Type >
bool operator<(const Handle<Type>& p_lhs, const Handle<Type>& p_rhs)
{
	return p_lhs.getValue() < p_rhs.getValue();
}


template < class Type >
inline std::ostream& operator<<(std::ostream& s, const Handle<Type>& p_handle)
{
	return s << "Handle: " << p_handle.getValue();
}



// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_HANDLE_H)
