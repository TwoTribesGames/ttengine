#if !defined(INC_TT_PRES_ANIM2D_SEQUENCE_INL)
#define INC_TT_PRES_ANIM2D_SEQUENCE_INL

#if !defined(INC_TT_PRES_ANIM2D_SEQUENCE_H)
#error This inline file should only be included by its parent headerfile.
#endif


template<typename Type>
Sequence<Type>::Sequence()
:
m_animations(),
m_isActive(false),
m_isLooping(false),
m_activeAnimationIndex(0),
m_startTags(),
m_startName(),
m_presObj()
{
}


template<typename Type>
void Sequence<Type>::update(real p_delta, bool p_flagsOnly)
{
	TT_ASSERT(isActiveAnimationIndexValid());
	
	// during loading any looping animation is moved to the back of the sequence
	m_isLooping = m_animations.back()->isLooping();
	
	if (p_flagsOnly)
	{
		if (m_isActive)
		{
			m_isActive = m_animations[m_activeAnimationIndex]->isActive() || // Current animation is active or,
			             m_activeAnimationIndex + 1 < m_animations.size();   // There is another animation.
		}
	}
	else
	{
		m_animations[m_activeAnimationIndex]->update(p_delta);
		
		if (m_animations[m_activeAnimationIndex]->isActive() == false)
		{
			typename Stack::size_type nextIndex = m_activeAnimationIndex + 1;
			
			if (nextIndex < m_animations.size())
			{
				if (m_animations[nextIndex]->isActive())
				{
					m_animations[nextIndex]->stop();
				}
				
				m_animations[nextIndex]->start(
						m_startTags,
						m_presObj,
						m_startName);
				
				m_activeAnimationIndex = nextIndex;
			}
			else
			{
				m_isActive = false;
			}
		}
	}
}


template<typename Type>
bool Sequence<Type>::hasNameOrTagMatch() const
{
	if (m_animations.empty()) return false;
	
	return m_animations.front()->hasNameOrTagMatch();
}


template<typename Type>
const Type& Sequence<Type>::getActive()
{
	TT_ASSERT(isEmpty() == false && 
	          isActiveAnimationIndexValid());
	
	return m_animations.at(m_activeAnimationIndex);
}


template<typename Type>
void Sequence<Type>::start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name)
{
	m_activeAnimationIndex = 0;
	m_startTags = p_tags;
	m_startName = p_name;
	m_presObj = p_presObj;
	
	m_animations[m_activeAnimationIndex]->start(p_tags, p_presObj, p_name);
	
	m_isActive = true;
}


template<typename Type>
void Sequence<Type>::stop()
{
	m_animations[m_activeAnimationIndex]->stop();
	m_activeAnimationIndex = 0;
}


template<typename Type>
void Sequence<Type>::pause()
{
	m_animations[m_activeAnimationIndex]->pause();
}


template<typename Type>
void Sequence<Type>::resume()
{
	m_animations[m_activeAnimationIndex]->resume();
}


template<typename Type>
void Sequence<Type>::reset()
{
	m_animations[m_activeAnimationIndex]->reset();
	m_activeAnimationIndex = 0;
}


template<typename Type>
Sequence<Type>::Sequence(const Sequence& p_rhs)
:
m_animations(),
m_isActive(p_rhs.m_isActive),
m_isLooping(p_rhs.m_isLooping),
m_activeAnimationIndex(0),

// Not copied, These are set when starting the animation and used during the animation.
m_startTags(),
m_startName(),
m_presObj()
{
	for(typename Stack::const_iterator it(p_rhs.m_animations.begin()); it != p_rhs.m_animations.end(); ++it)
	{
		m_animations.push_back(Type((*it)->clone()));
	}
}




#endif // !defined(INC_TT_PRES_ANIM2D_SEQUENCE_INL)
