#ifndef INC_TT_THREAD_ATOMIC_H
#define INC_TT_THREAD_ATOMIC_H


#include <tt/platform/tt_types.h>

namespace tt {
namespace thread {
namespace atomic {

// FIXME: All these functions are stubs for OS X!

static inline u32 inc32(vu32* p_addend)
{
#ifdef __GNUC__
	return __sync_add_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 inc32(vs32* p_addend)
{
#ifdef __GNUC__
	return __sync_add_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 inc64(vu64* p_addend)
{
#ifdef __GNUC__
	return __sync_add_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 inc64(vs64* p_addend)
{
#ifdef __GNUC__
	return __sync_add_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 dec32(vu32* p_addend)
{
#ifdef __GNUC__
	return __sync_sub_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 dec32(vs32* p_addend)
{
#ifdef __GNUC__
	return __sync_sub_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 dec64(vu64* p_addend)
{
#ifdef __GNUC__
	return __sync_sub_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 dec64(vs64* p_addend)
{
#ifdef __GNUC__
	return __sync_sub_and_fetch(p_addend, 1);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 add32(vu32* p_lhs, u32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 add32(vs32* p_lhs, s32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 add64(vu64* p_lhs, u64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 add64(vs64* p_lhs, s64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 and32(vu32* p_lhs, u32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_and(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 and32(vs32* p_lhs, s32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_and(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 and64(vu64* p_lhs, u64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_and(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 and64(vs64* p_lhs, s64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_and(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 or32(vu32* p_lhs, u32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_or(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 or32(vs32* p_lhs, s32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_or(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 or64(vu64* p_lhs, u64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_or(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 or64(vs64* p_lhs, s64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_or(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 xor32(vu32* p_lhs, u32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_xor(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 xor32(vs32* p_lhs, s32 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_xor(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 xor64(vu64* p_lhs, u64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_xor(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 xor64(vs64* p_lhs, s64 p_rhs)
{
#ifdef __GNUC__
	return __sync_fetch_and_xor(p_lhs, p_rhs);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline bool bitTestAndSet32(vu32* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vu32 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, bit) & bit) > 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndSet32(vs32* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vs32 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, bit) & bit) > 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndSet64(vu64* p_base, u64 p_bit)
{
#ifdef __GNUC__
	vu64 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, bit) & bit) > 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndSet64(vs64* p_base, u64 p_bit)
{
#ifdef __GNUC__
	vs64 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, bit) & bit) > 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndReset32(vu32* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vu32 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, ~bit) & bit) == 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndReset32(vs32* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vs32 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, ~bit) & bit) == 0;
#else
	#error No Atomic implementation for this compiler
#endif	

}


static inline bool bitTestAndReset64(vu64* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vu64 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, ~bit) & bit) == 0;
#else
	#error No Atomic implementation for this compiler
#endif	
}


static inline bool bitTestAndReset64(vs64* p_base, u32 p_bit)
{
#ifdef __GNUC__
	vs64 bit = (1 << p_bit);
	return (__sync_fetch_and_or(p_base, ~bit) & bit) == 0;
#else
	#error No Atomic implementation for this compiler
#endif	

}


static inline u32 exchange32(vu32* p_target, u32 p_value)
{
#ifdef __GNUC__
	return __atomic_exchange_n(p_target, p_value, __ATOMIC_RELEASE);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 exchange32(vs32* p_target, s32 p_value)
{
#ifdef __GNUC__
	return __atomic_exchange_n(p_target, p_value, __ATOMIC_RELEASE);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 exchange64(vu64* p_target, u64 p_value)
{
#ifdef __GNUC__
	return __atomic_exchange_n(p_target, p_value, __ATOMIC_RELEASE);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 exchange64(vs64* p_target, s64 p_value)
{
#ifdef __GNUC__
	return __atomic_exchange_n(p_target, p_value, __ATOMIC_RELEASE);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline void* exchangePointer(void* volatile* p_target, void* p_value)
{
#ifdef __GNUC__
	return __atomic_exchange_n(p_target, p_value, __ATOMIC_RELEASE);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 exchangeAdd32(vu32* p_addend, u32 p_value)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_addend, p_value);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 exchangeAdd32(vs32* p_addend, s32 p_value)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_addend, p_value);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 exchangeAdd64(vu32* p_addend, u64 p_value)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_addend, p_value);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 exchangeAdd64(vs32* p_addend, s64 p_value)
{
#ifdef __GNUC__
	return __sync_fetch_and_add(p_addend, p_value);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u32 compareExchange32(vu32* p_destination, u32 p_exchange, u32 p_comparand)
{
#ifdef __GNUC__
	return __sync_comare_and_swap(p_destination, p_comparand, p_exchange);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s32 compareExchange32(vs32* p_destination, s32 p_exchange, s32 p_comparand)
{
#ifdef __GNUC__
	return __sync_comare_and_swap(p_destination, p_comparand, p_exchange);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline u64 compareExchange64(vu64* p_destination, u64 p_exchange, u64 p_comparand)
{
#ifdef __GNUC__
	return __sync_comare_and_swap(p_destination, p_comparand, p_exchange);
#else
	#error No Atomic implementation for this compiler
#endif
}


static inline s64 compareExchange64(vs64* p_destination, s64 p_exchange, s64 p_comparand)
{
#ifdef __GNUC__
	return __sync_comare_and_swap(p_destination, p_comparand, p_exchange);
#else
	#error No Atomic implementation for this compiler
#endif
}

// Namespace end
}
}
}


#endif  // !defined(INC_TT_THREAD_ATOMIC_H)
