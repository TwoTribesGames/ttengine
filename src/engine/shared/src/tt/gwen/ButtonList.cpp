#include <tt/gwen/ButtonList.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( ButtonList )
,
m_items(),
m_selectedItem(0),
m_spacingX(5),
m_spacingY(5)
{
	SetScroll(false, true);
	SetAutoHideBars(true);
	SetMargin(Gwen::Margin(1, 1, 1, 1));
	m_InnerPanel->SetPadding(Gwen::Padding(4, 4, 2, 2));
}


ButtonList::~ButtonList()
{
}


void ButtonList::Clear()
{
	UnselectAll();
	for (Items::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		(*it)->DelayedDelete();
	}
	m_items.clear();
	BaseClass::Clear();
}


void ButtonList::Layout(Gwen::Skin::Base* p_skin)
{
	ReflowItems();
	BaseClass::Layout(p_skin);
}


void ButtonList::Render(Gwen::Skin::Base* p_skin)
{
	p_skin->DrawListBox(this);
}


void ButtonList::AddItem(Gwen::Controls::Button* p_item)
{
	TT_NULL_ASSERT(p_item);
	if (p_item == 0)
	{
		return;
	}
	
	if (HasItem(p_item))
	{
		TT_PANIC("Item 0x%08X was already added to this list. Cannot add the same item more than once.",
		         p_item);
		return;
	}
	
	p_item->SetParent(this);
	p_item->SetIsToggle(true);
	p_item->SetToggleState(false);
	p_item->onToggleOn .Add(this, &ButtonList::OnItemToggledOn);
	p_item->onToggleOff.Add(this, &ButtonList::OnItemToggledOff);
	
	m_items.push_back(p_item);
}


Gwen::Controls::Button* ButtonList::AddItem(const Gwen::TextObject& p_label,
                                            const Gwen::TextObject& p_imageName,
                                            const Gwen::String&     p_name)
{
	Gwen::Controls::Button* item = new Gwen::Controls::Button(this);
	item->SetName(p_name);
	if (p_label.GetUnicode().empty() == false)
	{
		item->SetText(p_label);
	}
	if (p_imageName.GetUnicode().empty() == false)
	{
		item->SetImage(p_imageName);
	}
	item->SizeToContents();
	
	AddItem(item);
	return item;
}


void ButtonList::RemoveItem(Gwen::Controls::Button* p_item)
{
	TT_NULL_ASSERT(p_item);
	if (p_item == 0)
	{
		return;
	}
	
	if (HasItem(p_item) == false)
	{
		TT_PANIC("Item 0x%08X does not belong to this ButtonList. Cannot remove it.", p_item);
		return;
	}
	
	if (m_selectedItem == p_item)
	{
		SetSelectedRow(0);
	}
	p_item->onToggleOn.RemoveHandler(this);
	p_item->onToggleOff.RemoveHandler(this);
	RemoveChild(p_item);
	m_items.erase(std::find(m_items.begin(), m_items.end(), p_item));
	p_item->DelayedDelete();
}


void ButtonList::SetSelectedRow(Gwen::Controls::Button* p_item)
{
	if (p_item != 0 && HasItem(p_item) == false)
	{
		TT_PANIC("Item 0x%08X does not belong to this ButtonList. Cannot select it.", p_item);
		return;
	}
	
	Gwen::Controls::Button* prevSelectedItem = m_selectedItem;
	
	m_selectedItem = p_item;
	
	if (prevSelectedItem != 0 && prevSelectedItem != m_selectedItem)
	{
		prevSelectedItem->SetToggleState(false);
	}
	
	if (p_item != 0 && prevSelectedItem != m_selectedItem)
	{
		p_item->SetToggleState(true);
	}
	
	onRowSelected.Call(this);
}


bool ButtonList::SelectByName(const Gwen::String& p_name)
{
	for (Items::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if ((*it)->GetName() == p_name)
		{
			SetSelectedRow(*it);
			return true;
		}
	}
	
	return false;
}


void ButtonList::UnselectAll()
{
	SetSelectedRow(0);
}


Gwen::Controls::Button* ButtonList::GetRow(unsigned int p_index)
{
	if (p_index >= RowCount())
	{
		TT_PANIC("Row index %u out of bounds. Must be between 0 and %u.", p_index, RowCount());
		return 0;
	}
	
	return m_items[static_cast<Items::size_type>(p_index)];
}


unsigned int ButtonList::RowCount() const
{
	return static_cast<unsigned int>(m_items.size());
}


void ButtonList::SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows)
{
	m_spacingX = p_spacingBetweenColumns;
	m_spacingY = p_spacingBetweenRows;
	Invalidate();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void ButtonList::ReflowItems()
{
	// FIXME: Should flow behavior be configurable? E.g. only lay out vertically, or current flow behavior...
	const Gwen::Rect&    listRect(m_InnerPanel->GetInnerBounds());
	const Gwen::Padding& padding (m_InnerPanel->GetPadding());
	
	const int listRight = (listRect.x + listRect.w) - padding.right;
	int  x                  = padding.left;
	int  y                  = padding.top;
	int  highestItemThisRow = 0;
	bool firstItem          = true;
	
	for (Items::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if ((*it)->Height() > highestItemThisRow)
		{
			highestItemThisRow = (*it)->Height();
		}
		
		if (firstItem == false && (x + (*it)->Width()) >= listRight)
		{
			// Item doesn't fit on this row anymore: place it on the next row
			x = padding.left;
			y += highestItemThisRow + m_spacingY;
			highestItemThisRow = (*it)->Height();
		}
		
		(*it)->SetPos(x, y);
		x += (*it)->Width() + m_spacingX;
		
		firstItem = false;
	}
}


bool ButtonList::HasItem(Gwen::Controls::Button* p_item) const
{
	return std::find(m_items.begin(), m_items.end(), p_item) != m_items.end();
}


void ButtonList::OnItemToggledOn(Gwen::Controls::Base* p_sender)
{
	if (p_sender == m_selectedItem)
	{
		// Selected item did not change
		return;
	}
	
	SetSelectedRow(gwen_cast<Gwen::Controls::Button>(p_sender));
	
	for (Items::iterator it = m_items.begin(); it != m_items.end(); ++it)
	{
		if (*it != p_sender)
		{
			(*it)->SetToggleState(false);
		}
	}
}


void ButtonList::OnItemToggledOff(Gwen::Controls::Base* p_sender)
{
	if (p_sender == m_selectedItem)
	{
		// Do not allow deselecting the selected item
		Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
		button->SetToggleState(true);
		
		// Clicking the selected item again re-sends a "selected" event
		SetSelectedRow(button);
	}
}

// Namespace end
}
}
