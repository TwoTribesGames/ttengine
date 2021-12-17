#include <tt/gwen/GroupedButtonList.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( GroupedButtonList )
,
m_groups(),
m_selectedItem(0),
m_spacingX(5),
m_spacingY(5)
{
	SetScroll(false, true);
	SetAutoHideBars(true);
	SetMargin (Gwen::Margin (1, 1, 1, 1));
	SetPadding(Gwen::Padding(1, 1, 1, 1));
}


GroupedButtonList::~GroupedButtonList()
{
}


void GroupedButtonList::Clear()
{
	UnselectAll();
	for (Groups::iterator groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
	{
		Group& group((*groupIt).second);
		for (Items::iterator itemIt = group.items.begin(); itemIt != group.items.end(); ++itemIt)
		{
			(*itemIt)->DelayedDelete();
		}
	}
	m_groups.clear();
	BaseClass::Clear();
}


void GroupedButtonList::Layout(Gwen::Skin::Base* p_skin)
{
	ReflowItems();
	BaseClass::Layout(p_skin);
}


void GroupedButtonList::Render(Gwen::Skin::Base* p_skin)
{
	p_skin->DrawListBox(this);
}


void GroupedButtonList::AddItem(const Gwen::TextObject& p_group,
                                Gwen::Controls::Button* p_item)
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
	
	Group& group(GetOrCreateGroup(p_group));
	
	p_item->SetParent(group.panel);
	p_item->SetIsToggle(true);
	p_item->SetToggleState(false);
	p_item->onToggleOn .Add(this, &GroupedButtonList::OnItemToggledOn);
	p_item->onToggleOff.Add(this, &GroupedButtonList::OnItemToggledOff);
	
	group.items.push_back(p_item);
}


Gwen::Controls::Button* GroupedButtonList::AddItem(const Gwen::TextObject& p_group,
                                                   const Gwen::TextObject& p_label,
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
	
	AddItem(p_group, item);
	return item;
}


void GroupedButtonList::RemoveItem(Gwen::Controls::Button* p_item)
{
	TT_NULL_ASSERT(p_item);
	if (p_item == 0)
	{
		return;
	}
	
	if (HasItem(p_item) == false)
	{
		TT_PANIC("Item 0x%08X does not belong to this GroupedButtonList. Cannot remove it.", p_item);
		return;
	}
	
	if (m_selectedItem == p_item)
	{
		SetSelectedRow(0);
	}
	p_item->onToggleOn .RemoveHandler(this);
	p_item->onToggleOff.RemoveHandler(this);
	RemoveChild(p_item);
	Group* group = FindGroupForItem(p_item);
	if (group != 0)
	{
		group->items.erase(std::find(group->items.begin(), group->items.end(), p_item));
		// FIXME: Should empty groups be destroyed immediately?
	}
	p_item->DelayedDelete();
}


void GroupedButtonList::SetSelectedRow(Gwen::Controls::Button* p_item)
{
	if (p_item != 0 && HasItem(p_item) == false)
	{
		TT_PANIC("Item 0x%08X does not belong to this GroupedButtonList. Cannot select it.", p_item);
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
		
		// Also expand the new group, if it wasn't already
		Group* group = FindGroupForItem(p_item);
		if (group != 0 && group->isExpanded == false)
		{
			SetGroupExpanded(group, true);
		}
	}
	
	onRowSelected.Call(this);
}


bool GroupedButtonList::SelectByName(const Gwen::String& p_name)
{
	for (Groups::iterator groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
	{
		Group& group((*groupIt).second);
		for (Items::iterator itemIt = group.items.begin(); itemIt != group.items.end(); ++itemIt)
		{
			if ((*itemIt)->GetName() == p_name)
			{
				SetSelectedRow(*itemIt);
				return true;
			}
		}
	}
	
	return false;
}


void GroupedButtonList::UnselectAll()
{
	SetSelectedRow(0);
}


/*
Gwen::Controls::Button* GroupedButtonList::GetRow(unsigned int p_index)
{
	// FIXME: Make this per group?
	if (p_index >= RowCount())
	{
		TT_PANIC("Row index %u out of bounds. Must be between 0 and %u.", p_index, RowCount());
		return 0;
	}
	
	return m_items[static_cast<Items::size_type>(p_index)];
}


unsigned int GroupedButtonList::RowCount() const
{
	// FIXME: Make this per group?
	return static_cast<unsigned int>(m_items.size());
}
// */


void GroupedButtonList::SetGroupExpanded(const Gwen::TextObject& p_group, bool p_expanded)
{
	Group* group = GetGroup(GroupID(p_group.Get()));
	if (group != 0)
	{
		SetGroupExpanded(group, p_expanded);
	}
}


bool GroupedButtonList::IsGroupExpanded(const Gwen::TextObject& p_group) const
{
	const Group* group = GetGroup(GroupID(p_group.Get()));
	return (group != 0) ? group->isExpanded : false;
}


GroupedButtonList::ExpansionState GroupedButtonList::GetAllGroupExpansionState() const
{
	ExpansionState state;
	for (Groups::const_iterator it = m_groups.begin(); it != m_groups.end(); ++it)
	{
		state[(*it).first.getValue()] = (*it).second.isExpanded;
	}
	return state;
}


void GroupedButtonList::SetAllGroupExpansionState(const ExpansionState& p_status)
{
	// Set the expansion state for all groups that were passed to this function
	for (ExpansionState::const_iterator stateIt = p_status.begin(); stateIt != p_status.end(); ++stateIt)
	{
		Groups::iterator groupIt = m_groups.find(GroupID((*stateIt).first));
		if (groupIt != m_groups.end())
		{
			SetGroupExpanded(&(*groupIt).second, (*stateIt).second);
		}
	}
	
	// If the group containing the selected item is no longer expanded,
	// select the first item in the first expanded group instead
	if (m_selectedItem != 0)
	{
		Group* selectedItemGroup = FindGroupForItem(m_selectedItem);
		TT_NULL_ASSERT(selectedItemGroup);
		if (selectedItemGroup != 0 && selectedItemGroup->isExpanded == false)
		{
			for (Groups::iterator it = m_groups.begin(); it != m_groups.end(); ++it)
			{
				Group& group((*it).second);
				if (group.isExpanded && group.items.empty() == false)
				{
					SetSelectedRow(group.items.front());
					break;
				}
			}
		}
	}
}


void GroupedButtonList::SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows)
{
	m_spacingX = p_spacingBetweenColumns;
	m_spacingY = p_spacingBetweenRows;
	Invalidate();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

GroupedButtonList::Group& GroupedButtonList::GetOrCreateGroup(const Gwen::TextObject& p_groupName)
{
	const GroupID groupID(p_groupName.Get());
	
	Groups::iterator it = m_groups.find(groupID);
	if (it != m_groups.end())
	{
		// Group already exists
		return (*it).second;
	}
	
	// Create a new group
	Group& group(m_groups[groupID]);
	
	group.panel = new Gwen::Controls::Base(this);
	group.panel->SetPadding(Gwen::Padding(5, 5, 2, 5));
	group.panel->Dock(Gwen::Pos::Top);
	
	group.header = new Gwen::Controls::Button(group.panel);
	group.header->SetText(p_groupName);
	group.header->SetAlignment(Gwen::Pos::Left & Gwen::Pos::CenterV);
	group.header->SizeToContents();
	group.header->SetIsToggle(true);
	group.header->SetToggleState(true);
	group.header->UserData.Set("group", groupID);
	group.header->onToggle.Add(this, &GroupedButtonList::OnGroupHeaderClicked);
	
	return group;
}


GroupedButtonList::Group* GroupedButtonList::GetGroup(GroupID p_groupID)
{
	Groups::iterator it = m_groups.find(p_groupID);
	return (it != m_groups.end()) ? &(*it).second : 0;
}


const GroupedButtonList::Group* GroupedButtonList::GetGroup(GroupID p_groupID) const
{
	Groups::const_iterator it = m_groups.find(p_groupID);
	return (it != m_groups.end()) ? &(*it).second : 0;
}


GroupedButtonList::Group* GroupedButtonList::FindGroupForItem(Gwen::Controls::Button* p_item)
{
	for (Groups::iterator it = m_groups.begin(); it != m_groups.end(); ++it)
	{
		if (std::find((*it).second.items.begin(), (*it).second.items.end(), p_item) != (*it).second.items.end())
		{
			return &(*it).second;
		}
	}
	
	// Item not in any group
	return 0;
}


const GroupedButtonList::Group* GroupedButtonList::FindGroupForItem(Gwen::Controls::Button* p_item) const
{
	for (Groups::const_iterator it = m_groups.begin(); it != m_groups.end(); ++it)
	{
		if (std::find((*it).second.items.begin(), (*it).second.items.end(), p_item) != (*it).second.items.end())
		{
			return &(*it).second;
		}
	}
	
	// Item not in any group
	return 0;
}


void GroupedButtonList::SetGroupExpanded(Group* p_group, bool p_expanded, bool p_updateUI)
{
	TT_NULL_ASSERT(p_group);
	if (p_group == 0) return;
	
	p_group->isExpanded = p_expanded;
	
	if (p_updateUI)
	{
		p_group->header->SetToggleState(p_expanded);
	}
}


void GroupedButtonList::ReflowItems()
{
	// FIXME: Should flow behavior be configurable? E.g. only lay out vertically, or current flow behavior...
	const int listWidth = GetInnerBounds().w - GetPadding().left - GetPadding().right;
	
	for (Groups::iterator groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
	{
		Group& group((*groupIt).second);
		
		group.panel->SetWidth(listWidth);
		
		const Gwen::Rect&    panelRect(group.panel->GetInnerBounds());
		const Gwen::Padding& padding  (group.panel->GetPadding());
		
		const int listRight = (panelRect.x + panelRect.w) - padding.right;
		int  x = padding.left;
		int  y = padding.top + group.header->Height() + 5;
		int  highestItemThisRow = 0;
		bool firstItem          = true;
		
		group.header->SetPos  (padding.left, padding.top);
		group.header->SetWidth(group.panel->Width() - padding.left - padding.right);
		
		for (Items::iterator it = group.items.begin(); it != group.items.end(); ++it)
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
		
		if (group.isExpanded)
		{
			group.panel->SetHeight(y + highestItemThisRow + padding.bottom);
		}
		else
		{
			group.panel->SetHeight(padding.top + group.header->Height() + padding.bottom);
		}
	}
}


bool GroupedButtonList::HasItem(Gwen::Controls::Button* p_item) const
{
	return FindGroupForItem(p_item) != 0;
}


void GroupedButtonList::OnItemToggledOn(Gwen::Controls::Base* p_sender)
{
	if (p_sender == m_selectedItem)
	{
		// Selected item did not change
		return;
	}
	
	SetSelectedRow(gwen_cast<Gwen::Controls::Button>(p_sender));
	
	for (Groups::iterator groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
	{
		Group& group((*groupIt).second);
		for (Items::iterator itemIt = group.items.begin(); itemIt != group.items.end(); ++itemIt)
		{
			if (*itemIt != p_sender)
			{
				(*itemIt)->SetToggleState(false);
			}
		}
	}
}


void GroupedButtonList::OnItemToggledOff(Gwen::Controls::Base* p_sender)
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


void GroupedButtonList::OnGroupHeaderClicked(Gwen::Controls::Base* p_sender)
{
	Gwen::Controls::Button* button = gwen_cast<Gwen::Controls::Button>(p_sender);
	if (button == 0) return;
	
	Group* group = GetGroup(button->UserData.Get<GroupID>("group"));
	if (group != 0)
	{
		SetGroupExpanded(group, button->GetToggleState(), false);
	}
}

// Namespace end
}
}
