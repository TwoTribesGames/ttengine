#include <string>

#include <tt/math/interpolation.h>

namespace tt {
namespace math {
namespace interpolation {

const char* getEasingTypeName(EasingType p_type)
{
	switch (p_type)
	{
	case EasingType_Linear:           return "Linear";
	case EasingType_QuadraticIn:      return "QuadraticIn";
	case EasingType_QuadraticOut:     return "QuadraticOut";
	case EasingType_QuadraticInOut:   return "QuadraticInOut";
	case EasingType_CubicIn:          return "CubicIn";
	case EasingType_CubicOut:         return "CubicOut";
	case EasingType_CubicInOut:       return "CubicInOut";
	case EasingType_QuarticIn:        return "QuarticIn";
	case EasingType_QuarticOut:       return "QuarticOut";
	case EasingType_QuarticInOut:     return "QuarticInOut";
	case EasingType_QuinticIn:        return "QuinticIn";
	case EasingType_QuinticOut:       return "QuinticOut";
	case EasingType_QuinticInOut:     return "QuinticInOut";
	case EasingType_SinusoidalIn:     return "SinusoidalIn";
	case EasingType_SinusoidalOut:    return "SinusoidalOut";
	case EasingType_SinusoidalInOut:  return "SinusoidalInOut";
	case EasingType_ExponentialIn:    return "ExponentialIn";
	case EasingType_ExponentialOut:   return "ExponentialOut";
	case EasingType_ExponentialInOut: return "ExponentialInOut";
	case EasingType_CircularIn:       return "CircularIn";
	case EasingType_CircularOut:      return "CircularOut";
	case EasingType_CircularInOut:    return "CircularInOut";
	case EasingType_BackIn:           return "BackIn";
	case EasingType_BackOut:          return "BackOut";
	case EasingType_BackInOut:        return "BackInOut";
	
	default:
		TT_PANIC("Invalid EasingType: %d", p_type);
		return "";
	}
}


EasingType getEasingTypeFromName(const std::string& p_name)
{
	for (s32 i = 0; i < EasingType_Count; ++i)
	{
		const EasingType type = static_cast<EasingType>(i);
		if (p_name == getEasingTypeName(type))
		{
			return type;
		}
	}
	
	return EasingType_Invalid;
}


// Namespace end
}
}
}
