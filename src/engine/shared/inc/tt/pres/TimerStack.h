#if !defined(INC_TT_PRES_TIMERSTACK_H)
#define INC_TT_PRES_TIMERSTACK_H

#include <tt/pres/anim2d/StackBase.h>
#include <tt/pres/fwd.h>
#include <tt/pres/Timer.h>


namespace tt {
namespace pres {

class TimerStack : public anim2d::StackBase<Timer>
{
public:
	TimerStack(){}
	virtual ~TimerStack(){}
	
	inline void push_back(const TimerPtr& p_timer)
	{
		m_allAnimations.push_back(p_timer);
		TT_ASSERT(m_activeAnimations.empty());
	}
	
	/*! \brief Saves the stack to a buffer.*/
	virtual bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus ) const;
	
	/*! \brief Loads the timer stack from a buffer.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags tags that all loaded animations should be applied to
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load( const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	           const DataTags& p_applyTags, const Tags& p_acceptedTags, 
	           code::ErrorStatus* p_errStatus);
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	virtual size_t getBufferSize() const;
	
	/*! \brief adds two stacks together. .*/
	void appendStack(TimerStack& p_other);
	
	TimerStack(const TimerStack& p_rhs);
private:
	
	TimerStack& operator=(const TimerStack&); // disable assignment
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TIMERSTACK_H)
