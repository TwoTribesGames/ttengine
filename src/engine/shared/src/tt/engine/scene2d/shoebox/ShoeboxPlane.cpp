#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ColorAnimationStack2D.h>
#include <tt/platform/tt_printf.h>

#include <tt/engine/scene2d/shoebox/ShoeboxPlane.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {


//--------------------------------------------------------------------------------------------------
// Public member functions


ShoeboxPlane::~ShoeboxPlane()
{
}


bool ShoeboxPlane::handleEvent(const std::string& p_event, const std::string& p_param)
{
	if (p_event == "show")
	{
		m_visible = true;
	}
	else if (p_event == "hide")
	{
		m_visible = false;
	}
	else if (p_event == "start")
	{
		if (getColorAnimation()   != 0) getColorAnimation()  ->start();
		if (getTextureAnimation() != 0) getTextureAnimation()->start();
		if (getAnimations()       != 0) getAnimations()      ->start();
	}
	else if (p_event == "stop")
	{
		if (getColorAnimation()   != 0) getColorAnimation()  ->stop();
		if (getTextureAnimation() != 0) getTextureAnimation()->stop();
		if (getAnimations()       != 0) getAnimations()      ->stop();
	}
	else if (p_event == "reset")
	{
		if (getColorAnimation()   != 0) getColorAnimation()  ->reset();
		if (getTextureAnimation() != 0) getTextureAnimation()->reset();
		if (getAnimations()       != 0) getAnimations()      ->reset();
	}
	else if (p_event == "pause")
	{
		if (getColorAnimation()   != 0) getColorAnimation()  ->pause();
		if (getTextureAnimation() != 0) getTextureAnimation()->pause();
		if (getAnimations()       != 0) getAnimations()      ->pause();
	}
	else if (p_event == "resume")
	{
		if (getColorAnimation()   != 0) getColorAnimation()  ->resume();
		if (getTextureAnimation() != 0) getTextureAnimation()->resume();
		if (getAnimations()       != 0) getAnimations()      ->resume();
	}
	else if (p_event == "set-directiontype")
	{
		anim2d::Animation2D::DirectionType type  = 
			anim2d::Animation2D::directionTypeFromString(p_param);
		
		if (getColorAnimation()   != 0) getColorAnimation()  ->setAnimationDirectionType(type);
		if (getTextureAnimation() != 0) getTextureAnimation()->setAnimationDirectionType(type);
		if (getAnimations()       != 0) getAnimations()      ->setAnimationDirectionType(type);
	}
	else if (p_event == "set-timetype")
	{
		anim2d::Animation2D::TimeType type = 
			anim2d::Animation2D::timeTypeFromString(p_param);
		
		if (getColorAnimation()   != 0) getColorAnimation()  ->setAnimationTimeType(type);
		if (getTextureAnimation() != 0) getTextureAnimation()->setAnimationTimeType(type);
		if (getAnimations()       != 0) getAnimations()      ->setAnimationTimeType(type);
	}
	else if (p_event == "set-tweentype")
	{
		anim2d::TweenType type = 
			anim2d::Tween::tweenTypeFromString(p_param);
		
		if (getColorAnimation()   != 0) getColorAnimation()  ->setAnimationTweenType(type);
		if (getTextureAnimation() != 0) getTextureAnimation()->setAnimationTweenType(type);
		if (getAnimations()       != 0) getAnimations()      ->setAnimationTweenType(type);
	}
	else
	{
		TT_PANIC("Unhandled event '%s'", p_event.c_str());
		return false;
	}
	
	return true;
}


// Namespace end
}
}
}
}
