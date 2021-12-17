#include <string>

#include <tt/math/Random.h>
#include <tt/script/bindings/bindings.h>
#include <tt/script/wrappers/VisualBoyWrapper.h>
#include <tt/system/Time.h>
#include <tt/score/Score.h>


namespace tt {
namespace script{
namespace bindings {

//-------------------------------------------------------------------------------------------------
// ColorRGB


tt::engine::renderer::ColorRGB* colorRGB_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 3)
	{
		TT_PANIC("ColorRGB incorrect number of parameters. (Got: %d, expected: 3)", params);
		return 0;
	}
	
	SQInteger r;
	SQInteger g;
	SQInteger b;
	
	if (SQ_FAILED(sq_getinteger(v, 2, &r)))
	{
		TT_PANIC("ColorRGB.r is not a integer value");
		return 0;
	}
	
	if (tt::math::clamp(r, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGB.r is outside the 0 - 255 range! Clamping to %d!\n", r);
	}
	
	if (SQ_FAILED(sq_getinteger(v, SQInteger(3), &g)))
	{
		TT_PANIC("ColorRGB.g is not a integer value");
		return 0;
	}
	
	if (tt::math::clamp(g, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGB.g is outside the 0 - 255 range! Clamping to %d!\n", g);
	}
	
	if (SQ_FAILED(sq_getinteger(v, SQInteger(4), &b)))
	{
		TT_PANIC("ColorRGB.b is not a integer value");
		return 0;
	}
	
	if (tt::math::clamp(b, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGB.b is outside the 0 - 255 range! Clamping to %d!\n", b);
	}
	
	return new tt::engine::renderer::ColorRGB(static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b));
}


void bindColorRGB(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::engine::renderer;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(ColorRGB);
	TT_SQBIND_SET_CONSTRUCTOR(ColorRGB, colorRGB_constructor);
	
	// Bind members
	TT_SQBIND_MEMBER(ColorRGB, u8, r);
	TT_SQBIND_MEMBER(ColorRGB, u8, g);
	TT_SQBIND_MEMBER(ColorRGB, u8, b);
}


//-------------------------------------------------------------------------------------------------
// ColorRGBA

tt::engine::renderer::ColorRGBA* colorRGBA_constructor(HSQUIRRELVM v)
{
	const SQInteger paramCount = sq_gettop(v) - 1; // Stack has paramCount + context.
	
	SQInteger r;
	SQInteger g;
	SQInteger b;
	SQInteger a = 255;
	
	if (paramCount == 1 || paramCount == 2)
	{
		// Constructed from ColorRGB with optional alpha
		const tt::engine::renderer::ColorRGB rgb(SqBind<tt::engine::renderer::ColorRGB>::get(v, 2));
		r = static_cast<SQInteger>(rgb.r);
		g = static_cast<SQInteger>(rgb.g);
		b = static_cast<SQInteger>(rgb.b);
		
		if (paramCount == 2 && SQ_FAILED(sq_getinteger(v, 3, &a)))
		{
			TT_PANIC("Parameter 2 of ColorRGBA constructor is not an integer value");
			return 0;
		}
	}
	else if (paramCount == 3 || paramCount == 4)
	{
		// Constructed from separate color channels with optional alpha
		if (SQ_FAILED(sq_getinteger(v, 2, &r)))
		{
			TT_PANIC("Parameter 1 (r) of ColorRGBA constructor is not an integer value");
			return 0;
		}
		
		if (SQ_FAILED(sq_getinteger(v, 3, &g)))
		{
			TT_PANIC("Parameter 2 (g) of ColorRGBA constructor is not an integer value");
			return 0;
		}
		
		if (SQ_FAILED(sq_getinteger(v, 4, &b)))
		{
			TT_PANIC("Parameter 3 (b) of ColorRGBA constructor is not an integer value");
			return 0;
		}
		
		if (paramCount == 4 && SQ_FAILED(sq_getinteger(v, 5, &a)))
		{
			TT_PANIC("Parameter 4 (a) of ColorRGBA constructor is not an integer value");
			return 0;
		}
	}
	else
	{
		TT_PANIC("ColorRGBA contructor: unsupported number of parameters. "
		         "(Got: %d, expected: 1, 2, 3 or 4)", paramCount);
		return 0;
	}
	
	// Verify that all color channels are in range
	if (tt::math::clamp(r, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGBA.r is outside the 0 - 255 range! Clamping to %d!", r);
	}
	
	if (tt::math::clamp(g, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGBA.g is outside the 0 - 255 range! Clamping to %d!", g);
	}
	
	if (tt::math::clamp(b, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGBA.b is outside the 0 - 255 range! Clamping to %d!", b);
	}
	
	if (tt::math::clamp(a, SQInteger(0), SQInteger(255)))
	{
		TT_WARN("ColorRGBA.a is outside the 0 - 255 range! Clamping to %d!", a);
	}
	
	return new tt::engine::renderer::ColorRGBA(
			static_cast<u8>(r), static_cast<u8>(g), static_cast<u8>(b), static_cast<u8>(a));
}


void bindColorRGBA(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::engine::renderer;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(ColorRGBA);
	TT_SQBIND_SET_CONSTRUCTOR(ColorRGBA, colorRGBA_constructor);
	
	// Bind methods
	TT_SQBIND_METHOD(ColorRGBA, rgb);
	
	// Bind members
	TT_SQBIND_MEMBER(ColorRGBA, u8, r);
	TT_SQBIND_MEMBER(ColorRGBA, u8, g);
	TT_SQBIND_MEMBER(ColorRGBA, u8, b);
	TT_SQBIND_MEMBER(ColorRGBA, u8, a);
}


//--------------------------------------------------------------------------------------------------
// Vector2

static inline real vector2Distance(const tt::math::Vector2& p_a, const tt::math::Vector2& p_b)
{
	return tt::math::distance(p_a, p_b);
}


static inline real vector2DotProduct(const tt::math::Vector2& p_a, const tt::math::Vector2& p_b)
{
	return tt::math::dotProduct(p_a, p_b);
}


tt::math::Vector2* vector2_constructor(HSQUIRRELVM v) 
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 2)
	{
		TT_PANIC("Vector2 incorrect number of parameters. (Got: %d, expected: 2)", params);
		return 0;
	}
	
	SQFloat x;
	SQFloat y;
	
	if (SQ_FAILED(sq_getfloat(v, 2, &x)))
	{
		TT_PANIC("Vector2.x is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 3, &y)))
	{
		TT_PANIC("Vector2.y is not a float value");
		return 0;
	}
	
	return new tt::math::Vector2(x, y);
}


void bindVector2(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::math;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(Vector2);
	TT_SQBIND_SET_CONSTRUCTOR(Vector2, vector2_constructor);
	
	// Bind methods
	TT_SQBIND_METHOD(Vector2, normalize);
	TT_SQBIND_METHOD(Vector2, getNormalized);
	TT_SQBIND_METHOD(Vector2, length);
	TT_SQBIND_METHOD(Vector2, lengthSquared);
	TT_SQBIND_METHOD(Vector2, rotate);
	TT_SQBIND_METHOD(Vector2, getRotated);
	TT_SQBIND_METHOD(Vector2, getAngle);
	
	// Bind members
	TT_SQBIND_MEMBER(Vector2, real, x);
	TT_SQBIND_MEMBER(Vector2, real, y);
	
	// Bind functions
	TT_SQBIND_FUNCTION_NAME(vector2Distance, "distance");
	TT_SQBIND_FUNCTION_NAME(vector2DotProduct, "dotProduct");
}


//--------------------------------------------------------------------------------------------------
// VectorRect

tt::math::VectorRect* vectorRect_constructor(HSQUIRRELVM v)
{
	const SQInteger params = sq_gettop(v) - 1;  // Stack has params + context.
	
	if (params != 3)
	{
		TT_PANIC("VectorRect incorrect number of parameters. (Got: %d, expected: 3)", params);
		return 0;
	}
	
	SQFloat width  = 0.0f;
	SQFloat height = 0.0f;
	
	const tt::math::Vector2& centerPos(SqBind<tt::math::Vector2>::get(v, 2));
	if (SQ_FAILED(sq_getfloat(v, 3, &width)))
	{
		return 0;
	}
	if (SQ_FAILED(sq_getfloat(v, 4, &height)))
	{
		return 0;
	}
	
	tt::math::VectorRect* rect = new tt::math::VectorRect(centerPos, width, height);
	rect->setCenterPosition(centerPos);
	return rect;
}


void bindVectorRect(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::math;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(VectorRect);
	
	// Bind methods
	TT_SQBIND_SET_CONSTRUCTOR(VectorRect, vectorRect_constructor);
	TT_SQBIND_METHOD_NAME(VectorRect, getCenterPosition, "getPosition");
	TT_SQBIND_METHOD_NAME(VectorRect, setCenterPosition, "setPosition");
	TT_SQBIND_METHOD     (VectorRect, getWidth);
	TT_SQBIND_METHOD     (VectorRect, setWidth);
	TT_SQBIND_METHOD     (VectorRect, getHeight);
	TT_SQBIND_METHOD     (VectorRect, setHeight);
	TT_SQBIND_METHOD     (VectorRect, translate);
	TT_SQBIND_METHOD     (VectorRect, getMin);
	TT_SQBIND_METHOD     (VectorRect, getMaxInside);
	TT_SQBIND_METHOD     (VectorRect, getMaxEdge);
	
	// Rectangles are top-down; game code is bottom up. So in scription getTop() actually should return
	// the result of getBottom()
	// the C++ "top" of a rect is at the visible bottom, and vice versa
	TT_SQBIND_METHOD_NAME(VectorRect, getBottom, "getTop");
	TT_SQBIND_METHOD_NAME(VectorRect, getTop, "getBottom");
	TT_SQBIND_METHOD     (VectorRect, getLeft);
	TT_SQBIND_METHOD     (VectorRect, getRight);
	
	// Because the following functions are overloaded the parameters need to be explicitly specified.
	sqbind_method<VectorRect, const tt::math::Vector2&, real, bool>(_vm, _SC("containsCircle"),   &VectorRect::contains);
	sqbind_method<VectorRect, const tt::math::Vector2&, real, bool>(_vm, _SC("intersectsCircle"), &VectorRect::intersects);
	
	sqbind_method<VectorRect, const VectorRect&,              bool>(_vm, _SC("containsRect"),   &VectorRect::contains);
	sqbind_method<VectorRect, const VectorRect&, VectorRect*, bool>(_vm, _SC("intersectsRect"), &VectorRect::intersects);
	
	sqbind_method<VectorRect, const tt::math::Vector2&, bool>(_vm, _SC("containsPoint"), &VectorRect::contains);
}

//--------------------------------------------------------------------------------------------------
// Interpolation

tt::math::interpolation::Easing<real>* easingReal_constructor(HSQUIRRELVM v)
{
	const SQInteger paramCount = sq_gettop(v) - 1; // Stack has paramCount + context.
	
	using namespace tt::math::interpolation;
	
	real begin;
	real end;
	real duration;
	SQInteger type = EasingType_Invalid;
	
	if (paramCount != 4)
	{
		TT_PANIC("EasingReal constructor should have 4 parameters");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 2, &begin)))
	{
		TT_PANIC("Parameter 1 (begin) of EasingReal constructor is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 3, &end)))
	{
		TT_PANIC("Parameter 2 (end) of EasingReal constructor is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getfloat(v, 4, &duration)))
	{
		TT_PANIC("Parameter 3 (duration) of EasingReal constructor is not a float value");
		return 0;
	}
	
	if (SQ_FAILED(sq_getinteger(v, 5, &type)))
	{
		TT_PANIC("Parameter 4 (EasyingType) of EasingReal constructor is not an integer value");
		return 0;
	}
	
	EasingType easingType = static_cast<EasingType>(type);
	
	// Verify that the easing type is in range
	if (easingType < 0 || easingType >= EasingType_Count)
	{
		TT_PANIC("Parameter 4 (EasingType) of EasingReal constructor has an invalid value '%d'. "
		         "The value should be in the range [0..%d]", type, EasingType_Count-1);
		return 0;
	}
	
	return new tt::math::interpolation::Easing<real>(begin, end, duration, easingType);
}


void bindInterpolation(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace tt::math::interpolation;
	
	// Bind real variant of Easing
	TT_SQBIND_INIT_NAME(Easing<real>, "EasingReal");
	TT_SQBIND_SET_CONSTRUCTOR(Easing<real>, easingReal_constructor);
	
	TT_SQBIND_METHOD(Easing<real>, getBeginValue);
	TT_SQBIND_METHOD(Easing<real>, getEndValue);
	TT_SQBIND_METHOD(Easing<real>, getDuration);
	TT_SQBIND_METHOD(Easing<real>, getType);
	TT_SQBIND_METHOD(Easing<real>, getValue);
	TT_SQBIND_METHOD(Easing<real>, getTime);
	TT_SQBIND_METHOD(Easing<real>, update);
	
	// Bind helper function
	TT_SQBIND_FUNCTION(getEasingTypeFromName);
	
	// Bind all ints
	TT_SQBIND_CONSTANT(EasingType_Linear);
	TT_SQBIND_CONSTANT(EasingType_QuadraticIn);
	TT_SQBIND_CONSTANT(EasingType_QuadraticOut);
	TT_SQBIND_CONSTANT(EasingType_QuadraticInOut);
	TT_SQBIND_CONSTANT(EasingType_CubicIn);
	TT_SQBIND_CONSTANT(EasingType_CubicOut);
	TT_SQBIND_CONSTANT(EasingType_CubicInOut);
	TT_SQBIND_CONSTANT(EasingType_QuarticIn);
	TT_SQBIND_CONSTANT(EasingType_QuarticOut);
	TT_SQBIND_CONSTANT(EasingType_QuarticInOut);
	TT_SQBIND_CONSTANT(EasingType_QuinticIn);
	TT_SQBIND_CONSTANT(EasingType_QuinticOut);
	TT_SQBIND_CONSTANT(EasingType_QuinticInOut);
	TT_SQBIND_CONSTANT(EasingType_SinusoidalIn);
	TT_SQBIND_CONSTANT(EasingType_SinusoidalOut);
	TT_SQBIND_CONSTANT(EasingType_SinusoidalInOut);
	TT_SQBIND_CONSTANT(EasingType_ExponentialIn);
	TT_SQBIND_CONSTANT(EasingType_ExponentialOut);
	TT_SQBIND_CONSTANT(EasingType_ExponentialInOut);
	TT_SQBIND_CONSTANT(EasingType_CircularIn);
	TT_SQBIND_CONSTANT(EasingType_CircularOut);
	TT_SQBIND_CONSTANT(EasingType_CircularInOut);
	TT_SQBIND_CONSTANT(EasingType_BackIn);
	TT_SQBIND_CONSTANT(EasingType_BackOut);
	TT_SQBIND_CONSTANT(EasingType_BackInOut);
}

//--------------------------------------------------------------------------------------------------
// Random

real frnd()
{
	return tt::math::Random::getStatic().getNormalizedNext();
}


real frnd_minmax(real p_lower, real p_upper)
{
	return (frnd() * (p_upper-p_lower)) + p_lower;
}


u32 rnd()
{
	return tt::math::Random::getStatic().getNext();
}


u32 rnd_minmax(u32 p_lower, u32 p_upper)
{
	return tt::math::Random::getStatic().getNext(p_lower, p_upper + 1);
}


void bindRandom(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_FUNCTION(frnd);
	TT_SQBIND_FUNCTION(frnd_minmax);
	TT_SQBIND_FUNCTION(rnd);
	TT_SQBIND_FUNCTION(rnd_minmax);
}


void bindLeaderboardsTypes(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace tt::score;
	
	TT_SQBIND_CONSTANT_NAME(DownloadRequestSource_Game,                 "DownloadRequestSource_Game");
	TT_SQBIND_CONSTANT_NAME(DownloadRequestSource_User,                 "DownloadRequestSource_User");
	
	TT_SQBIND_CONSTANT_NAME(DownloadRequestRangeType_Global,            "DownloadRequestRangeType_Global");
	TT_SQBIND_CONSTANT_NAME(DownloadRequestRangeType_GlobalAroundUser,  "DownloadRequestRangeType_GlobalAroundUser");
	TT_SQBIND_CONSTANT_NAME(DownloadRequestRangeType_Friends,           "DownloadRequestRangeType_Friends");
	TT_SQBIND_CONSTANT_NAME(DownloadRequestRangeType_FriendsAroundUser, "DownloadRequestRangeType_FriendsAroundUser");
}


void bindStatsTypes(const tt::script::VirtualMachinePtr& p_vm)
{
	// Stats
	TT_SQBIND_SETVM(p_vm);
	using namespace stats;
	
	TT_SQBIND_CONSTANT(StatType_Int);
	TT_SQBIND_CONSTANT(StatType_Float);
	//TT_SQBIND_CONSTANT(StatType_AverageRate);
}


void bindKeyTypes(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	using namespace tt::input;
	
	TT_SQBIND_CONSTANT(Key_Backspace);
	TT_SQBIND_CONSTANT(Key_Tab);
	TT_SQBIND_CONSTANT(Key_Clear);  // FIXME: What is the "clear" key?
	TT_SQBIND_CONSTANT(Key_Enter); // Return/Enter key
	TT_SQBIND_CONSTANT(Key_Shift);   // Shift key
	TT_SQBIND_CONSTANT(Key_Control); // Ctrl key
	TT_SQBIND_CONSTANT(Key_Alt);     // Alt key
	TT_SQBIND_CONSTANT(Key_Break);   // Pause/Break key
	TT_SQBIND_CONSTANT(Key_CapsLock);
	TT_SQBIND_CONSTANT(Key_Escape);
	TT_SQBIND_CONSTANT(Key_Space);
	TT_SQBIND_CONSTANT(Key_PageUp);
	TT_SQBIND_CONSTANT(Key_PageDown);
	TT_SQBIND_CONSTANT(Key_End);
	TT_SQBIND_CONSTANT(Key_Home);
	TT_SQBIND_CONSTANT(Key_Left);
	TT_SQBIND_CONSTANT(Key_Up);
	TT_SQBIND_CONSTANT(Key_Right);
	TT_SQBIND_CONSTANT(Key_Down);
	TT_SQBIND_CONSTANT(Key_Insert);
	TT_SQBIND_CONSTANT(Key_Delete);
	TT_SQBIND_CONSTANT(Key_0);
	TT_SQBIND_CONSTANT(Key_1);
	TT_SQBIND_CONSTANT(Key_2);
	TT_SQBIND_CONSTANT(Key_3);
	TT_SQBIND_CONSTANT(Key_4);
	TT_SQBIND_CONSTANT(Key_5);
	TT_SQBIND_CONSTANT(Key_6);
	TT_SQBIND_CONSTANT(Key_7);
	TT_SQBIND_CONSTANT(Key_8);
	TT_SQBIND_CONSTANT(Key_9);
	TT_SQBIND_CONSTANT(Key_A);
	TT_SQBIND_CONSTANT(Key_B);
	TT_SQBIND_CONSTANT(Key_C);
	TT_SQBIND_CONSTANT(Key_D);
	TT_SQBIND_CONSTANT(Key_E);
	TT_SQBIND_CONSTANT(Key_F);
	TT_SQBIND_CONSTANT(Key_G);
	TT_SQBIND_CONSTANT(Key_H);
	TT_SQBIND_CONSTANT(Key_I);
	TT_SQBIND_CONSTANT(Key_J);
	TT_SQBIND_CONSTANT(Key_K);
	TT_SQBIND_CONSTANT(Key_L);
	TT_SQBIND_CONSTANT(Key_M);
	TT_SQBIND_CONSTANT(Key_N);
	TT_SQBIND_CONSTANT(Key_O);
	TT_SQBIND_CONSTANT(Key_P);
	TT_SQBIND_CONSTANT(Key_Q);
	TT_SQBIND_CONSTANT(Key_R);
	TT_SQBIND_CONSTANT(Key_S);
	TT_SQBIND_CONSTANT(Key_T);
	TT_SQBIND_CONSTANT(Key_U);
	TT_SQBIND_CONSTANT(Key_V);
	TT_SQBIND_CONSTANT(Key_W);
	TT_SQBIND_CONSTANT(Key_X);
	TT_SQBIND_CONSTANT(Key_Y);
	TT_SQBIND_CONSTANT(Key_Z);
	TT_SQBIND_CONSTANT(Key_Numpad0);
	TT_SQBIND_CONSTANT(Key_Numpad1);
	TT_SQBIND_CONSTANT(Key_Numpad2);
	TT_SQBIND_CONSTANT(Key_Numpad3);
	TT_SQBIND_CONSTANT(Key_Numpad4);
	TT_SQBIND_CONSTANT(Key_Numpad5);
	TT_SQBIND_CONSTANT(Key_Numpad6);
	TT_SQBIND_CONSTANT(Key_Numpad7);
	TT_SQBIND_CONSTANT(Key_Numpad8);
	TT_SQBIND_CONSTANT(Key_Numpad9);
	TT_SQBIND_CONSTANT(Key_Multiply); // asterisk ('*')
	TT_SQBIND_CONSTANT(Key_Slash); // slash/divide key ('/')
	TT_SQBIND_CONSTANT(Key_F1);
	TT_SQBIND_CONSTANT(Key_F2);
	TT_SQBIND_CONSTANT(Key_F3);
	TT_SQBIND_CONSTANT(Key_F4);
	TT_SQBIND_CONSTANT(Key_F5);
	TT_SQBIND_CONSTANT(Key_F6);
	TT_SQBIND_CONSTANT(Key_F7);
	TT_SQBIND_CONSTANT(Key_F8);
	TT_SQBIND_CONSTANT(Key_F9);
	TT_SQBIND_CONSTANT(Key_F10);
	TT_SQBIND_CONSTANT(Key_F11);
	TT_SQBIND_CONSTANT(Key_F12);
	TT_SQBIND_CONSTANT(Key_NumLock);
	TT_SQBIND_CONSTANT(Key_ScrollLock);
	TT_SQBIND_CONSTANT(Key_Semicolon);  // ;
	TT_SQBIND_CONSTANT(Key_Plus);
	TT_SQBIND_CONSTANT(Key_Comma);
	TT_SQBIND_CONSTANT(Key_Minus);
	TT_SQBIND_CONSTANT(Key_Period);
	TT_SQBIND_CONSTANT(Key_SlashQuestionmark);  // The /? key (on US keyboards)
	TT_SQBIND_CONSTANT(Key_Grave);      // Key containing tilde on US keyboards ('`')
	TT_SQBIND_CONSTANT(Key_LeftSquareBracket);
	TT_SQBIND_CONSTANT(Key_Backslash);
	TT_SQBIND_CONSTANT(Key_RightSquareBracket);
	TT_SQBIND_CONSTANT(Key_Apostrophe);
}


//--------------------------------------------------------------------------------------------------
// Time

void bindTime(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(Time);
	TT_SQBIND_STATIC_METHOD(Time, getMilliSeconds);
	TT_SQBIND_STATIC_METHOD(Time, getSeconds);
	TT_SQBIND_STATIC_METHOD(Time, getNowAsString);
}


u32 Time::getMilliSeconds()
{
	return static_cast<u32>(tt::system::Time::getInstance()->getMilliSeconds());
}


u32 Time::getSeconds()
{
	return static_cast<u32>(tt::system::Time::getInstance()->getSeconds());
}


std::string Time::getNowAsString()
{
	return tt::system::Time::getInstance()->getNowAsString();
}

//--------------------------------------------------------------------------------------------------
// Input

void bindInput(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::input;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_FUNCTION(getCurrentControllerType);
	
	TT_SQBIND_CONSTANT(ControllerType_Keyboard);
	TT_SQBIND_CONSTANT(ControllerType_GenericController);
	TT_SQBIND_CONSTANT(ControllerType_Xbox360Controller);
}


//--------------------------------------------------------------------------------------------------
// Platform

void bindPlatform(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::app;
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_FUNCTION(getPlatform);
	
	TT_SQBIND_CONSTANT(Platform_WIN);
	TT_SQBIND_CONSTANT(Platform_MAC);
	TT_SQBIND_CONSTANT(Platform_LNX);
}


//--------------------------------------------------------------------------------------------------
// Wrappers

void bindSharedWrappers(const tt::script::VirtualMachinePtr& p_vm)
{
	using namespace tt::script::wrappers;
	VisualBoyWrapper::bind(p_vm);
}

// Namespace end
}
}
}
