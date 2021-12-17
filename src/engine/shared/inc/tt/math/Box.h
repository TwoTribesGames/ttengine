#if !defined(INC_TT_MATH_BOX_H)
#define INC_TT_MATH_BOX_H

#include <tt/math/math.h>
#include <tt/math/Point3.h>
#include <tt/math/Vector3.h>

namespace tt {
namespace math {

/*! \brief Box class.
    \note Uses 0,0,0 as world center.
          with positive X going right,
		  with positive Y going up,
		  with positive Z going forward (to "camera").
		  
		  Width  is X axis.
		  Height is Y axis.
		  Depth  is Z axis.
		  
		  The position of the box is the bottom left back corner.*/
template <class Type>
class Box
{
public:
	typedef Type PositionType;
	typedef typename PositionType::ValueType ValueType;
	
	/*! \param p_pos The bottom left back corner of the Box.
	    \param p_width  The width of the Box.
	    \param p_height The height of the Box.
		\param p_depth  The depth of the Box.*/
	Box(const PositionType& p_pos, ValueType p_width, 
	    ValueType p_height, ValueType p_depth);
	/*! \param p_min bottom left  back  corner.
	    \param p_max top    right front corner. */
	Box(const PositionType& p_min, const PositionType& p_max);
	
	/*! \brief Defaults to a box at position (0, 0, 0) with dimensions (1, 1, 1). */
	Box();
	
	inline ValueType getTop()    const {return m_pos.y + getHeight() - 1;}
	inline ValueType getBottom() const {return m_pos.y;                  }
	inline ValueType getLeft()   const {return m_pos.x;                  }
	inline ValueType getRight()  const {return m_pos.x + getWidth() - 1; }
	inline ValueType getFront()  const {return m_pos.z + getDepth() - 1; }
	inline ValueType getBack()   const {return m_pos.z;                  }
	
	inline PositionType getPosition()  const {return m_pos;            }
	PositionType getCenterPosition()   const;
	inline ValueType getWidth()        const {return m_width;          }
	inline ValueType getHalfWidth()    const {return getHalf(m_width); }
	inline ValueType getHeight()       const {return m_height;         }
	inline ValueType getHalfHeight()   const {return getHalf(m_height);}
	inline ValueType getDepth()        const {return m_depth;         }
	inline ValueType getHalfDepth()    const {return getHalf(m_depth);}
	
	/*! \return bottom left back corner. */
	inline PositionType getMin() const {return getPosition();}
	/*! \return top right front corner. */
	inline PositionType getMax() const {return PositionType(getRight(), getTop(), getFront());}
	
	void translate(PositionType p_vector) {m_pos += p_vector;}
	/*! \brief Set the position of Box.
	    \note This does NOT change width, height or depth of the Box. */
	inline void setPosition(PositionType p_pos) {m_pos = p_pos;}
	
	/*! \brief Set the center position of Box.
	    \note This does NOT change width, height or depth of the Box. */
	void setCenterPosition(PositionType p_centerPos);
	
	/*! \brief Set the width of Box.
	    \note This will NOT change the position, 
	          but can change the left and right. */
	inline void setWidth(ValueType p_width) {m_width = p_width;}
	/*! \brief Set the height of Box.
	    \note This will NOT change the position, 
	          but can change the top and bottom. */
	inline void setHeight(ValueType p_height) {m_height = p_height;}
	/*! \brief Set the depth of Box.
	    \note This will NOT change the position, 
	          but can change the front and back. */
	inline void setDepth(ValueType p_depth) {m_depth = p_depth;}
	
	/*! \brief Set the left side of Box.
	    \note This can change the Box position and width. */
	void setLeft(ValueType p_left);
	/*! \brief Set the right side of Box.
	    \note This can change the Box position and width. */
	void setRight(ValueType p_right);
	/*! \brief Set the left and right side of Box.
	    \note This can change the Box position and width. */
	void setLeftRight(ValueType p_left, ValueType p_right);
	/*! \brief Move box so left side is aligned to p_left.
	    \note This will NOT change width, height or depth. */
	inline void alignLeft(ValueType p_left)   {m_pos.x = p_left;}
	/*! \brief Move box so right side is aligned to p_left.
	    \note This will NOT change width, height or depth. */
	inline void alignRight(ValueType p_right) {m_pos.x = p_right - getWidth() - 1;}
	
	/*! \brief Set the top side of Box.
	    \note This can change the Box position and height. */
	void setTop(ValueType p_top);
	/*! \brief Set the bottom side of Box.
	    \note This can change the Box position and height. */
	void setBottom(ValueType p_bottom);
	/*! \brief Set the top and bottom side of Box.
	    \note This can change the Box position and height. */
	void setTopBottom(ValueType p_top, ValueType p_bottom);
	/*! \brief Move box so top side is aligned to p_top.
	    \note This will NOT change width, height or depth. */
	inline void alignTop(ValueType p_top)   {m_pos.y = p_top - getHeight() - 1;}
	/*! \brief Move box so bottom side is aligned to p_bottom.
	    \note This will NOT change width, height or depth. */
	inline void alignBottom(ValueType p_bottom) {m_pos.y = p_bottom;}
	
	/*! \brief Set the back side of Box.
	    \note This can change the Box position and depth. */
	void setBack(ValueType p_back);
	/*! \brief Set the front side of Box.
	    \note This can change the Box position and depth. */
	void setFront(ValueType p_front);
	/*! \brief Set the back and front side of Box.
	    \note This can change the Box position and depth. */
	void setBackFront(ValueType p_back, ValueType p_front);
	/*! \brief Move box so back side is aligned to p_back.
	    \note This will NOT change width, height or depth. */
	void alignBack(ValueType p_back)   {m_pos.z = p_back;}
	/*! \brief Move box so front side is aligned to p_front.
	    \note This will NOT change width, height or depth. */
	inline void alignFront(ValueType p_front) {m_pos.z = p_front - getDepth() - 1;}
	
	bool contains(const PositionType& p_point, ValueType p_radius = 0) const;
	bool intersects(const PositionType& p_point, ValueType p_radius = 0) const;
    bool contains(const Box& p_box) const;
    bool intersects(const Box& p_box) const;
	
	inline bool operator==(const Box& p_rhs) const
	{
		return m_pos    == p_rhs.m_pos    && m_width == p_rhs.m_width &&
		       m_height == p_rhs.m_height && m_depth == p_rhs.m_depth;
	}
	
	inline bool operator!=(const Box& p_rhs) const
	{ return operator==(p_rhs) == false; }
	
private:
	PositionType m_pos;
	ValueType m_width;
	ValueType m_height;
	ValueType m_depth;
};


#include "Box.inl"


typedef Box<tt::math::Point3> PointBox;
typedef Box<tt::math::Vector3> VectorBox;


// end namespace 
}
}

#endif // !defined(INC_TT_MATH_BOX_H)
