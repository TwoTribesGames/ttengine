#ifndef INC_TT_SHARED_MATH_TTCMATH_H
#define INC_TT_SHARED_MATH_TTCMATH_H

#include <cmath>
#include <tt/platform/tt_types.h>


//! \brief  Declares all members of cmath into the tt::math namespace.
//!			Definition order maps 
//! 		http://www.cplusplus.com/reference/clibrary/cmath/

namespace tt {
namespace math {

//! \brief Cos
inline double cos ( double x )
{
	return std::cos(x);
}

inline float cos ( float x )
{
	return std::cos(x);
}


inline long double cos ( long double x )
{
	return std::cos(x);
}


//! \brief Sin
inline double sin ( double x )
{
	return std::sin(x);
}

inline float sin (float x)
{
	return std::sin(x);
}


inline long double sin ( long double x )
{
	return std::sin(x);
}


//! \brief Tan
inline double tan ( double x )
{
	return std::tan(x);
}


inline float tan ( float x )
{
	return std::tan(x);
}


inline long double tan(long double x )
{
	return std::tan(x);
}


//! \brief Acos
inline double acos(double x)
{
	return std::acos(x);
}


inline float acos(float x)
{
	return std::acos(x);
}


inline long double acos(long double x)
{
	return std::acos(x);
}


//! \brief ASin
inline double asin(double x)
{
	return std::asin(x);
}


inline float asin(float x)
{
	return std::asin(x);
}


inline long double asin(long double x)
{
	return std::asin(x);
}



//! \brief ATan
inline double atan ( double x )
{
	return std::atan(x);
}


inline float atan ( float x )
{
	return std::atan(x);
}


inline long double atan ( long double x )
{
	return std::atan(x);
}



//! \brief Atan2
inline double atan2 ( double y, double x )
{
	return std::atan2(y,x);
}

inline long double atan2 ( long double y, long double x)
{
	return std::atan2(y,x);
}


inline float atan2 ( float y, float x )
{
	return std::atan2(y,x);
}




//! \brief Cosh
inline double cosh ( double x )
{
	return std::cosh(x);
}


inline float cosh ( float x )
{
	return std::cosh(x);
}


inline long double cosh ( long double x )
{
	return std::cosh(x);
}




//! \brief Sinh
inline double sinh ( double x )
{
	return std::sinh(x);
}


inline float sinh ( float x )
{
	return std::sinh(x);
}


inline long double sinh ( long double x )
{
	return std::sinh(x);
}



//! \brief Tanh
inline double tanh ( double x )
{
	return std::tanh(x);
}

inline float tanh ( float x )
{
	return std::tanh(x);
}


inline long double tanh ( long double x )
{
	return std::tanh(x);
}




//! \brief exp
inline double exp ( double x )
{
	return std::exp(x);
}

inline float exp (  float x )
{
	return std::exp(x);
}


inline long double exp ( long double x )
{
	return std::exp(x);
}



//! \brief frexp
inline double frexp ( double x, int * exp )
{
	return std::frexp(x,exp);
}

inline float frexp ( float x, int * exp )
{
	return std::frexp(x,exp);
}

inline long double frexp ( long double x, int * exp )
{
	return std::frexp(x,exp);
}



//! \brief ldexp
inline double ldexp ( double x, int exp )
{
	return std::ldexp(x,exp);
}

inline float ldexp ( float x, int exp )
{
	return std::ldexp(x,exp);
}


inline long double ldexp ( long double x, int exp )
{
	return std::ldexp(x,exp);
}


//! \brief log
inline double log ( double x )
{
	return std::log(x);
}

inline float log ( float x )
{
	return std::log(x);
}

inline long double log ( long double x )
{
	return std::log(x);
}


//! \brief log10
inline double log10 ( double x )
{
	return std::log10(x);
}

inline float log10 ( float x )
{
	return std::log10(x);
}


inline long double log10 ( long double x )
{
	return std::log10(x);
}



//! \brief modf
inline double modf (  double x,  double * intpart )
{
	return std::modf(x, intpart);
}

inline long double modf ( long double x, long double * intpart )
{
	return std::modf(x, intpart);
}


inline float modf ( float x, float * intpart )
{
	return std::modf(x, intpart);
}




//! \brief pow
inline double pow ( double base, double exponent )
{
	return std::pow(base, exponent);
}

inline long double pow ( long double base, long double exponent)
{
	return std::pow(base, exponent);
}


inline float pow ( float base, float exponent )
{
	return std::pow(base, exponent);
}



//! \brief sqrt

// internal function
inline float sqrt ( float number )
{
	return ::sqrtf(number);
}


inline double sqrt ( double number )
{
	return ::sqrt(number);
}


//! \brief ceil
inline double ceil ( double x )
{
	return std::ceil(x);
}

inline float ceil ( float x )
{
	return std::ceil(x);
}


inline long double ceil ( long double x )
{
	return std::ceil(x);
}




//! \brief fabs
inline double fabs ( double x )
{
	return std::fabs(x);
}

inline float fabs ( float x )
{
	return std::fabs(x);
}


inline long double fabs ( long double x )
{
	return std::fabs(x);
}




//! \brief floor
inline double floor ( double x )
{
	return std::floor(x);
}


inline float floor ( float x )
{
	return std::floor(x);
}


inline long double floor ( long double x )
{
	return std::floor(x);
}



//! \brief fmod
inline double fmod ( double numerator, double denominator )
{
	return std::fmod(numerator, denominator);
}


inline float fmod ( float numerator, float denominator )
{
	return std::fmod(numerator, denominator);
}


inline long double fmod ( 
		long double numerator, 
		long double denominator )
{
	return std::fmod(numerator, denominator);
}



}
}



#endif

