#include <tt/platform/tt_error.h>

#include <tt/menu/elements/Table.h>
#include <tt/menu/elements/Marker.h>
#include <tt/menu/MenuKeyboard.h>
#include <tt/menu/MenuDebug.h>
#include <tt/menu/MenuElementAction.h>


namespace tt {
namespace menu {
namespace elements {

using math::PointRect;


//------------------------------------------------------------------------------
// Public member functions

Table::Table(const std::string& p_name,
             const MenuLayout&  p_layout,
             s32                p_columns,
             s32                p_rows,
             s32                p_border,
             bool               p_hasMarker)
:
ContainerBase<ValueDecorator>(p_name, p_layout),
m_marker(0),
m_columns(p_columns),
m_rows(p_rows),
m_childCount(0),
m_border(p_border),
m_hasMarker(p_hasMarker)
{
	MENU_CREATION_Printf("Table::Table: Element '%s': New Table: "
	                     "%d rows, %d colums.\n",
	                     getName().c_str(),
	                     m_rows, m_columns);
	TT_ASSERTMSG(m_rows    > 0, "Need at least one row.");
	TT_ASSERTMSG(m_columns > 0, "Need at least one column.");
	TT_ASSERTMSG(m_border >= 0, "Cannot have negative border.");
	
	if (m_hasMarker)
	{
		m_marker = new Marker("", p_layout);
	}
}


Table::~Table()
{
	MENU_CREATION_Printf("Table::~Table: Element '%s': "
	                     "Freeing memory for all children.\n",
	                     getName().c_str());
	delete m_marker;
}


void Table::render(const PointRect& p_rect, s32 p_z)
{
	ContainerBase<ValueDecorator>::render(p_rect, p_z);
	
	if (m_hasMarker && m_marker != 0 && m_focusChild != 0)
	{
		PointRect rect(m_focusChild->getRectangle());
		rect.translate(p_rect.getPosition());
		
		m_marker->render(rect, p_z - 3);
	}
}


void Table::doLayout(const PointRect& p_rect)
{
	// Here we have to divide the available space among the children
	// TODO: Divide more intelligently than equal space sharing
	s32 width  = (p_rect.getWidth()  - (m_border * (m_columns - 1))) / m_columns;
	s32 height = (p_rect.getHeight() - (m_border * (m_rows    - 1))) / m_rows;
	
	PointRect rect(p_rect.getPosition(), width, height);
	
	for (s32 i = 0; i < m_columns; ++i)
	{
		rect.setPosition(math::Point2((i * (width + m_border)) + p_rect.getPosition().x,
		                 rect.getPosition().y));
		
		for (s32 j = 0; j < m_rows; ++j)
		{
			if (static_cast<s32>(m_children.size()) > ((j * m_columns) + i))
			{
				rect.setPosition(math::Point2(rect.getPosition().x,
					(j * (height + m_border)) + p_rect.getPosition().y));
				
				MenuLayoutManager::Elements cell;
				ChildVector::size_type idx =
					static_cast<ChildVector::size_type>((j * m_columns) + i);
				cell.push_back(m_children.at(idx));
				MenuLayoutManager::doLayout(getLayout(), cell, rect);
			}
		}
	}
	
	
	// Select an initial child
	setInitialSelection();
	
	/*
	// Select the first child, child by value or child by name
	TT_ASSERTMSG(m_initialChildName.empty() || m_initialChildValue.empty(),
	                "Table '%s': Cannot specify both an initial child "
	                "by name ('%s') AND by value ('%s').",
	                getName().c_str(),
	                m_initialChildName.c_str(),
	                m_initialChildValue.c_str());
	
	if (m_initialChildValue.empty() == false)
	{
		// Select the child by value
		selectChildByValue(m_initialChildValue);
		if (getSelectedChildIndex() != -1)
		{
			// Selection succeeded
			return;
		}
	}
	
	if (m_initialChildName.empty() == false)
	{
		// Select the child by name
		selectChildByName(m_initialChildName);
		if (getSelectedChildIndex() != -1)
		{
			// Selection succeeded
			return;
		}
	}
	
	// Set the focus to the first focusable child if no focus was set yet
	ChildVector::size_type index = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		(*it)->setSelected(false);
		
		if ((*it)->isDefaultSelected() ||
		    isSelectableElement(*it))
		{
			m_focusChild       = *it;
			m_focusChildIndex = index;
			
			m_focusChild->setSelected(true);
			break;
		}
	}
	//*/
}


s32 Table::getMinimumWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		s32 minWidth = -m_border;
		for (s32 i = 0; i < m_columns; ++i)
		{
			minWidth += m_border;
			minWidth += getColumnMinimumWidth(i);
		}
		
		return minWidth;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("%s has undefined width type!",
		            getName().c_str());
		return 0;
	}
}


s32 Table::getMinimumHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		s32 minHeight = -m_border;
		for (s32 i = 0; i < m_rows; ++i)
		{
			minHeight += m_border;
			minHeight += getRowMinimumHeight(i);
		}
		
		return minHeight;
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("%s has undefined height type!",
		            getName().c_str());
		return 0;
	}
}


s32 Table::getRequestedWidth() const
{
	if (getLayout().getWidthType() == MenuLayout::Size_Auto ||
	    getLayout().getWidthType() == MenuLayout::Size_Max)
	{
		s32 reqWidth = -m_border;
		for (s32 i = 0; i < m_columns; ++i)
		{
			reqWidth += m_border;
			reqWidth += getColumnRequestedWidth(i);
		}
		
		return reqWidth;
	}
	else if (getLayout().getWidthType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getWidth();
	}
	else
	{
		TT_PANIC("%s has undefined width type!",
		            getName().c_str());
		return 0;
	}
}


s32 Table::getRequestedHeight() const
{
	if (getLayout().getHeightType() == MenuLayout::Size_Auto ||
	    getLayout().getHeightType() == MenuLayout::Size_Max)
	{
		s32 reqHeight = -m_border;
		for (s32 i = 0; i < m_rows; ++i)
		{
			reqHeight += m_border;
			reqHeight += getRowRequestedHeight(i);
		}
		
		return reqHeight;
	}
	else if (getLayout().getHeightType() == MenuLayout::Size_Absolute)
	{
		return getLayout().getHeight();
	}
	else
	{
		TT_PANIC("%s has undefined height type!",
		            getName().c_str());
		return 0;
	}
}


bool Table::onKeyPressed(const MenuKeyboard& p_keys)
{
	int childIdx = getSelectedChildIndex();
	int row      = childIdx / m_columns;
	int col      = childIdx % m_columns;
	
	// Check if selection needs to be changed
	if (p_keys.isKeySet(MenuKeyboard::MENU_RIGHT))
	{
		if (col < m_columns - 1)
		{
			// Move to the next column in the row
			selectChildByIndex(childIdx + 1);
			return true;
		}
		else
		{
			if (isUserLoopEnabled())
			{
				// Loop to the first column in the row
				selectChildByIndex(row * m_columns);
				return true;
			}
			else
			{
				// No looping; input not handled
				return false;
			}
		}
	}
	else if (p_keys.isKeySet(MenuKeyboard::MENU_LEFT))
	{
		if (col > 0)
		{
			// Move to the previous column in the row
			selectChildByIndex(childIdx - 1);
			return true;
		}
		else
		{
			if (isUserLoopEnabled())
			{
				// Loop to the last column in the row
				selectChildByIndex((row * m_columns) + (m_columns - 1));
				return true;
			}
			else
			{
				// No looping; input not handled
				return false;
			}
		}
	}
	else if (p_keys.isKeySet(MenuKeyboard::MENU_UP))
	{
		if (row > 0)
		{
			// Move up one row
			selectChildByIndex(((row - 1) * m_columns) + col);
			return true;
		}
		else
		{
			if (isUserLoopEnabled())
			{
				// Loop to the last row
				selectChildByIndex(((m_rows - 1) * m_columns) + col);
				return true;
			}
			else
			{
				// No looping; input not handled
				return false;
			}
		}
	}
	else if (p_keys.isKeySet(MenuKeyboard::MENU_DOWN))
	{
		if (row < (m_rows - 1))
		{
			// Move down one row
			selectChildByIndex(((row + 1) * m_columns) + col);
			return true;
		}
		else
		{
			if (isUserLoopEnabled())
			{
				// Loop to the first row
				selectChildByIndex(col);
				return true;
			}
			else
			{
				// No looping; input not handled
				return false;
			}
		}
	}
	
	// Fall back to default container behavior
	return ContainerBase<ValueDecorator>::onKeyPressed(p_keys);
}


bool Table::doAction(const MenuElementAction& p_action)
{
	if (ContainerBase<ValueDecorator>::doAction(p_action))
	{
		return true;
	}
	
	if (p_action.getCommand() == "select_by_value")
	{
		TT_ASSERTMSG(p_action.getParameterCount() == 1,
		                "Command '%s' takes one parameter: "
		                "the value of the child to select.",
		                p_action.getCommand().c_str());
		
		selectChildByValue(p_action.getParameter(0));
		
		return true;
	}
	
	return false;
}


void Table::setInitialChildByValue(const std::string& p_value)
{
	m_initialChildValue = p_value;
}


void Table::selectChildByValue(const std::string& p_value)
{
	// Select the child with the specified value
	ChildVector::size_type index = 0;
	for (ChildVector::iterator it = m_children.begin();
	     it != m_children.end(); ++it, ++index)
	{
		if ((*it)->getValue() == p_value)
		{
			if (isSelectableElement(*it))
			{
				selectChildByIndex(static_cast<int>(index));
			}
			/*
			else
			{
				// Cannot select child; select the first selectable one instead
				int first_child = getFirstSelectableChildIndex();
				if (first_child != -1)
				{
					selectChildByIndex(first_child);
				}
			}
			//*/
			return;
		}
	}
	
	// Child not found
	TT_PANIC("Table '%s' does not have a child with value '%s'.",
	            getName().c_str(), p_value.c_str());
}


void Table::setSelected(bool p_selected)
{
	MenuElement::setSelected(p_selected);
	
	// Ignore completely
}


Table* Table::clone() const
{
	return new Table(*this);
}


//------------------------------------------------------------------------------
// Protected member functions

Table::Table(const Table& p_rhs)
:
ContainerBase<ValueDecorator>(p_rhs),
m_marker(0),
m_columns(p_rhs.m_columns),
m_rows(p_rhs.m_rows),
m_childCount(p_rhs.m_childCount),
m_initialChildValue(p_rhs.m_initialChildValue),
m_border(p_rhs.m_border),
m_hasMarker(p_rhs.m_hasMarker)
{
	if (p_rhs.m_marker != 0)
	{
		m_marker = p_rhs.m_marker->clone();
	}
}


bool Table::setInitialSelection()
{
	// Select the first child, child by value or child by name
	TT_ASSERTMSG(m_initialChildName.empty() || m_initialChildValue.empty(),
	             "Table '%s': Cannot specify both an initial child "
	             "by name ('%s') AND by value ('%s').",
	             getName().c_str(),
	             m_initialChildName.c_str(),
	             m_initialChildValue.c_str());
	
	if (m_initialChildValue.empty() == false)
	{
		// Select the child by value
		selectChildByValue(m_initialChildValue);
		if (getSelectedChildIndex() != -1)
		{
			// Selection succeeded
			return true;
		}
	}
	
	// Use default behavior
	return ContainerBase<ValueDecorator>::setInitialSelection();
}


//------------------------------------------------------------------------------
// Private member functions

s32 Table::getColumnMinimumWidth(s32 p_column) const
{
	s32 minWidth = 0;
	for (s32 i = 0; i < m_rows; ++i)
	{
		ChildVector::size_type idx =
			static_cast<ChildVector::size_type>((i * m_columns) + p_column);
		if (m_children.size() > idx)
		{
			minWidth = std::max(minWidth,
			                    m_children.at(idx)->getMinimumWidth());
		}
	}
	
	return minWidth;
}


s32 Table::getColumnRequestedWidth(s32 p_column) const
{
	s32 reqWidth = 0;
	for (s32 i = 0; i < m_rows; ++i)
	{
		ChildVector::size_type idx =
			static_cast<ChildVector::size_type>((i * m_columns) + p_column);
		if (m_children.size() > idx)
		{
			reqWidth = std::max(reqWidth,
			                    m_children.at(idx)->getRequestedWidth());
		}
	}
	
	return reqWidth;
}


s32 Table::getRowMinimumHeight(s32 p_row) const
{
	s32 minHeight = 0;
	for (s32 i = 0; i < m_columns; ++i)
	{
		ChildVector::size_type idx =
			static_cast<ChildVector::size_type>((p_row * m_columns) + i);
		if (m_children.size() > idx)
		{
			minHeight = std::max(minHeight,
			                     m_children.at(idx)->getMinimumHeight());
		}
	}
	
	return minHeight;
}


s32 Table::getRowRequestedHeight(s32 p_row) const
{
	s32 reqHeight = 0;
	for (s32 i = 0; i < m_columns; ++i)
	{
		ChildVector::size_type idx =
			static_cast<ChildVector::size_type>((p_row * m_columns) + i);
		if (m_children.size() > idx)
		{
			reqHeight = std::max(reqHeight,
			                     m_children.at(idx)->getRequestedHeight());
		}
	}
	
	return reqHeight;
}

// Namespace end
}
}
}
