#ifndef INC_TT_TYPES_H
#define INC_TT_TYPES_H

//-------------------------------------
// Standard (tt) types

typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;

typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed long s32;

typedef signed long long int s64;

typedef volatile u8 vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;

typedef volatile s8 vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float f32;
typedef volatile f32 vf32;

#ifdef _WIN64
typedef s64 intptr;
typedef u64 uintptr;
#else
typedef s32 intptr;
typedef u32 uintptr;
#endif

//-------------------------------------
// real

typedef float real;
typedef double real64;

// Fixed - float conversion (no effect on WIN)
inline float realToFloat(real p_value)
{
	return p_value;
}

inline double realToFloat(real64 p_value)
{
	return p_value;
}


inline int realToInt(real p_value)
{
	return static_cast<int>(p_value);
}

//-------------------------------------
// Smart pointer

//#include <boost/shared_ptr.hpp>
//#include <boost/weak_ptr.hpp>

#if defined(_MSC_VER)
#include <memory>

template<typename T>
class tt_ptr
{
public:
#if _MSC_VER < 1600
	typedef std::tr1::unique_ptr<T> unique;
	typedef std::tr1::shared_ptr<T> shared;
	typedef std::tr1::weak_ptr<T>   weak;
#else
	typedef std::unique_ptr<T> unique;
	typedef std::shared_ptr<T> shared;
	typedef std::weak_ptr<T>   weak;
#endif
};


/*! \brief Dynamic cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_dynamic_cast(const Other& p_other)
{
#if _MSC_VER < 1600
	return std::tr1::dynamic_pointer_cast<Ty>(p_other);
#else
	return std::dynamic_pointer_cast<Ty>(p_other);
#endif
}

/*! \brief Const cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_const_cast(const Other& p_other)
{
#if _MSC_VER < 1600
	return std::tr1::const_pointer_cast<Ty>(p_other);
#else
	return std::const_pointer_cast<Ty>(p_other);
#endif
}

/*! \brief Static cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_static_cast(const Other& p_other)
{
#if _MSC_VER < 1600
	return std::tr1::static_pointer_cast<Ty>(p_other);
#else
	return std::static_pointer_cast<Ty>(p_other);
#endif
}

#else

#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

template<typename T>
class tt_ptr
{
public:
	typedef std::unique_ptr<T> unique;
	typedef boost::shared_ptr<T> shared;
	typedef boost::weak_ptr<T>   weak;
};


/*! \brief Dynamic cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_dynamic_cast(const Other& p_other)
{
	return boost::dynamic_pointer_cast<Ty>(p_other);
}

/*! \brief Const cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_const_cast(const Other& p_other)
{
	return boost::const_pointer_cast<Ty>(p_other);
}

/*! \brief Static cast for shared pointers. */
template<class Ty, class Other>
typename tt_ptr<Ty>::shared tt_ptr_static_cast(const Other& p_other)
{
	return boost::static_pointer_cast<Ty>(p_other);
}

#endif


//-------------------------------------

// Validate the types
#include <tt/platform/tt_types_validation.h>

#endif  // !defined(INC_TT_TYPES_H)
