#if !defined(INC_TT_MATH_RECT_H)
#define INC_TT_MATH_RECT_H

#include <tt/math/math.h>
#include <tt/math/Point2.h>
#include <tt/math/Vector2.h>

namespace tt {
namespace math {

/*! \brief Rectangle class.
           Expects X axis Left -> Right
           Expects Y axis Top  -> Down
    \note Uses "screen space" (0, 0) is upper left,
          with positive X going right and positive Y going down. */
template <class Type, s32 inclusiveCorrection>
class Rect
{
public:
	typedef Type PositionType;
	typedef typename PositionType::ValueType ValueType;
	
	/*! \param p_pos The top left corner of the Rect.
	    \param p_width The width of the Rect.
	    \param p_height The height of the Rect. */
	Rect(const PositionType& p_pos, ValueType p_width, ValueType p_height);
	/*! \param p_min top    left corner.
	    \param p_max bottom right corner. */
	Rect(const PositionType& p_min, const PositionType& p_max);
	
	/*! \brief Defaults to a rectangle at position (0, 0) with dimensions (1, 1). */
	Rect();
	
	inline void setValues(const PositionType& p_pos, ValueType p_width, ValueType p_height)
	{
		m_pos = p_pos; m_width = p_width; m_height = p_height;
		TT_ASSERT(m_width  >= 0); TT_ASSERT(m_height >= 0);
	}
	
	inline ValueType getTop()    const {return m_pos.y;}
	inline ValueType getBottom() const {return m_pos.y + getHeight() - inclusiveCorrection;}
	inline ValueType getLeft()   const {return m_pos.x;}
	inline ValueType getRight()  const {return m_pos.x + getWidth() - inclusiveCorrection;}
	
	inline const PositionType& getPosition() const {return m_pos;            }
	PositionType getCenterPosition() const;
	inline ValueType getWidth()        const {return m_width;          }
	inline ValueType getHalfWidth()    const {return getHalf(m_width); }
	inline ValueType getHeight()       const {return m_height;         }
	inline ValueType getHalfHeight()   const {return getHalf(m_height);}
	
	/*! \return top left corner. */
	inline const PositionType& getMin() const {return getPosition();}
	/*! \return bottom right corner. */
	inline PositionType getMaxInside() const { return PositionType(getRight(), getBottom());                    }
	inline PositionType getMaxEdge()   const { return PositionType(m_pos.x + getWidth(), m_pos.y + getHeight());}
	
	inline Rect& translate(const PositionType& p_vector)    { m_pos += p_vector; return *this; }
	inline Rect getTranslated(PositionType p_vector) const { return Rect(m_pos + p_vector, m_width, m_height); }
	
	inline Rect& scale(ValueType p_scale) { m_width *= p_scale; m_height *= p_scale; return *this; }
	inline Rect getScaled(ValueType p_scale) const { return Rect(m_pos, m_width * p_scale, m_height * p_scale); }
	
	/*! \brief Set the position of Rect.
	    \note This does NOT change width or height of the Rect. */
	inline void setPosition(const PositionType& p_pos) {m_pos = p_pos;}
	
	/*! \brief Set the center position of Rect.
	    \note This does NOT change width or height of the Rect. */
	void setCenterPosition(const PositionType& p_centerPos);
	
	/*! \brief Set the width of Rect.
	    \note This will NOT change the position, 
	          but can change the left and right. */
	inline void setWidth(ValueType p_width) {m_width = p_width;}
	/*! \brief Set the height of Rect.
	    \note This will NOT change the position, 
	          but can change the top and bottom. */
	inline void setHeight(ValueType p_height) {m_height = p_height;}
	/*! \brief Set the left side of Rect.
	    \note This can change the rect position and width. */
	void setLeft(ValueType p_left);
	/*! \brief Set the right side of Rect.
	    \note This can change the rect position and width. */
	void setRight(ValueType p_right);
	/*! \brief Set the left and right side of Rect.
	    \note This can change the rect position and width. */
	void setLeftRight(ValueType p_left, ValueType p_right);
	/*! \brief Move Rect so left side is aligned to p_left.
	    \note This will NOT change width or height. */
	inline void alignLeft(ValueType p_left)   {m_pos.x = p_left;}
	/*! \brief Move Rect so right side is aligned to p_left.
	    \note This will NOT change width or height. */
	inline void alignRight(ValueType p_right) {m_pos.x = p_right - getWidth() - inclusiveCorrection;}
	
	/*! \brief Set the top side of Rect.
	    \note This can change the rect position and height. */
	void setTop(ValueType p_top);
	/*! \brief Set the bottom side of Rect.
	    \note This can change the rect position and height. */
	void setBottom(ValueType p_bottom);
	/*! \brief Set the top and bottom side of Rect.
	    \note This can change the rect position and height. */
	void setTopBottom(ValueType p_top, ValueType p_bottom);
	/*! \brief Move Rect so top side is aligned to p_top.
	    \note This will NOT change width or height. */
	inline void alignTop(ValueType p_top)   {m_pos.y = p_top;}
	/*! \brief Move Rect so bottom side is aligned to p_bottom.
	    \note This will NOT change width or height. */
	inline void alignBottom(ValueType p_bottom) {m_pos.y = p_bottom - getHeight() - inclusiveCorrection;}
	
	bool contains(const PositionType& p_point) const;
	bool intersects(const PositionType& p_point) const;
	bool contains(const PositionType& p_point, ValueType p_radius) const;
	bool intersects(const PositionType& p_point, ValueType p_radius) const;
	bool contains(const Rect& p_rect) const;
	
	/*! \brief Indicates whether this rectangle intersects the rectangle that was passed.
	    \param p_intersectionRect Optional pointer to a Rect that will receive the rectangle of intersection. */
	bool intersects(const Rect& p_rect, Rect* p_intersectionRect = 0) const;
	
	/*! \brief Indicates whether the rectangle has surface area (width and height are not 0). */
	inline bool hasArea() const { return m_width > 0 && m_height > 0; }
	
	bool operator==(const Rect& p_rhs) const;
	bool operator!=(const Rect& p_rhs) const;
	
private:
	PositionType m_pos;
	ValueType    m_width;
	ValueType    m_height;
};


template <class Type, s32 inclusiveCorrection>
inline std::ostream& operator<<(std::ostream& p_s, const Rect<Type, inclusiveCorrection>& p_rect)
{
	return p_s << "[pos: " << p_rect.getPosition() << " width: " << p_rect.getWidth() << " height: " << p_rect.getWidth() << "]";
}


template <class Type, s32 inclusiveCorrection>
inline const Rect<Type, inclusiveCorrection> merge(const Rect<Type, inclusiveCorrection>& p_leftRect,
                                                   const Rect<Type, inclusiveCorrection>& p_rightRect)
{
	const Type& leftMin  = p_leftRect. getMin();
	const Type& rightMin = p_rightRect.getMin();
	const Type  leftMax  = p_leftRect. getMaxInside();
	const Type  rightMax = p_rightRect.getMaxInside();
	return Rect<Type, inclusiveCorrection>(Type(std::min(leftMin.x, rightMin.x),
	                                            std::min(leftMin.y, rightMin.y)),
	                                       Type(std::max(leftMax.x, rightMax.x),
	                                            std::max(leftMax.y, rightMax.y)));
}


#include "Rect.inl"


typedef Rect<tt::math::Point2,  1> PointRect;
typedef Rect<tt::math::Vector2, 0> VectorRect;
#include <tt/math/RectPredicates.h>


// end namespace 
}
}

#endif // !defined(INC_TT_MATH_RECT_H)
