#if !defined(INC_TT_PRES_ANIM2D_STACKBASE_INL)
#define INC_TT_PRES_ANIM2D_STACKBASE_INL

#if !defined(INC_TT_PRES_ANIM2D_STACKBASE_H)
#error This inline file should only be included by its parent headerfile.
#endif


template<typename Type>
StackBase<Type>::StackBase()
:
m_allAnimations(),
m_allSequences(),
m_activeAnimations(),
m_activeSequences(),
m_flags(),
m_hasNameOrTagMatch(false)
#if !defined(TT_BUILD_FINAL)
,
m_isUpdating(false)
#endif
{
}

// FIXME: replace Start/Stop/Reset update(0) with an updateFlags() function?
// FIXME: does load() from derived classes also need an update(0)?
// FIXME: add appendStack, clone and push_back?

template<typename Type>
void StackBase<Type>::update(real p_delta, bool p_flagsOnly)
{
	// reset bookkeeping flags
	m_flags.resetAllFlags();
	
	typename Stack::size_type loopingCount(0);
	
#if !defined(TT_BUILD_FINAL)
	m_isUpdating = true;
#endif
	
	if (p_flagsOnly)
	{
		for (typename Stack::iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
		{
			updateFlags(loopingCount, *it);
		}
		for (typename SequenceStack::iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
		{
			(*it)->update(0, true);
			updateFlags(loopingCount, *it);
		}
	}
	else
	{
		for (typename Stack::iterator it = m_activeAnimations.begin(); it != m_activeAnimations.end(); ++it)
		{
			(*it)->update(p_delta);
			updateFlags(loopingCount, *it);
		}
		for (typename SequenceStack::iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
		{
			(*it)->update(p_delta);
			updateFlags(loopingCount, *it);
		}
	}
	
	// check if this stack has only looping animations.
	if (loopingCount != 0 && loopingCount == m_activeAnimations.size() + m_activeSequences.size())
	{
		m_flags.setFlags(FlagsMask(Flags_IsActive) | FlagsMask(Flags_IsAllLooping));
	}
	
#if !defined(TT_BUILD_FINAL)
	m_isUpdating = false;
#endif
}


template<typename Type>
DataTags StackBase<Type>::getTags() const
{
	DataTags tags;
	for (typename Stack::const_iterator it = m_allAnimations.begin(); it != m_allAnimations.end(); ++it)
	{
		tags.addDataTags((*it)->getTags());
	}
	
	for (typename SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		const typename SequenceType::Stack& animations((*it)->getAnimations());
		for (typename SequenceType::Stack::const_iterator sequenceIt = animations.begin();
		     sequenceIt != animations.end(); ++sequenceIt)
		{
			tags.addDataTags((*sequenceIt)->getTags());
		}
	}
	return tags;
}


template<typename Type>
void StackBase<Type>::start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name)
{
#if !defined(TT_BUILD_FINAL)
	TT_ASSERT(m_isUpdating == false);
#endif
	m_activeAnimations.clear();
	
	for(typename Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->start(p_tags, p_presObj, p_name);
		if ((*it)->hasNameOrTagMatch()) 
		{
			m_activeAnimations.push_back(*it);
		}
	}
	
	m_activeSequences.clear();
	
	for (typename SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		(*it)->start(p_tags, p_presObj, p_name);
		if ((*it)->hasNameOrTagMatch())
		{
			m_activeSequences.push_back(*it);
		}
	}
	
	m_hasNameOrTagMatch = (m_activeAnimations.empty() == false || 
	                       m_activeSequences.empty()  == false);
	
	update(0, true);
}


template<typename Type>
void StackBase<Type>::stop()
{
#if !defined(TT_BUILD_FINAL)
	TT_ASSERT(m_isUpdating == false);
#endif
	
	for(typename Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		(*it)->stop();
	}
	m_activeAnimations.clear();
	
	for (typename SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		(*it)->stop();
	}
	m_activeSequences.clear();
	
	update(0, true);
}


template<typename Type>
void StackBase<Type>::pause()
{
	for(typename Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		(*it)->pause();
	}
	
	for (typename SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		(*it)->pause();
	}
}


template<typename Type>
void StackBase<Type>::resume()
{
	for(typename Stack::const_iterator it(m_activeAnimations.begin()) ; it != m_activeAnimations.end() ; ++it)
	{
		(*it)->resume();
	}
	
	for (typename SequenceStack::const_iterator it = m_activeSequences.begin(); it != m_activeSequences.end(); ++it)
	{
		(*it)->resume();
	}
}


template<typename Type>
void StackBase<Type>::reset()
{
#if !defined(TT_BUILD_FINAL)
	TT_ASSERT(m_isUpdating == false);
#endif
	
	for(typename Stack::const_iterator it(m_allAnimations.begin()) ; it != m_allAnimations.end() ; ++it)
	{
		(*it)->reset();
	}
	m_activeAnimations.clear();
	
	for (typename SequenceStack::const_iterator it = m_allSequences.begin(); it != m_allSequences.end(); ++it)
	{
		(*it)->reset();
	}
	m_activeSequences.clear();
	
	update(0, true);
}


template<typename Type>
StackBase<Type>::StackBase(const StackBase& p_rhs)
:
m_allAnimations(),
m_allSequences(),
m_activeAnimations(),
m_activeSequences(),
m_flags(p_rhs.m_flags),
m_hasNameOrTagMatch(p_rhs.m_hasNameOrTagMatch)
#if !defined(TT_BUILD_FINAL)
,
m_isUpdating(false)
#endif
{
	for(typename Stack::const_iterator it(p_rhs.m_allAnimations.begin()); it != p_rhs.m_allAnimations.end(); ++it)
	{
		m_allAnimations.push_back(Type((*it)->clone()));
	}
	TT_ASSERT(m_activeAnimations.empty());
	
	for(typename SequenceStack::const_iterator it(p_rhs.m_allSequences.begin()); it != p_rhs.m_allSequences.end(); ++it)
	{
		m_allSequences.push_back(SequencePtr((*it)->clone()));
	}
	TT_ASSERT(m_activeSequences.empty());
}


// HACKFIX: FIXME: Come up with better solution to prevent copying of m_animations
//                 in derived classes
template<typename Type>
StackBase<Type>::StackBase(const StackBase& p_rhs, bool /*initonly*/)
:
m_allAnimations(),
m_allSequences(),
m_activeAnimations(),
m_activeSequences(),
m_flags(p_rhs.m_flags),
m_hasNameOrTagMatch(p_rhs.m_hasNameOrTagMatch)
#if !defined(TT_BUILD_FINAL)
,
m_isUpdating(false)
#endif
{
}


//--------------------------------------------------------------------------------------------------
// Private member functions


template<typename Type>
template<typename IteratorType>
void StackBase<Type>::updateFlags(typename Stack::size_type& p_loopingCount, IteratorType& p_it)
{
	TT_ASSERT(p_it->hasNameOrTagMatch());
	
	if(p_it->isLooping())
	{
		p_loopingCount++;
		m_flags.setFlag(Flags_IsLooping);
	}
	else if (p_it->isActive())
	{
		m_flags.setFlag(Flags_IsActive);
	}
}


#endif // !defined(INC_TT_PRES_ANIM2D_STACKBASE_INL)
