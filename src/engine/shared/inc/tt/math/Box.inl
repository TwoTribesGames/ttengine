#if !defined(INC_TT_MATH_BOX_INL)
#define INC_TT_MATH_BOX_INL

#if !defined(INC_TT_MATH_BOX_H)
#error This inline file should only be included by its parent headerfile.
#endif

template <class Type>
Box<Type>::Box(const PositionType& p_centerPos,
                ValueType p_width, ValueType p_height, ValueType p_depth)
:
m_pos(p_centerPos),
m_width(p_width),
m_height(p_height),
m_depth(p_depth)
{
	TT_ASSERT(m_width  >= 0);
	TT_ASSERT(m_height >= 0);
	TT_ASSERT(m_depth  >= 0);
}


template <class Type>
Box<Type>::Box(const PositionType& p_min, const PositionType& p_max)
:
m_pos(p_min),
m_width( p_max.x - p_min.x + 1),
m_height(p_max.y - p_min.y + 1),
m_depth( p_max.z - p_min.z + 1)
{
	TT_ASSERT(p_min.x <= p_max.x);
	TT_ASSERT(p_min.y <= p_max.y);
	TT_ASSERT(p_min.z <= p_max.z);
	TT_ASSERT(m_width  >= 0);
	TT_ASSERT(m_height >= 0);
	TT_ASSERT(m_depth  >= 0);
}


template<class Type>
Box<Type>::Box()
:
m_pos(0, 0, 0),
m_width(1),
m_height(1),
m_depth(1)
{
}


template <class Type>
typename Box<Type>::PositionType Box<Type>::getCenterPosition() const
{
	return PositionType(m_pos.x + getHalfWidth(), 
	                    m_pos.y + getHalfHeight(),
						m_pos.z + getHalfDepth());
}


template <class Type>
void Box<Type>::setCenterPosition(PositionType p_centerPos)
{
	m_pos.setValues(p_centerPos.x - getHalfWidth(), 
	                p_centerPos.y - getHalfHeight(),
					p_centerPos.z - getHalfDepth());
}


template <class Type>
void Box<Type>::setLeft(ValueType p_left)
{
	setLeftRight(p_left, getRight());
}


template <class Type>
void Box<Type>::setRight(ValueType p_right)
{
	setLeftRight(getLeft(), p_right);
}


template <class Type>
void Box<Type>::setLeftRight(ValueType p_left, ValueType p_right)
{
	m_width = p_right - p_left + 1;
	TT_ASSERT(m_width >= 0);
	m_pos.x = p_left;
}


template <class Type>
void Box<Type>::setTop(ValueType p_top)
{
	setTopBottom(p_top, getBottom());
}


template <class Type>
void Box<Type>::setBottom(ValueType p_bottom)
{
	setTopBottom(getTop(), p_bottom);
}


template <class Type>
void Box<Type>::setTopBottom(ValueType p_top, ValueType p_bottom)
{
	m_height = p_top - p_bottom + 1;
	TT_ASSERT(m_height >= 0);
	m_pos.y  = p_top;
}


template <class Type>
void Box<Type>::setBack(ValueType p_back)
{
	setBackFront(p_back, getFront());
}


template <class Type>
void Box<Type>::setFront(ValueType p_front)
{
	setBackFront(getBack(), p_front());
}


template <class Type>
void Box<Type>::setBackFront(ValueType p_back, ValueType p_front)
{
	m_depth = p_front - p_back + 1;
	TT_ASSERT(m_depth >= 0);
	m_pos.z = p_back;
}


template <class Type>
bool Box<Type>::contains(const PositionType& p_point, ValueType p_radius) const
{
	if (p_point.x - p_radius >= getLeft()   &&
	    p_point.x + p_radius <= getRight()  &&
	    p_point.y - p_radius <= getTop()    &&
	    p_point.y + p_radius >= getBottom() &&
	    p_point.z - p_radius >= getBack()   &&
	    p_point.z + p_radius <= getFront())
	{
		return true;
	}
	return false;
}


template <class Type>
bool Box<Type>::intersects(const PositionType& p_point, ValueType p_radius) const
{
	if (p_point.x + p_radius >= getLeft()   &&
	    p_point.x - p_radius <= getRight()  &&
	    p_point.y + p_radius <= getTop()    &&
	    p_point.y - p_radius >= getBottom() &&
	    p_point.z + p_radius >= getBack()   &&
	    p_point.z - p_radius <= getFront())
	{
		return true;
	}
	return false;
}


template <class Type>
bool Box<Type>::contains(const Box<Type>& p_box) const
{
	if (p_box.getLeft()   >= getLeft()   &&
	    p_box.getRight()  <= getRight()  &&
	    p_box.getTop()    <= getTop()    &&
	    p_box.getBottom() >= getBottom() &&
	    p_box.getBack()   <= getBack()   &&
	    p_box.getFront()  <= getFront())
	{
		return true;
	}
	return false;
}


template <class Type>
bool Box<Type>::intersects(const Box<Type>& p_box) const

{
	if (p_box.getRight()  >= getLeft()   &&
	    p_box.getLeft()   <= getRight()  &&
	    p_box.getBottom() <= getTop()    &&
	    p_box.getTop()    >= getBottom() &&
	    p_box.getFront()  <= getBack()   &&
	    p_box.getBack()   <= getFront())
	{
		return true;
	}
	return false;
}


#endif // !defined(INC_TT_MATH_BOX_INL)
