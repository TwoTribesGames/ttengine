#if !defined(INC_TT_ENGINE_ANIM2D_STACKBASE_H)
#define INC_TT_ENGINE_ANIM2D_STACKBASE_H
#include <vector>

#include <tt/code/helpers.h>
#include <tt/code/ErrorStatus.h>
#include <tt/engine/anim2d/fwd.h>


namespace tt {
namespace engine {
namespace anim2d {


template<typename Type>
class StackBase
{
public:
	StackBase();
	virtual ~StackBase() { }
	
	/*! \brief Removes all elements of this Stack */
	virtual inline void clear() { code::helpers::freeContainer(m_animations); }
	
	/*! \brief Get wether this stack has any elements */
	virtual inline bool empty() const { return m_animations.empty(); }
	
	/*! \brief Update logic of this animation.
	    \param p_delta Time passed in seconds.*/
	virtual void update(real p_delta, bool p_flagsOnly = false);
	
	/*! \brief Returns whether any of the animations are active. 
		If all Stack elements are looping this will always return true.*/
	inline bool isActive() const { return m_isActive; }
	
	/*! \brief Returns whether all the animations are looping */
	inline bool isAllLooping() const { return m_isAllLooping; }
	
	/*! \brief Returns whether any of the animations are started with the tag filtering.
		Used for knowing if the data acquired from this stack should be used.*/
	inline bool isTagged() const { return m_isTagged; }
	
	/*! \brief Returns the tags of this Stack*/
	virtual Tags getTags() const;
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of strings representing Tags */ 
	virtual void start(const Tags& p_tags);
	
	/*! \brief Starts all the animations without tags. */ 
	virtual void start();
	
	/*! \brief Stops the animation.*/
	virtual void stop();
	
	/*! \brief Pauses the animation.*/
	virtual void pause();
	
	/*! \brief Resumes the animation.*/
	virtual void resume();
	
	/*! \brief Unpauses and stops the animation, sets time to 0.*/
	virtual void reset();
	
	/*! \brief Saves the stack to a buffer.*/
	virtual bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const = 0;
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const = 0;
	
protected:
	typedef std::vector<Type> Stack;
	
	Stack m_animations;
	
	//Enable copy
	StackBase(const StackBase& p_rhs);
	
private:
	//Disable assignment
	StackBase& operator=(const StackBase&);
	
	// bookkeeping variables
	bool m_isActive;
	bool m_isTagged;
	bool m_isAllLooping;
};




#include "StackBase.inl"


//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_ANIM2D_STACKBASE_H)
