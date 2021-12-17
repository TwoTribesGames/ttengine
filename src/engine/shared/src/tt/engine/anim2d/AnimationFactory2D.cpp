#include <tt/engine/anim2d/AnimationFactory2D.h>
#include <tt/engine/anim2d/AnimationStack2D.h>
#include <tt/engine/anim2d/ParticleAnimation2D.h>
#include <tt/engine/anim2d/RotationAnimation2D.h>
#include <tt/engine/anim2d/ScaleAnimation2D.h>
#include <tt/engine/anim2d/TranslationAnimation2D.h>
#include <tt/fs/fs.h>
#include <tt/platform/tt_error.h>
#include <tt/xml/XmlDocument.h>
#include <tt/xml/XmlNode.h>




namespace tt {
namespace engine {
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
		
	case PositionAnimation2D::AnimationType_Particles:
		return PositionAnimation2DPtr(new ParticleAnimation2D);
		
	default:
		TT_PANIC("Unknown AnimationType: %d gotten from str: '%s'.", animationType, p_type.c_str());
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
	
	case PositionAnimation2D::AnimationType_Particles:
		return PositionAnimation2DPtr(new ParticleAnimation2D);
	
	default:
		return PositionAnimation2DPtr();
	}
}


AnimationStack2DPtr AnimationFactory2D::createStackFromFile( const std::string& p_file )
{
	//create a new animation stack
	AnimationStack2DPtr animationStack(new AnimationStack2D);

	if(fs::fileExists(p_file + ".2dab"))
	{
		fs::FilePtr file(fs::open(p_file + ".2dab", tt::fs::OpenMode_Read));
		if(animationStack->load(file, false) == false)  // FIXME: Pass "invert Y" param to createStackFromFile?
		{
			TT_PANIC("binary animation file reading failed");
			return AnimationStack2DPtr();
		}
	}
	else 
	{
		TT_ASSERTMSG(fs::fileExists(p_file + ".2dax"),
		             "Could not find animation file '%s'.", p_file.c_str());

		// load the xml
		xml::XmlDocument doc(p_file + ".2dax");

		TT_ASSERTMSG(doc.getRootNode()->getName()=="animationstack",
			"Animation root node is not animationstack");

		// load the animationStack2D
		if(animationStack->load(doc.getRootNode(), false) == false)  // FIXME: Pass "invert Y" param to createStackFromFile?
		{
			TT_PANIC("xml animation file reading failed");
			return AnimationStack2DPtr();
		}
	}

	return animationStack;
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
