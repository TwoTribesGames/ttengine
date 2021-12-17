#if !defined(INC_TT_ENGINE_ANIM2D_STACKBASE_INL)
#define INC_TT_ENGINE_ANIM2D_STACKBASE_INL

#if !defined(INC_TT_ENGINE_ANIM2D_STACKBASE_H)
#error This inline file should only be included by its parent headerfile.
#endif


template<typename Type>
StackBase<Type>::StackBase()
:
m_animations(),
m_isActive(false),
m_isTagged(false),
m_isAllLooping(false)
{
}

// FIXME: replace Start/Stop/Reset update(0) with an updateFlags() function?
// FIXME: does load() from derived classes also need an update(0)?
// FIXME: add appendStack, clone and push_back?

template<typename Type>
void StackBase<Type>::update(real p_delta, bool p_flagsOnly)
{
	// reset bookkeeping flags
	m_isActive = false;
	m_isTagged = false;
	m_isAllLooping = false;
	
	typename Stack::size_type loopingCount(0);
	
	// Warning: code duplication; if you change ANYTHING in this if-clause, make sure it is also
	// changed in the else clause
	if (p_flagsOnly)
	{
		for (typename Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
		{
			if((*it)->isTagged())
			{
				m_isTagged = true;
				
				if ((*it)->isActive() && (*it)->isLooping() == false)
				{
					m_isActive = true;
				}
			}
			
			if((*it)->isLooping())
			{
				loopingCount++;
			}
		}
	}
	else
	{
		for (typename Stack::iterator it = m_animations.begin(); it != m_animations.end(); ++it)
		{
			if((*it)->isTagged())
			{
				(*it)->update(p_delta);
				
				m_isTagged = true;
				
				if ((*it)->isActive() && (*it)->isLooping() == false)
				{
					m_isActive = true;
				}
			}
			
			if((*it)->isLooping())
			{
				loopingCount++;
			}
		}
	}
	
	// check if this stack has only looping animations.
	if (loopingCount == m_animations.size())
	{
		m_isActive = true;
		m_isAllLooping = true;
	}
}


template<typename Type>
Tags StackBase<Type>::getTags() const
{
	Tags tags;
	for (typename Stack::const_iterator it = m_animations.begin(); it != m_animations.end(); ++it)
	{
		tags.insert((*it)->getTags().begin(), (*it)->getTags().end());
	}
	return tags;
}


template<typename Type>
void StackBase<Type>::start( const Tags& p_tags )
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->start(p_tags);
	}
	
	update(0, true);
}


template<typename Type>
void StackBase<Type>::start()
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->start();
	}
	
	update(0, true);
}


template<typename Type>
void StackBase<Type>::stop()
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->stop();
	}
	
	update(0, true);
}


template<typename Type>
void StackBase<Type>::pause()
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->pause();
	}
}


template<typename Type>
void StackBase<Type>::resume()
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->resume();
	}
}


template<typename Type>
void StackBase<Type>::reset()
{
	for(typename Stack::const_iterator it(m_animations.begin()) ; it != m_animations.end() ; ++it)
	{
		(*it)->reset();
	}
	
	update(0, true);
}


template<typename Type>
StackBase<Type>::StackBase(const StackBase& p_rhs)
:
m_animations(),
m_isActive(p_rhs.m_isActive),
m_isTagged(p_rhs.m_isTagged),
m_isAllLooping(p_rhs.m_isAllLooping)
{
	for(typename Stack::const_iterator it(p_rhs.m_animations.begin()); it != p_rhs.m_animations.end(); ++it)
	{
		m_animations.push_back(Type((*it)->clone()));
	}
}


#endif // !defined(INC_TT_ENGINE_ANIM2D_STACKBASE_INL)
