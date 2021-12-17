#if !defined(INC_TT_PRES_ANIM2D_STACKBASE_H)
#define INC_TT_PRES_ANIM2D_STACKBASE_H


#include <vector>

#include <tt/code/BitMask.h>
#include <tt/code/helpers.h>
#include <tt/code/ErrorStatus.h>
#include <tt/pres/anim2d/fwd.h>
#include <tt/pres/anim2d/Sequence.h>
#include <tt/pres/DataTags.h>
#include <tt/pres/PresentationValue.h>


namespace tt {
namespace pres {
namespace anim2d {


template<typename RawType>
class StackBase
{
public:
	StackBase();
	virtual ~StackBase() { }
	
	/*! \brief Removes all elements of this Stack */
	virtual inline void clear() { code::helpers::freeContainer(m_allAnimations);
	                              code::helpers::freeContainer(m_allSequences);
	                              code::helpers::freeContainer(m_activeAnimations);
	                              code::helpers::freeContainer(m_activeSequences); }
	
	/*! \brief Get wether this stack has any elements */
	inline bool allEmpty()    const { return m_allAnimations.empty()    && m_allSequences.empty();    }
	
	/*! \brief Get wether this stack has any active elements */
	inline bool activeEmpty() const { return m_activeAnimations.empty() && m_activeSequences.empty(); }
	
	/*! \brief Update logic of this animation.
	    \param p_delta Time passed in seconds.*/
	virtual void update(real p_delta, bool p_flagsOnly = false);
	
	/*! \brief Returns whether any of the animations are active. 
		If all Stack elements are looping this will always return true.*/
	inline bool isActive() const { return m_flags.checkFlag(Flags_IsActive); }
	
	/*! \brief Returns whether one of the active animations is looping */
	inline bool isLooping() const { return m_flags.checkFlag(Flags_IsLooping); }
	
	/*! \brief Returns whether all the animations are looping */
	inline bool isAllLooping() const { return m_flags.checkFlag(Flags_IsAllLooping); }
	
	/*! \brief Returns whether any of the animations are started with the tag filtering.
		Used for knowing if the data acquired from this stack should be used.*/
	inline bool hasNameOrTagMatch() const { return m_hasNameOrTagMatch; }
	
	/*! \brief Returns the tags of this Stack*/
	virtual DataTags getTags() const;
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_customValues Any custom values found, will be replaced by values in this container. */
	virtual void start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name);
	
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
	typedef typename tt_ptr<RawType>::shared      Type;
	typedef std::vector<Type>                     Stack;
	typedef Sequence<Type>                        SequenceType;
	typedef typename tt_ptr<SequenceType>::shared SequencePtr;
	typedef std::vector<SequencePtr>              SequenceStack;
	
	
	Stack         m_allAnimations;
	SequenceStack m_allSequences;
	
	Stack         m_activeAnimations; // A subset of m_allAnimations. Only the active animations are stored here.
	SequenceStack m_activeSequences;  // A subset of m_allSequences.  Only the active sequences are stored here.
	
	//Enable copy
	StackBase(const StackBase& p_rhs);
	
	//FIXME: Hack: this implementation doesn't copy the m_animations vector
	StackBase(const StackBase& p_rhs, bool p_initOnly);
	
private:
	enum Flags
	{
		Flags_IsActive,
		Flags_IsAllLooping,
		Flags_IsLooping,
		
		Flags_Count
	};
	typedef code::BitMask<Flags, Flags_Count> FlagsMask;
	
	template<typename IteratorType>
	void updateFlags(typename Stack::size_type& p_loopingCount, IteratorType& p_it);
	
	//Disable assignment
	StackBase& operator=(const StackBase&);
	
	// bookkeeping variables
	FlagsMask m_flags;
	bool m_hasNameOrTagMatch;
	
#if !defined(TT_BUILD_FINAL)
	bool m_isUpdating; // Used to make sure we're not modifying containers while looping through them.
#endif
};




#include "StackBase.inl"


//namespace end
}
}
}

#endif // !defined(INC_TT_PRES_ANIM2D_STACKBASE_H)
