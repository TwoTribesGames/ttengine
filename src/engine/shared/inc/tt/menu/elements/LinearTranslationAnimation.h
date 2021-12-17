#if !defined(INC_TT_MENU_ELEMENTS_LINEARTRANSLATIONANIMATION_H)
#define INC_TT_MENU_ELEMENTS_LINEARTRANSLATIONANIMATION_H


#include <tt/menu/elements/TranslationAnimationInterface.h>


namespace tt {
namespace menu {
namespace elements {

/*! \brief Animates the position from one point to another. */
class LinearTranslationAnimation : public TranslationAnimationInterface
{
public:
	LinearTranslationAnimation();
	virtual ~LinearTranslationAnimation();
	
	virtual void setStartPos(const math::Vector2& p_pos);
	virtual math::Vector2 getStartPos() const;
	
	virtual void setDestinationPos(const math::Vector2& p_pos);
	virtual math::Vector2 getDestinationPos() const;
	
	virtual void setDuration(s32 p_duration);
	virtual s32  getDuration() const;
	
	virtual void update();
	
	virtual math::Vector2 getPos() const;
	
	virtual bool isDone() const;
	
	virtual void start();
	
private:
	// No copying
	LinearTranslationAnimation(const LinearTranslationAnimation&);
	const LinearTranslationAnimation& operator=(const LinearTranslationAnimation&);
	
	
	math::Vector2 m_pos;        //!< Current position in the animation.
	math::Vector2 m_destPos;    //!< Position to move to.
	math::Vector2 m_startPos;   //!< Position to start at.
	s32           m_duration;   //!< Duration for the animation, in frames.
	s32           m_frame;      //!< Current frame.
	math::Vector2 m_frameDelta; //!< Amount to move each frame.
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_LINEARTRANSLATIONANIMATION_H)
