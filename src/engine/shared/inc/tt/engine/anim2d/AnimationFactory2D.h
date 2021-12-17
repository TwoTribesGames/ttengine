#if !defined(INC_TT_ENGINE_ANIM2D_ANIMATIONFACTORY2D_H)
#define INC_TT_ENGINE_ANIM2D_ANIMATIONFACTORY2D_H

#include <string>

#include <tt/engine/anim2d/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace engine {
namespace anim2d {

class AnimationFactory2D
{
public:
	static PositionAnimation2DPtr create(const std::string& p_type);
	static PositionAnimation2DPtr create(u8 p_type);

	/*/brief Creates an AnimationStack2D from file
		/param p_file path to the animation file without extension
		/return returns a pointer to an AnimationStack2D	*/
	static AnimationStack2DPtr createStackFromFile(const std::string& p_file);
	
	static u8 getBinaryType(const ConstPositionAnimation2DPtr& p_anim);
	static std::string getTextType(const ConstPositionAnimation2DPtr& p_anim);
	
private:
	AnimationFactory2D();
	AnimationFactory2D(const AnimationFactory2D&);
	~AnimationFactory2D();
	const AnimationFactory2D& operator=(const AnimationFactory2D&);
};

//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_ANIMATIONFACTORY2D_H)
