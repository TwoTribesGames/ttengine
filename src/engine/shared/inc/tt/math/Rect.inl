#if !defined(INC_TT_MATH_RECT_INL)
#define INC_TT_MATH_RECT_INL

#if !defined(INC_TT_MATH_RECT_H)
#error This inline file should only be included by its parent headerfile.
#endif

template <class Type, s32 inclusiveCorrection>
Rect<Type, inclusiveCorrection>::Rect(const PositionType& p_pos,
                                      ValueType p_width, ValueType p_height)
:
m_pos(p_pos),
m_width(p_width),
m_height(p_height)
{
	TT_ASSERT(m_width  >= 0);
	TT_ASSERT(m_height >= 0);
}


template <class Type, s32 inclusiveCorrection>
Rect<Type, inclusiveCorrection>::Rect(const PositionType& p_min, const PositionType& p_max)
:
m_pos(p_min),
m_width( p_max.x - p_min.x + inclusiveCorrection),
m_height(p_max.y - p_min.y + inclusiveCorrection)
{
	TT_ASSERT(p_min.x <= p_max.x);
	TT_ASSERT(p_min.y <= p_max.y);
	TT_ASSERT(m_width  >= 0);
	TT_ASSERT(m_height >= 0);
}


template<class Type, s32 inclusiveCorrection>
Rect<Type, inclusiveCorrection>::Rect()
:
m_pos(0, 0),
m_width(1),
m_height(1)
{
}


template <class Type, s32 inclusiveCorrection>
typename Rect<Type, inclusiveCorrection>::PositionType Rect<Type, inclusiveCorrection>::getCenterPosition() const
{
	return PositionType(m_pos.x + getHalfWidth(),
	                    m_pos.y + getHalfHeight());
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setCenterPosition(const PositionType& p_centerPos)
{
	m_pos.setValues(p_centerPos.x - getHalfWidth(),
	                p_centerPos.y - getHalfHeight());
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setLeft(ValueType p_left)
{
	setLeftRight(p_left, getRight());
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setRight(ValueType p_right)
{
	setLeftRight(getLeft(), p_right);
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setLeftRight(ValueType p_left, ValueType p_right)
{
	m_width = p_right - p_left + inclusiveCorrection;
	TT_ASSERT(m_width >= 0);
	m_pos.x = p_left;
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setTop(ValueType p_top)
{
	setTopBottom(p_top, getBottom());
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setBottom(ValueType p_bottom)
{
	setTopBottom(getTop(), p_bottom);
}


template <class Type, s32 inclusiveCorrection>
void Rect<Type, inclusiveCorrection>::setTopBottom(ValueType p_top, ValueType p_bottom)
{
	m_height = p_bottom - p_top + inclusiveCorrection;
	TT_ASSERT(m_height >= 0);
	m_pos.y  = p_top;
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::contains(const PositionType& p_point) const
{
	if (p_point.x >= getLeft()  &&
	    p_point.x <= getRight() &&
	    p_point.y >= getTop()   &&
	    p_point.y <= getBottom())
	{
		return true;
	}
	return false;
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::intersects(const PositionType& p_point) const
{
	return contains(p_point);
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::contains(const PositionType& p_point, ValueType p_radius) const
{
	if (p_point.x - p_radius >= getLeft()  &&
	    p_point.x + p_radius <= getRight() &&
	    p_point.y - p_radius >= getTop()   &&
	    p_point.y + p_radius <= getBottom())
	{
		return true;
	}
	return false;
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::intersects(const PositionType& p_point, ValueType p_radius) const
{
	if (p_point.x + p_radius >= getLeft()  &&
	    p_point.x - p_radius <= getRight() &&
	    p_point.y + p_radius >= getTop()   &&
	    p_point.y - p_radius <= getBottom())
	{
		return true;
	}
	return false;
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::contains(const Rect<Type, inclusiveCorrection>& p_rect) const
{
	if (p_rect.getLeft()   >= getLeft()  &&
	    p_rect.getRight()  <= getRight() &&
	    p_rect.getTop()    >= getTop()   &&
	    p_rect.getBottom() <= getBottom())
	{
		return true;
	}
	return false;
}


template <class Type, s32 inclusiveCorrection>
bool Rect<Type, inclusiveCorrection>::intersects(const Rect<Type, inclusiveCorrection>& p_rect, Rect<Type, inclusiveCorrection>* p_intersectionRect) const
{
	if (p_rect.getRight()  >= getLeft()  &&
	    p_rect.getLeft()   <= getRight() &&
	    p_rect.getBottom() >= getTop()   &&
	    p_rect.getTop()    <= getBottom())
	{
		if (p_intersectionRect != 0)
		{
			const Type topLeft(std::max(p_rect.getLeft(), getLeft()), std::max(p_rect.getTop(), getTop()));
			const Type bottomRight(std::min(p_rect.getRight(), getRight()), std::min(p_rect.getBottom(), getBottom()));
			*p_intersectionRect = Rect<Type, inclusiveCorrection>(topLeft, bottomRight);
		}
		return true;
	}
	return false;
}


template <class Type, s32 inclusiveCorrection>
inline bool Rect<Type, inclusiveCorrection>::operator==(const Rect<Type, inclusiveCorrection>& p_rhs) const
{
	return p_rhs.m_pos == m_pos && p_rhs.m_width == m_width && p_rhs.m_height == m_height;
}


template <class Type, s32 inclusiveCorrection>
inline bool Rect<Type, inclusiveCorrection>::operator!=(const Rect<Type, inclusiveCorrection>& p_rhs) const
{
	return p_rhs.m_pos != m_pos || p_rhs.m_width != m_width || p_rhs.m_height != m_height;
}


#endif // !defined(INC_TT_MATH_RECT_INL)
