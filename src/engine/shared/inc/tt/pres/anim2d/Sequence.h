#if !defined(INC_TT_PRES_ANIM2D_SEQUENCE_H)
#define INC_TT_PRES_ANIM2D_SEQUENCE_H

#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/DataTags.h>


namespace tt {
namespace pres {
namespace anim2d {


template<typename Type>
class Sequence
{
public:
	typedef std::vector<Type> Stack;
	
	Sequence();
	
	/*! \brief Update logic of this animation.
	    \param p_delta Time passed in seconds.
	    \param p_flagsOnly Whether to only update the islooping/isActive... Flags.*/
	void update(real p_delta, bool p_flagsOnly = false);
	
	/*! \brief Returns whether the sequence is started with the tag filtering.
	           Used for knowing if the data acquired from this sequence should be used.*/
	bool hasNameOrTagMatch() const;
	
	/*! \brief Returns the currently active animation in the sequence. */
	const Type& getActive();
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_presObj Owning presentation Object, Used for replacing custom values.
	    \param p_name Name of the animation to start. */
	void start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name);
	
	/*! \brief Stops the animation.*/
	void stop();
	
	/*! \brief Pauses the animation.*/
	void pause();
	
	/*! \brief Resumes the animation.*/
	void resume();
	
	/*! \brief Unpauses and stops the animation, sets time to 0.*/
	void reset();
	
	/*! \brief Returns whether this sequence is active.*/
	inline bool isActive() const { return m_isActive; }
	
	/*! \brief Returns whether this sequence has a looping animation */
	inline bool isLooping() const { return m_isLooping; }

	inline bool isEmpty() const { return m_animations.empty(); }
	inline const Type& getFront() { TT_ASSERT(isEmpty() == false); return m_animations.front(); }
	
	/*! \brief Adds an animation to this Sequence. */
	inline void addAnimation(const Type& p_animation) { m_animations.push_back(p_animation); }
	
	inline const Stack& getAnimations() const { return m_animations; }
	
	Sequence* clone() const { return new Sequence(*this); }
	
protected:
	Stack m_animations;
	
	//Enable copy
	Sequence(const Sequence& p_rhs);
	
private:
	bool isActiveAnimationIndexValid() { return m_activeAnimationIndex < m_animations.size(); }
	//Disable assignment
	Sequence& operator=(const Sequence&);
	
	// bookkeeping variables
	bool m_isActive;
	bool m_isLooping;
	
	typename Stack::size_type m_activeAnimationIndex;
	Tags                      m_startTags;
	std::string               m_startName;
	PresentationObject*       m_presObj; // FIXME: Get rid of this raw pointer
};




#include "Sequence.inl"


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_SEQUENCE_H)
