#if !defined(INC_TT_PRES_ANIM2D_ANIMATIONFACTORY2D_H)
#define INC_TT_PRES_ANIM2D_ANIMATIONFACTORY2D_H

#include <string>

#include <tt/pres/anim2d/fwd.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace pres {
namespace anim2d {

class AnimationFactory2D
{
public:
	static PositionAnimation2DPtr create(const std::string& p_type);
	static PositionAnimation2DPtr create(u8 p_type);
	
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

#endif // !defined(INC_TT_PRES_ANIM2D_ANIMATIONFACTORY2D_H)
