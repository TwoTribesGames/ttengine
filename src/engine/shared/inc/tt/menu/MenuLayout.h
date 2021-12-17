#if !defined(INC_TT_MENU_MENULAYOUT_H)
#define INC_TT_MENU_MENULAYOUT_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace menu {

/*! \brief Menu element layout parameters. */
class MenuLayout
{
public:
	enum OrderType
	{
		Order_Horizontal,
		Order_Vertical,
		Order_Undefined
	};
	
	enum SizeType
	{
		Size_Auto,
		Size_Max,
		Size_Absolute,
		Size_Undefined
	};
	
	enum PositionType
	{
		Position_Min,
		Position_Center,
		Position_Max,
		Position_Absolute_From_Min,
		Position_Absolute_From_Max,
		
		Position_Undefined,
		
		Position_Left   = Position_Min,
		Position_Right  = Position_Max,
		Position_Top    = Position_Min,
		Position_Bottom = Position_Max
	};
	
	
	MenuLayout();
	~MenuLayout();
	
	inline OrderType    getOrder()                  const { return m_order;                  }
	inline SizeType     getWidthType()              const { return m_widthType;              }
	inline SizeType     getHeightType()             const { return m_heightType;             }
	inline s32          getWidth()                  const { return m_width;                  }
	inline s32          getHeight()                 const { return m_height;                 }
	inline PositionType getHorizontalPositionType() const { return m_horizontalPositionType; }
	inline PositionType getVerticalPositionType()   const { return m_verticalPositionType;   }
	inline s32          getLeft()                   const { return m_left;                   }
	inline s32          getTop()                    const { return m_top;                    }
	
	inline void setOrder(OrderType p_order)                                      { m_order                  = p_order;                  }
	inline void setWidthType(SizeType p_widthType)                               { m_widthType              = p_widthType;              }
	inline void setHeightType(SizeType p_heightType)                             { m_heightType             = p_heightType;             }
	inline void setWidth(s32 p_width)                                            { m_width                  = p_width;                  }
	inline void setHeight(s32 p_height)                                          { m_height                 = p_height;                 }
	inline void setHorizontalPositionType(PositionType p_horizontalPositionType) { m_horizontalPositionType = p_horizontalPositionType; }
	inline void setVerticalPositionType(PositionType p_verticalPositionType)     { m_verticalPositionType   = p_verticalPositionType;   }
	inline void setLeft(s32 p_left)                                              { m_left                   = p_left;                   }
	inline void setTop(s32 p_top)                                                { m_top                    = p_top;                    }
	
	bool operator==(const MenuLayout& p_rhs) const;
	inline bool operator!=(const MenuLayout& p_rhs) const
	{ return operator==(p_rhs) == false; }
	
	/*! \brief Set any undefined variable to the default values. */
	void setUndefinedToDefault();
	
private:
	OrderType    m_order;
	SizeType     m_widthType;
	SizeType     m_heightType;
	PositionType m_horizontalPositionType;
	PositionType m_verticalPositionType;
	
	s32          m_width;
	s32          m_height;
	s32          m_left;
	s32          m_top;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENULAYOUT_H)
