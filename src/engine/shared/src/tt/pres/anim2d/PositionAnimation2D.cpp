#include <tt/pres/anim2d/PositionAnimation2D.h>


namespace tt {
namespace pres {
namespace anim2d {

PositionAnimation2D::PositionAnimation2D()
:
Animation2D()
{
}


PositionAnimation2D::~PositionAnimation2D()
{
}


PositionAnimation2D::AnimationType PositionAnimation2D::animationTypeFromString(const std::string& p_name)
{
	if (p_name == "translation")     return AnimationType_Translation;
	if (p_name == "gametranslation") return AnimationType_GameTranslation;
	if (p_name == "rotation")        return AnimationType_Rotation;
	if (p_name == "scale")           return AnimationType_Scale;
	
	return static_cast<PositionAnimation2D::AnimationType>(-1);
}


std::string PositionAnimation2D::animationTypeToString(AnimationType p_animationtype)
{
	switch (p_animationtype)
	{
	case AnimationType_Translation:     return "translation";
	case AnimationType_GameTranslation: return "gametranslation";
	case AnimationType_Rotation:        return "rotation";
	case AnimationType_Scale:           return "scale";
	
	default: TT_PANIC("Unknown animationtype: %d", p_animationtype); return "unknown";
	}
}


PositionAnimation2D::PositionAnimation2D(const PositionAnimation2D& p_rhs)
:
Animation2D(p_rhs)
{
}

//namespace end
}
}
}
