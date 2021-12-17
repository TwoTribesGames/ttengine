#include <tt/pres/anim2d/AnimationFactory2D.h>
#include <tt/pres/anim2d/AnimationStack2D.h>
#include <tt/pres/anim2d/RotationAnimation2D.h>
#include <tt/pres/anim2d/ScaleAnimation2D.h>
#include <tt/pres/anim2d/TranslationAnimation2D.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>




namespace tt {
namespace pres {
namespace anim2d {

PositionAnimation2DPtr AnimationFactory2D::create(const std::string& p_type)
{
	PositionAnimation2D::AnimationType animationType(
		PositionAnimation2D::animationTypeFromString(p_type));
	
	switch (animationType)
	{
	case PositionAnimation2D::AnimationType_Translation:
		return PositionAnimation2DPtr(new TranslationAnimation2D(false));
		
	case PositionAnimation2D::AnimationType_GameTranslation:
		return PositionAnimation2DPtr(new TranslationAnimation2D(true));
		
	case PositionAnimation2D::AnimationType_Rotation:
		return PositionAnimation2DPtr(new RotationAnimation2D);
		
	case PositionAnimation2D::AnimationType_Scale:
		return PositionAnimation2DPtr(new ScaleAnimation2D);
		
	default:
		// FIXME: Panic here?
		return PositionAnimation2DPtr();
	}
}


PositionAnimation2DPtr AnimationFactory2D::create(u8 p_type)
{
	switch (p_type)
	{
	case PositionAnimation2D::AnimationType_Translation:
		return PositionAnimation2DPtr(new TranslationAnimation2D(false));
		
	case PositionAnimation2D::AnimationType_GameTranslation:
		return PositionAnimation2DPtr(new TranslationAnimation2D(true));
		
	case PositionAnimation2D::AnimationType_Rotation:
		return PositionAnimation2DPtr(new RotationAnimation2D);
	
	case PositionAnimation2D::AnimationType_Scale:
		return PositionAnimation2DPtr(new ScaleAnimation2D);
	
	default:
		return PositionAnimation2DPtr();
	}
}


u8 AnimationFactory2D::getBinaryType(const ConstPositionAnimation2DPtr& p_anim)
{
	if (p_anim == 0)
	{
		return 0;
	}
	return static_cast<u8>(p_anim->getSortWeight());
}


std::string AnimationFactory2D::getTextType(const ConstPositionAnimation2DPtr& p_anim)
{
	if (p_anim == 0)
	{
		return std::string();
	}
	switch (p_anim->getSortWeight())
	{
	case 1: return "translation";
	case 2: return "rotation";
	case 3: return "scale";
	default: return std::string();
	}
}


//namespace end
}
}
}
