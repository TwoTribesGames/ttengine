#if !defined(INC_TT_PRES_ANIM2D_TWEENTYPES_H)
#define INC_TT_PRES_ANIM2D_TWEENTYPES_H

#include <string>
#include <tt/platform/tt_types.h>


namespace tt {
namespace pres {
namespace anim2d {


enum TweenType // types from http://hosted.zeh.com.br/mctween/animationtypes.html
{
	TweenType_SINE,
	TweenType_QUINT,
	TweenType_QUART,
	TweenType_QUAD,
	TweenType_EXPO,
	TweenType_ELASTIC,
	TweenType_CUBIC,
	TweenType_CIRC,
	TweenType_BOUNCE,
	TweenType_BACK,

	TweenType_Count
};


namespace tween {
class Easing;
}


class Tween
{
public:
	static TweenType tweenTypeFromString(const std::string& p_name);
	static std::string tweenTypeToString(TweenType p_tweentype);
	
	static const tween::Easing& tweenFactory(TweenType p_tweentype);
};


namespace tween {


class Easing 
{
public:
	Easing() {}
	virtual ~Easing() {}
	
	//pure virtual
	virtual real easeIn   (real t, real b, real c, real d) const = 0;
	virtual real easeOut  (real t, real b, real c, real d) const = 0;
	virtual real easeInOut(real t, real b, real c, real d) const = 0;

};


class Back : public Easing
{
public:
	virtual ~Back() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Bounce : public Easing
{
public:
	virtual ~Bounce() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Circ : public Easing
{
public:
	virtual ~Circ() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Cubic : public Easing
{
public:
	virtual ~Cubic() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Elastic : public Easing
{
public:
	virtual ~Elastic() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Expo : public Easing
{
public:
	virtual ~Expo() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Quad : public Easing 
{

public:
	virtual ~Quad() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Quart : public Easing 
{

public:
	virtual ~Quart() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};

class Quint : public Easing 
{
public :
	virtual ~Quint() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


class Sine : public Easing 
{
public :
	virtual ~Sine() {}
	virtual real easeIn   (real t, real b, real c, real d) const;
	virtual real easeOut  (real t, real b, real c, real d) const;
	virtual real easeInOut(real t, real b, real c, real d) const;
};


static Sine g_Sine;
static Quint g_Quint;
static Quart g_Quart;
static Quad  g_Quad;
static Expo g_Expo;
static Elastic g_Elastic;
static Cubic g_Cubic;
static Circ g_Circ;
static Bounce g_Bounce;
static Back g_Back;

//namespace end
}
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_TWEENTYPES_H)
