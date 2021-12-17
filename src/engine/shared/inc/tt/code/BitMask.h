#if !defined(INC_TT_CODE_BITMASK_H)
#define INC_TT_CODE_BITMASK_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace code {

template < bool needs64 > struct BitMaskSizeTrait
{
	typedef u32 ValueType;
	static const s32       maxBits    = 32;
	static const ValueType allBitsSet = 0xFFFFFFFF;
};


template < >              struct BitMaskSizeTrait<true>
{
	typedef u64 ValueType;
	static const s32       maxBits    = 64;
	static const ValueType allBitsSet = 0xFFFFFFFFFFFFFFFFull;
};


template < class FlagEnum, s32 FlagCountParam >
class BitMask
{
public:
	typedef FlagEnum Flag;
	typedef BitMaskSizeTrait<(FlagCountParam > 32)> SizeTrait;
	typedef typename SizeTrait::ValueType ValueType;
	static const s32 FlagCount = FlagCountParam;
	
	explicit inline BitMask(ValueType p_flags = 0u)
	:
	m_flags(p_flags)
	{ }
	
	explicit inline BitMask(Flag p_flag)
	:
	m_flags(ValueType(1) << p_flag)
	{ }
	
	inline bool      isEmpty()         const { return m_flags == 0; }
	inline ValueType getFlags()        const { return m_flags;      }
	std::string      getBitsAsString() const;
	
	inline void setFlag(   Flag p_flag)       { TT_MINMAX_ASSERT(p_flag, 0, FlagCount - 1);         m_flags |=  (ValueType(1) << p_flag);       }
	inline void resetFlag( Flag p_flag)       { TT_MINMAX_ASSERT(p_flag, 0, FlagCount - 1);         m_flags &= ~(ValueType(1) << p_flag);       }
	inline void toggleFlag(Flag p_flag)       { TT_MINMAX_ASSERT(p_flag, 0, FlagCount - 1);         m_flags ^=  (ValueType(1) << p_flag);       }
	inline bool checkFlag( Flag p_flag) const { TT_MINMAX_ASSERT(p_flag, 0, FlagCount - 1); return (m_flags &   (ValueType(1) << p_flag)) != ValueType(0); }
	
	// Set a flag on/off
	inline void setFlag(Flag p_flag, bool p_on)
	{
		if (p_on)   setFlag(p_flag);
		else      resetFlag(p_flag);
	}
	
	inline void setFlags(     const BitMask<FlagEnum, FlagCount>& p_bitmask)       { m_flags |=  p_bitmask.m_flags; }
	inline void resetFlags(   const BitMask<FlagEnum, FlagCount>& p_bitmask)       { m_flags &= ~p_bitmask.m_flags; }
	inline void toggleFlags(  const BitMask<FlagEnum, FlagCount>& p_bitmask)       { m_flags ^=  p_bitmask.m_flags; }
	//! \returns true if all bits from p_bitmask are set.
	inline bool checkFlags(   const BitMask<FlagEnum, FlagCount>& p_bitmask) const { return (m_flags & p_bitmask.m_flags) == p_bitmask.m_flags; }
	//! \returns true if any bit from p_bitmask are set.
	inline bool checkAnyFlags(const BitMask<FlagEnum, FlagCount>& p_bitmask) const { return (m_flags &  p_bitmask.m_flags) != 0; }
	inline bool noOtherFlags( const BitMask<FlagEnum, FlagCount>& p_bitmask) const { return (m_flags & ~p_bitmask.m_flags) == 0; }
	
	inline void setAllFlags()
	{
		// Start with all bits set and then shift the unused bits to zero.
		m_flags = SizeTrait::allBitsSet >> (SizeTrait::maxBits - FlagCount);
	}
	inline void resetAllFlags() { m_flags = 0; }
	
private:
	ValueType m_flags;
	
	TT_STATIC_ASSERT(FlagCount >= 0);
	TT_STATIC_ASSERT((size_t)FlagCount <= (sizeof(ValueType) * 8)); // Make sure there are enough bits in ValueType to represent all flags
};


template < class FlagEnum, s32 FlagCount >
bool operator==(const BitMask<FlagEnum, FlagCount>& p_lhs,
                const BitMask<FlagEnum, FlagCount>& p_rhs)
{
	return p_lhs.getFlags() == p_rhs.getFlags();
}


template < class FlagEnum, s32 FlagCount >
const BitMask<FlagEnum, FlagCount> operator|(const BitMask<FlagEnum, FlagCount>& p_lhs,
                                             const BitMask<FlagEnum, FlagCount>& p_rhs)
{
	return BitMask<FlagEnum, FlagCount>(p_lhs.getFlags() | p_rhs.getFlags());
}


template < class FlagEnum, s32 FlagCount >
const BitMask<FlagEnum, FlagCount> operator&(const BitMask<FlagEnum, FlagCount>& p_lhs,
                                             const BitMask<FlagEnum, FlagCount>& p_rhs)
{
	return BitMask<FlagEnum, FlagCount>(p_lhs.getFlags() & p_rhs.getFlags());
}


//-------------------------------------------------------------------------------------------------
// Inline functions

template < class FlagEnum, s32 FlagCount >
std::string BitMask<FlagEnum, FlagCount>::getBitsAsString() const
{
	std::string result(FlagCount, '0');
	
	ValueType       scratch = m_flags;
	const ValueType msb     = static_cast<ValueType>(1) << (FlagCount - 1);
	for (s32 i = 0; i < FlagCount; ++i)
	{
		if (scratch & msb)
		{
			result[i] = '1';
		}
		scratch <<= 1;
	}
	return result;
}

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_BITMASK_H)
