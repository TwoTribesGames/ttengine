#if !defined(INC_TT_MENU_ELEMENTS_TRANSLATIONANIMATIONINTERFACE_H)
#define INC_TT_MENU_ELEMENTS_TRANSLATIONANIMATIONINTERFACE_H


#include <tt/platform/tt_types.h>
#include <tt/math/Vector2.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Interface class for translation animations. */
class TranslationAnimationInterface
{
public:
	TranslationAnimationInterface() { }
	virtual ~TranslationAnimationInterface() { }
	
	virtual void setStartPos(const math::Vector2& p_pos) = 0;
	virtual math::Vector2 getStartPos() const = 0;
	
	virtual void setDestinationPos(const math::Vector2& p_pos) = 0;
	virtual math::Vector2 getDestinationPos() const = 0;
	
	/*! \param p_duration Duration for the animation, in frames. */
	virtual void setDuration(s32 p_duration) = 0;
	virtual s32  getDuration() const = 0;
	
	virtual void update() = 0;
	
	virtual math::Vector2 getPos() const = 0;
	
	virtual bool isDone() const = 0;
	
	virtual void start() = 0;
	
private:
	// No copying
	TranslationAnimationInterface(const TranslationAnimationInterface&);
	const TranslationAnimationInterface& operator=(const TranslationAnimationInterface&);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_TRANSLATIONANIMATIONINTERFACE_H)
