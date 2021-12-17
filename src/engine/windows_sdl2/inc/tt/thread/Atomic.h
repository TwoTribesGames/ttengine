#ifndef INC_TT_THREAD_ATOMIC_H
#define INC_TT_THREAD_ATOMIC_H

#include <windows.h>
#undef min
#undef max

#include <tt/platform/tt_types.h>


namespace tt {
namespace thread {
namespace atomic {

static inline u32 inc32(vu32* p_addend)
{
	return static_cast<u32>(
		InterlockedIncrement(reinterpret_cast<LONG volatile*>(p_addend)));
}


static inline s32 inc32(vs32* p_addend)
{
	return static_cast<s32>(
		InterlockedIncrement(reinterpret_cast<LONG volatile*>(p_addend)));
}


static inline u64 inc64(vu64* p_addend)
{
	return static_cast<u64>(
		InterlockedIncrement64(reinterpret_cast<LONGLONG volatile*>(p_addend)));
}


static inline s64 inc64(vs64* p_addend)
{
	return static_cast<s64>(
		InterlockedIncrement64(reinterpret_cast<LONGLONG volatile*>(p_addend)));
}


static inline u32 dec32(vu32* p_addend)
{
	return static_cast<u32>(
		InterlockedDecrement(reinterpret_cast<LONG volatile*>(p_addend)));
}


static inline s32 dec32(vs32* p_addend)
{
	return static_cast<s32>(
		InterlockedDecrement(reinterpret_cast<LONG volatile*>(p_addend)));
}


static inline u64 dec64(vu64* p_addend)
{
	return static_cast<u64>(
		InterlockedDecrement64(reinterpret_cast<LONGLONG volatile*>(p_addend)));
}


static inline s64 dec64(vs64* p_addend)
{
	return static_cast<s64>(
		InterlockedDecrement64(reinterpret_cast<LONGLONG volatile*>(p_addend)));
}


static inline u32 add32(vu32* p_lhs, u32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old + p_rhs, old) != old );
	
	return static_cast<u32>(old);
}


static inline s32 add32(vs32* p_lhs, s32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old + p_rhs, old) != old );
	
	return static_cast<s32>(old);
}


static inline u64 add64(vu64* p_lhs, u64 p_rhs)
{
	return static_cast<u64>(
		InterlockedExchangeAdd64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                         static_cast<LONGLONG>(p_rhs))) + p_rhs;
}


static inline s64 add64(vs64* p_lhs, s64 p_rhs)
{
	return static_cast<s64>(
		InterlockedExchangeAdd64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                         static_cast<LONGLONG>(p_rhs))) + p_rhs;
}


static inline u32 and32(vu32* p_lhs, u32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old & p_rhs, old) != old );
	
	return static_cast<u32>(old);
}


static inline s32 and32(vs32* p_lhs, s32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old | p_rhs, old) != old );
	
	return static_cast<s32>(old);
}


static inline u64 and64(vu64* p_lhs, u64 p_rhs)
{
	return static_cast<u64>(
		InterlockedAnd64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                 static_cast<LONGLONG>(p_rhs)));
}


static inline s64 and64(vs64* p_lhs, s64 p_rhs)
{
	return static_cast<s64>(
		InterlockedAnd64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                 static_cast<LONGLONG>(p_rhs)));
}


static inline u32 or32(vu32* p_lhs, u32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old | p_rhs, old) != old );
	
	return static_cast<u32>(old);
}


static inline s32 or32(vs32* p_lhs, s32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old | p_rhs, old) != old );
	
	return static_cast<s32>(old);
}


static inline u64 or64(vu64* p_lhs, u64 p_rhs)
{
	return static_cast<u64>(
		InterlockedOr64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                static_cast<LONGLONG>(p_rhs)));
}


static inline s64 or64(vs64* p_lhs, s64 p_rhs)
{
	return static_cast<s64>(
		InterlockedOr64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                static_cast<LONGLONG>(p_rhs)));
}


static inline u32 xor32(vu32* p_lhs, u32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old ^ p_rhs, old) != old );
	
	return static_cast<s32>(old);
}


static inline s32 xor32(vs32* p_lhs, s32 p_rhs)
{
	LONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONG*>(p_lhs);
	}
	while ( InterlockedCompareExchange(
	            reinterpret_cast<volatile LONG*>(p_lhs), old ^ p_rhs, old) != old );
	
	return static_cast<s32>(old);
}


static inline u64 xor64(vu64* p_lhs, u64 p_rhs)
{
	return static_cast<u64>(
		InterlockedXor64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                 static_cast<LONGLONG>(p_rhs)));
}


static inline s64 xor64(vs64* p_lhs, s64 p_rhs)
{
	return static_cast<s64>(
		InterlockedXor64(reinterpret_cast<LONGLONG volatile*>(p_lhs),
		                 static_cast<LONGLONG>(p_rhs)));
}


static inline bool bitTestAndSet32(vu32* p_base, u32 p_bit)
{
	return InterlockedBitTestAndSet(
		reinterpret_cast<LONG volatile*>(p_base),
		static_cast<LONG>(p_bit)) == TRUE;
}


static inline bool bitTestAndSet32(vs32* p_base, u32 p_bit)
{
	return InterlockedBitTestAndSet(
		reinterpret_cast<LONG volatile*>(p_base),
		static_cast<LONG>(p_bit)) == TRUE;
}


static inline bool bitTestAndSet64(vu64* p_base, u64 p_bit)
{
	LONGLONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONGLONG*>(p_base);
	}
	while ( InterlockedCompareExchange64(
	            reinterpret_cast<volatile LONGLONG*>(p_base), old | (u64(1) << p_bit), old) != old );
	
	return (old & (u64(1) << p_bit)) != 0;
}


static inline bool bitTestAndSet64(vs64* p_base, u64 p_bit)
{
	LONGLONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONGLONG*>(p_base);
	}
	while ( InterlockedCompareExchange64(
	            reinterpret_cast<volatile LONGLONG*>(p_base), old | (u64(1) << p_bit), old) != old );
	
	return (old & (u64(1) << p_bit)) != 0;
}


static inline bool bitTestAndReset32(vu32* p_base, u32 p_bit)
{
	return InterlockedBitTestAndReset(
		reinterpret_cast<LONG volatile*>(p_base),
		static_cast<LONG>(p_bit)) == TRUE;
}


static inline bool bitTestAndReset32(vs32* p_base, u32 p_bit)
{
	return InterlockedBitTestAndReset(
		reinterpret_cast<LONG volatile*>(p_base),
		static_cast<LONG>(p_bit)) == TRUE;
}


static inline bool bitTestAndReset64(vu64* p_base, u32 p_bit)
{
	LONGLONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONGLONG*>(p_base);
	}
	while ( InterlockedCompareExchange64(
	            reinterpret_cast<volatile LONGLONG*>(p_base), old & ~(u64(1) << p_bit), old) != old );
	
	return (old & (u64(1) << p_bit)) != 0;
}


static inline bool bitTestAndReset64(vs64* p_base, u32 p_bit)
{
	LONGLONG old;
	
	do
	{
		old = *reinterpret_cast<volatile LONGLONG*>(p_base);
	}
	while ( InterlockedCompareExchange64(
	            reinterpret_cast<volatile LONGLONG*>(p_base), old & ~(u64(1) << p_bit), old) != old );
	
	return (old & (u64(1) << p_bit)) != 0;
}


static inline u32 exchange32(vu32* p_target, u32 p_value)
{
	return static_cast<u32>(
		InterlockedExchange(reinterpret_cast<LONG volatile*>(p_target),
		                    static_cast<LONG>(p_value)));
}


static inline s32 exchange32(vs32* p_target, s32 p_value)
{
	return static_cast<s32>(
		InterlockedExchange(reinterpret_cast<LONG volatile*>(p_target),
		                      static_cast<LONG>(p_value)));
}


static inline u64 exchange64(vu64* p_target, u64 p_value)
{
	return static_cast<u64>(
		InterlockedExchange64(reinterpret_cast<LONGLONG volatile*>(p_target),
		                    static_cast<LONGLONG>(p_value)));
}


static inline s64 exchange64(vs64* p_target, s64 p_value)
{
	return static_cast<s64>(
		InterlockedExchange64(reinterpret_cast<LONGLONG volatile*>(p_target),
		                      static_cast<LONGLONG>(p_value)));
}


static inline void* exchangePointer(void* volatile* p_target, void* p_value)
{
	return static_cast<void*>(
		InterlockedExchangePointer(reinterpret_cast<PVOID volatile*>(p_target),
		                           static_cast<PVOID>(p_value)));
}


static inline u32 exchangeAdd32(vu32* p_addend, u32 p_value)
{
	return static_cast<u32>(
		InterlockedExchangeAdd(reinterpret_cast<LONG volatile*>(p_addend),
		                       static_cast<LONG>(p_value)));
}


static inline s32 exchangeAdd32(vs32* p_addend, s32 p_value)
{
	return static_cast<s32>(
		InterlockedExchangeAdd(reinterpret_cast<LONG volatile*>(p_addend),
		                       static_cast<LONG>(p_value)));
}


static inline u64 exchangeAdd64(vu32* p_addend, u64 p_value)
{
	return static_cast<u64>(
		InterlockedExchangeAdd64(reinterpret_cast<LONGLONG volatile*>(p_addend),
		                         static_cast<LONGLONG>(p_value)));
}


static inline s64 exchangeAdd64(vs32* p_addend, s64 p_value)
{
	return static_cast<s64>(
		InterlockedExchangeAdd64(reinterpret_cast<LONGLONG volatile*>(p_addend),
		                         static_cast<LONGLONG>(p_value)));
}


static inline u32 compareExchange32(vu32* p_destination, u32 p_exchange, u32 p_comparand)
{
	return static_cast<u32>(
		InterlockedCompareExchange(reinterpret_cast<LONG volatile*>(p_destination),
		                           static_cast<LONG>(p_exchange),
		                           static_cast<LONG>(p_comparand)));
}


static inline s32 compareExchange32(vs32* p_destination, s32 p_exchange, s32 p_comparand)
{
	return static_cast<s32>(
		InterlockedCompareExchange(reinterpret_cast<LONG volatile*>(p_destination),
		                           static_cast<LONG>(p_exchange),
		                           static_cast<LONG>(p_comparand)));
}


static inline u64 compareExchange64(vu64* p_destination, u64 p_exchange, u64 p_comparand)
{
	return static_cast<u64>(
		InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile*>(p_destination),
		                             static_cast<LONGLONG>(p_exchange),
		                             static_cast<LONGLONG>(p_comparand)));
}


static inline s64 compareExchange64(vs64* p_destination, s64 p_exchange, s64 p_comparand)
{
	return static_cast<s64>(
		InterlockedCompareExchange64(reinterpret_cast<LONGLONG volatile*>(p_destination),
		                             static_cast<LONGLONG>(p_exchange),
		                             static_cast<LONGLONG>(p_comparand)));
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_THREAD_ATOMIC_H)
