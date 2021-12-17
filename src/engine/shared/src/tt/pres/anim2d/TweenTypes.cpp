#include <tt/pres/anim2d/TweenTypes.h>
#include <tt/platform/tt_error.h>
#include <tt/math/math.h>


namespace tt {
namespace pres {
namespace anim2d {

anim2d::TweenType Tween::tweenTypeFromString( const std::string& p_name )
{
	if      (p_name == "sine")        return TweenType_SINE;
	else if (p_name == "quint")       return TweenType_QUINT;
	else if (p_name == "quart")       return TweenType_QUART;
	else if (p_name == "quad")        return TweenType_QUAD;
	else if (p_name == "expo")        return TweenType_EXPO;
	else if (p_name == "elastic")     return TweenType_ELASTIC;
	else if (p_name == "cubic")       return TweenType_CUBIC;
	else if (p_name == "circ")        return TweenType_CIRC;
	else if (p_name == "bounce")      return TweenType_BOUNCE;
	else if (p_name == "back")        return TweenType_BACK;

	else
	{
		TT_PANIC("Unknown tweentype '%s'.", p_name.c_str());
		return TweenType(0);
	}
}


std::string Tween::tweenTypeToString(TweenType p_tweentype)
{
	switch (p_tweentype)
	{
	case TweenType_SINE:            return "sine";
	case TweenType_QUINT:           return "quint";
	case TweenType_QUART:           return "quart";
	case TweenType_QUAD:            return "quad";
	case TweenType_EXPO:            return "expo";
	case TweenType_ELASTIC:         return "elastic";
	case TweenType_CUBIC:           return "cubic";
	case TweenType_CIRC:            return "circ";
	case TweenType_BOUNCE:          return "bounce";
	case TweenType_BACK:            return "back";

	default: TT_PANIC("Unknown tween type: %d", p_tweentype); return "unknown";
	}
}


const anim2d::tween::Easing& Tween::tweenFactory(TweenType p_tweentype)
{
	switch(p_tweentype)
	{
	case TweenType_SINE:            return tween::g_Sine;
	case TweenType_QUINT:           return tween::g_Quint;
	case TweenType_QUART:           return tween::g_Quart;
	case TweenType_QUAD:            return tween::g_Quad;
	case TweenType_EXPO:            return tween::g_Expo;
	case TweenType_ELASTIC:         return tween::g_Elastic;
	case TweenType_CUBIC:           return tween::g_Cubic;
	case TweenType_CIRC:            return tween::g_Circ;
	case TweenType_BOUNCE:          return tween::g_Bounce;
	case TweenType_BACK:            return tween::g_Back;

	default: TT_PANIC("Unknown tween type: %d", p_tweentype); return tween::g_Quad;
	}
}


namespace tween {


//tween functions from http://code.google.com/p/cpptweener
/***** SINE ****/

real Sine::easeIn (real t, real b , real c, real d) const
{
	return -c * math::cos(t / d * (math::pi / 2)) + c + b;
}
real Sine::easeOut(real t, real b , real c, real d) const
{
	return c * math::sin(t / d * (math::pi / 2)) + b;
}

real Sine::easeInOut(real t, real b , real c, real d) const
{
	return -c / 2 * (math::cos(math::pi * t / d) - 1) + b;
}

/**** Quint ****/

real Quint::easeIn (real t, real b , real c, real d) const
{
	t /= d;
	return c * t * t * t * t * t + b;
}
real Quint::easeOut(real t, real b , real c, real d) const
{
	t = t / d - 1;
	return c * (t * t * t * t * t + 1) + b;
}

real Quint::easeInOut(real t, real b , real c, real d) const
{
	t /= d / 2;
	if (t < 1) return c / 2 * t * t * t * t * t + b;
	t -= 2;
	return c / 2 * (t * t * t * t * t + 2) + b;
}

/**** Quart ****/
real Quart::easeIn (real t, real b , real c, real d) const
{
	t /= d;
	return c * t * t * t * t + b;
}
real Quart::easeOut(real t,real b , real c, real d)const 
{
	t = t / d - 1;
	return -c * (t * t * t * t - 1) + b;
}

real Quart::easeInOut(real t,real b , real c, real d) const
{
	t /= d / 2;
	if (t < 1) return c / 2 * t * t * t * t + b;
	t -= 2;
	return -c / 2 * (t * t * t * t - 2) + b;
}

/**** Quad ****/
real Quad::easeIn (real t,real b , real c, real d) const
{
	t /= d;
	return c * t * t + b;
}
real Quad::easeOut(real t,real b , real c, real d) const
{
	t /= d;
	return -c * t * (t - 2) + b;
}

real Quad::easeInOut(real t,real b , real c, real d)const 
{
	t /= d / 2;
	if (t < 1) return ((c / 2) * (t * t)) + b;
	--t;
	return -c / 2 * (((t - 2) * t) - 1) + b;
	/*
	originally return -c/2 * (((t-2)*(--t)) - 1) + b;

	I've had to swap (--t)*(t-2) due to diffence in behaviour in
	pre-increment operators between java and c++, after hours
	of joy
	*/
	//return c * (-0.5f * (math::cos(t/d * math::pi) - 1.0f)) + b;
}

/**** Expo ****/

real Expo::easeIn (real t,real b , real c, real d) const
{
	return (t == 0) ? b : c * math::pow(static_cast<real>(2), 10.0f * (t / d - 1.0f)) + b;
}
real Expo::easeOut(real t,real b , real c, real d) const
{
	return (t == d) ? b + c : c * ( -math::pow(static_cast<real>(2), -10 * t / d) + 1) + b;
}

real Expo::easeInOut(real t,real b , real c, real d) const
{
	if (t == 0) return b;
	if (t == d) return b + c;
	t /= d / 2;
	if (t < 1) return c / 2 * math::pow(static_cast<real>(2), 10 * (t - 1)) + b;
	--t;
	return c / 2 * (-math::pow(static_cast<real>(2), -10 * t) + 2) + b;
}


/****  Elastic ****/

real Elastic::easeIn (real t,real b , real c, real d) const
{
	if (t == 0) return b;
	t /= d;
	if (t == 1) return b + c;
	real p = d * 0.3f;
	real a = c;
	real s = p / 4;
	t -= 1;
	real postFix = a * math::pow(static_cast<real>(2), 10 * t); // this is a fix, again, with post-increment operators
	return -(postFix * math::sin((t * d - s) * (2 * math::pi) / p)) + b;
}

real Elastic::easeOut(real t,real b , real c, real d) const
{
	if (t == 0) return b;  
	t /= d;
	if (t == 1) return b + c;
	real p = d * 0.3f;
	real a = c;
	real s = p / 4;
	return a * math::pow(static_cast<real>(2), -10 * t) * 
		math::sin((t * d - s) * (2 * math::pi) / p) + c + b;
}

real Elastic::easeInOut(real t,real b , real c, real d) const
{
	if (t == 0) return b;
	t /= d / 2;
	if (t == 2) return b + c;
	real p = d * (0.3f * 1.5f);
	real a = c;
	real s = p / 4;

	t-=1;
	if (t < 1) 
	{
		real postFix = a * math::pow(static_cast<real>(2), 10 * t); // postIncrement is evil
		return -.5f * (postFix * math::sin((t * d - s) * (2 * math::pi) / p)) + b;
	}
	real postFix = a * math::pow(static_cast<real>(2), -10 * t); // postIncrement is evil
	return postFix * math::sin((t * d - s) * (2 * math::pi) / p) * .5f + c + b;
}

/****  Cubic ****/
real Cubic::easeIn (real t,real b , real c, real d) const
{
	t /= d;
	return c * t * t * t + b;
}
real Cubic::easeOut(real t,real b , real c, real d) const
{
	t = t / d - 1;
	return c * (t * t * t + 1) + b;
}

real Cubic::easeInOut(real t,real b , real c, real d) const 
{
	t /= d / 2;
	if (t < 1) return c / 2 * t * t * t + b;
	t -= 2;
	return c / 2 * (t * t * t + 2) + b;
}

/*** Circ ***/

real Circ::easeIn (real t,real b , real c, real d) const
{  
	t /= d;
	return -c * (math::sqrt(1 - t * t) - 1) + b;
}
real Circ::easeOut(real t,real b , real c, real d) const
{
	t = t / d - 1;
	return c * math::sqrt(1 - t * t) + b;
}

real Circ::easeInOut(real t,real b , real c, real d) const
{
	t /= d / 2;
	if (t < 1) return -c / 2 * (math::sqrt(1 - t * t) - 1) + b;
	t -= 2;
	return c / 2 * (math::sqrt(1 - t * t) + 1) + b;
}

/****  Bounce ****/
real Bounce::easeIn (real t,real b , real c, real d) const
{
	return c - easeOut (d - t, 0, c, d) + b;
}


real Bounce::easeOut(real t,real b , real c, real d) const
{
	t /= d;
	if (t < static_cast<real>(1 / 2.75f))
	{
		return c * (7.5625f * t * t) + b;
	} 
	else if (t < static_cast<real>(2 / 2.75f))
	{
		t -= (1.5f / 2.75f);
		return c * (7.5625f * t * t + .75f) + b;
	}
	else if (t < static_cast<real>(2.5f / 2.75f))
	{
		t -= (2.25f / 2.75f);
		return c * (7.5625f * t * t + .9375f) + b;
	}
	else
	{
		t -= (2.625f / 2.75f);
		return c * (7.5625f * t * t + .984375f) + b;
	}
}

real Bounce::easeInOut(real t,real b , real c, real d) const
{
	if (t < d / 2) return easeIn (t * 2, 0, c, d) * 0.5f + b;
	else return easeOut (t * 2 - d, 0, c, d) * 0.5f + c * 0.5f + b;
}

/**** Back *****/

real Back::easeIn (real t,real b , real c, real d) const
{
	real s = 1.70158f;
	t /= d;
	return c * t * t * ((s + 1) * t - s) + b;
}
real Back::easeOut(real t,real b , real c, real d) const
{
	real s = 1.70158f;
	t = t / d - 1;
	return c * (t * t * ((s + 1) * t + s) + 1) + b;
}

real Back::easeInOut(real t,real b , real c, real d) const
{
	real s = 1.70158f * 1.525f;
	t /= d / 2;
	if (t < 1) return c / 2 * (t * t * ((s + 1) * t - s)) + b;
	t -= 2;
	return c / 2 * (t * t * ((s + 1) * t + s) + 2) + b;
}



}

//namespace end
}
}
}
