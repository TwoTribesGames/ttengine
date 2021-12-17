#include <tt/gwen/GroupedList.h>
#include <tt/platform/tt_error.h>


namespace tt {
namespace gwen {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR( GroupedList )
,
m_groups(),
m_spacingX(5),
m_spacingY(5)
{
	SetScroll(false, true);
	SetAutoHideBars(true);
	SetMargin (Gwen::Margin (1, 1, 1, 1));
	SetPadding(Gwen::Padding(1, 1, 1, 1));
}


GroupedList::~GroupedList()
{
}


void GroupedList::Clear()
{
	//UnselectAll();
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


void GroupedList::Layout(Gwen::Skin::Base* p_skin)
{
	ReflowItems();
	BaseClass::Layout(p_skin);
}


void GroupedList::Render(Gwen::Skin::Base* /*p_skip*/)
{
//	p_skin->DrawListBox(this);
}


void GroupedList::AddItem(const Gwen::TextObject& p_group,
                          Gwen::Controls::Base*   p_item)
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
	
	group.items.push_back(p_item);
}


void GroupedList::RemoveItem(Gwen::Controls::Base* p_item)
{
	TT_NULL_ASSERT(p_item);
	if (p_item == 0)
	{
		return;
	}
	
	if (HasItem(p_item) == false)
	{
		TT_PANIC("Item 0x%08X does not belong to this GroupedList. Cannot remove it.", p_item);
		return;
	}
	
	RemoveChild(p_item);
	Group* group = FindGroupForItem(p_item);
	if (group != 0)
	{
		group->items.erase(std::find(group->items.begin(), group->items.end(), p_item));
		// FIXME: Should empty groups be destroyed immediately?
	}
	p_item->DelayedDelete();
}


void GroupedList::SetGroupExpanded(const Gwen::TextObject& p_group, bool p_expanded)
{
	Group* group = GetGroup(GroupID(p_group.Get()));
	if (group != 0)
	{
		SetGroupExpanded(group, p_expanded);
	}
}


bool GroupedList::IsGroupExpanded(const Gwen::TextObject& p_group) const
{
	const Group* group = GetGroup(GroupID(p_group.Get()));
	return (group != 0) ? group->isExpanded : false;
}


GroupedList::ExpansionState GroupedList::GetAllGroupExpansionState() const
{
	ExpansionState state;
	for (Groups::const_iterator it = m_groups.begin(); it != m_groups.end(); ++it)
	{
		state[(*it).first.getValue()] = (*it).second.isExpanded;
	}
	return state;
}


void GroupedList::SetAllGroupExpansionState(const ExpansionState& p_status)
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
}


void GroupedList::SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows)
{
	m_spacingX = p_spacingBetweenColumns;
	m_spacingY = p_spacingBetweenRows;
	Invalidate();
}


void GroupedList::onGroupExpanded(const Gwen::TextObject& /*p_groupName*/, bool /*p_exanded*/)
{
}


//--------------------------------------------------------------------------------------------------
// Private member functions

GroupedList::Group& GroupedList::GetOrCreateGroup(const Gwen::TextObject& p_groupName)
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
	group.header->onToggle.Add(this, &GroupedList::OnGroupHeaderClicked);
	
	return group;
}


GroupedList::Group* GroupedList::GetGroup(GroupID p_groupID)
{
	Groups::iterator it = m_groups.find(p_groupID);
	return (it != m_groups.end()) ? &(*it).second : 0;
}


const GroupedList::Group* GroupedList::GetGroup(GroupID p_groupID) const
{
	Groups::const_iterator it = m_groups.find(p_groupID);
	return (it != m_groups.end()) ? &(*it).second : 0;
}


GroupedList::Group* GroupedList::FindGroupForItem(Gwen::Controls::Base* p_item)
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


const GroupedList::Group* GroupedList::FindGroupForItem(Gwen::Controls::Base* p_item) const
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


void GroupedList::SetGroupExpanded(Group* p_group, bool p_expanded, bool p_updateUI)
{
	TT_NULL_ASSERT(p_group);
	if (p_group == 0) return;
	
	p_group->isExpanded = p_expanded;
	
	if (p_updateUI)
	{
		p_group->header->SetToggleState(p_expanded);
	}
	
	//
	for (Items::iterator itemIt = p_group->items.begin(); itemIt != p_group->items.end(); ++itemIt)
	{
		p_expanded ? (*itemIt)->Show() : (*itemIt)->Hide();
	}
	
	onGroupExpanded(p_group->header->GetText(), p_expanded);
}


void GroupedList::ReflowItems()
{
	// FIXME: Should flow behavior be configurable? E.g. only lay out vertically, or current flow behavior...
	const int listWidth = GetInnerBounds().w - GetPadding().left - GetPadding().right;
	
	for (Groups::iterator groupIt = m_groups.begin(); groupIt != m_groups.end(); ++groupIt)
	{
		Group& group((*groupIt).second);
		
		group.panel->SetWidth(listWidth);
		
		//const Gwen::Rect&    panelRect(group.panel->GetInnerBounds());
		const Gwen::Padding& padding  (group.panel->GetPadding());
		
		int  x = padding.left;
		int  y = padding.top + group.header->Height() + 5;
		int  w = group.panel->Width() - padding.left - padding.right;
		
		group.header->SetPos  (0, padding.top);
		group.header->SetWidth(group.panel->Width());
		
		for (Items::iterator it = group.items.begin(); it != group.items.end(); ++it)
		{
			(*it)->SetWidth(w);
			(*it)->SetPos(x, y);
			
			if ((*it)->Hidden() == false)
			{
				y += (*it)->Height() + 5;
			}
		}
		
		if (group.isExpanded)
		{
			group.panel->SetHeight(y + padding.bottom);
		}
		else
		{
			group.panel->SetHeight(padding.top + group.header->Height() + padding.bottom);
		}
	}
}


bool GroupedList::HasItem(Gwen::Controls::Base* p_item) const
{
	return FindGroupForItem(p_item) != 0;
}


void GroupedList::OnGroupHeaderClicked(Gwen::Controls::Base* p_sender)
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
