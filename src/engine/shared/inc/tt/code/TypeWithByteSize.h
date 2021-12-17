#ifndef INC_TT_CODE_TYPEWITHBITSIZE_H
#define INC_TT_CODE_TYPEWITHBITSIZE_H

#include <tt/platform/tt_types.h>

namespace tt {
namespace code {


// Declare size-types
template<int bitSize> 
class TypeWithBitSize; 


template <>
class TypeWithBitSize<16>
{
public:
	typedef s16 type;
	typedef u16 unsignedType;
	typedef s32 doubleType;
};


template <>
class TypeWithBitSize<32>
{
public:
	typedef s32 type;
	typedef u32 unsignedType;
	typedef s64 doubleType;
};


template <>
class TypeWithBitSize<64>
{
public:
	typedef s64 type;
	typedef u64 unsignedType;
	typedef s64 doubleType; // FIXME: We don't have a 128 bit type.
};


// End namespace
}
}

#endif
