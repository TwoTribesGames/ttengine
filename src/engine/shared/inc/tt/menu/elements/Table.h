#if !defined(INC_TT_MENU_ELEMENTS_TABLE_H)
#define INC_TT_MENU_ELEMENTS_TABLE_H


#include <tt/menu/elements/ContainerBase.h>
#include <tt/menu/elements/ValueDecorator.h>


namespace tt {
namespace menu {
namespace elements {

// Forward declarations
class Marker;


/*! \brief Simple menu element that can contain other menu elements. */
class Table : public ContainerBase<ValueDecorator>
{
public:
	typedef value_element_tag element_category;
	
	
	Table(const std::string& p_name,
	      const MenuLayout&  p_layout,
	      s32                p_columns,
	      s32                p_rows,
	      s32                p_border,
	      bool               p_hasMarker);
	virtual ~Table();
	
	virtual void render(const math::PointRect& p_rect, s32 p_z);
	
	virtual void doLayout(const math::PointRect& p_rect);
	
	virtual s32 getMinimumWidth()    const;
	virtual s32 getMinimumHeight()   const;
	virtual s32 getRequestedWidth()  const;
	virtual s32 getRequestedHeight() const;
	
	virtual bool onKeyPressed(const MenuKeyboard& p_keys);
	
	virtual bool doAction(const MenuElementAction& p_action);
	
	virtual void setInitialChildByValue(const std::string& p_value);
	
	/*! \brief Selects a direct child by the element's value.
	           Asserts if no direct child with the specified value was found. */
	virtual void selectChildByValue(const std::string& p_value);
	
	virtual void setSelected(bool p_selected);
	
	virtual Table* clone() const;
	
protected:
	Table(const Table& p_rhs);
	
	virtual bool setInitialSelection();
	
private:
	Marker*     m_marker;
	s32         m_columns;
	s32         m_rows;
	s32         m_childCount;
	std::string m_initialChildValue;
	
	s32  m_border;
	bool m_hasMarker;
	
	
	s32 getColumnMinimumWidth(s32 p_column)   const;
	s32 getColumnRequestedWidth(s32 p_column) const;
	s32 getRowMinimumHeight(s32 p_row)        const;
	s32 getRowRequestedHeight(s32 p_row)      const;
	
	// No assignment
	Table& operator=(const Table&);
};

// Namespace end
}
}
}

#endif  // !defined(INC_TT_MENU_ELEMENTS_TABLE_H)
