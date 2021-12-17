#if !defined(INC_TT_GWEN_GROUPEDLIST_H)
#define INC_TT_GWEN_GROUPEDLIST_H


#include <map>
#include <vector>

#include <Gwen/Controls/ScrollControl.h>
//#include <Gwen/Gwen.h>

#include <tt/math/hash/Hash.h>


namespace tt {
namespace gwen {

class GroupedList : public Gwen::Controls::ScrollControl
{
public:
	GWEN_CONTROL(GroupedList, Gwen::Controls::ScrollControl);
	virtual ~GroupedList();
	
	virtual void Clear();
	
	virtual void Layout(Gwen::Skin::Base* p_skin);
	virtual void Render(Gwen::Skin::Base* p_skin);
	
	void AddItem(const Gwen::TextObject& p_group,
	             Gwen::Controls::Base* p_item);
	void RemoveItem(Gwen::Controls::Base* p_item);
	
	void SetGroupExpanded(const Gwen::TextObject& p_group, bool p_expanded);
	bool IsGroupExpanded (const Gwen::TextObject& p_group) const;
	
	typedef std::map<u32, bool> ExpansionState;  // Group ID -> expanded yes/no
	
	ExpansionState GetAllGroupExpansionState() const;
	void           SetAllGroupExpansionState(const ExpansionState& p_status);
	
	void SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows);
	
	virtual void onGroupExpanded(const Gwen::TextObject& p_groupName, bool p_exanded);
	
	Gwen::Event::Caller onRowSelected;
	
private:
	typedef math::hash::Hash<32> GroupID;
	
	typedef std::vector<Gwen::Controls::Base*> Items;
	struct Group
	{
		inline Group()
		:
		panel(0),
		header(0),
		isExpanded(true),
		items()
		{ }
		
		Gwen::Controls::Base*   panel;
		Gwen::Controls::Button* header;
		bool                    isExpanded;
		Items                   items;
	};
	typedef std::map<GroupID, Group> Groups;
	
	
	Group&       GetOrCreateGroup(const Gwen::TextObject& p_groupName);
	Group*       GetGroup(GroupID p_groupID);
	const Group* GetGroup(GroupID p_groupID) const;
	Group*       FindGroupForItem(Gwen::Controls::Base* p_item);
	const Group* FindGroupForItem(Gwen::Controls::Base* p_item) const;
	
	void SetGroupExpanded(Group* p_group, bool p_expanded, bool p_updateUI = true);
	
	void ReflowItems();
	bool HasItem(Gwen::Controls::Base* p_item) const;
	void OnGroupHeaderClicked(Gwen::Controls::Base* p_sender);
	
	// No copying
	GroupedList(const GroupedList&);
	GroupedList& operator=(const GroupedList&);
	
	
	Groups                  m_groups;
	int                     m_spacingX;
	int                     m_spacingY;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_GROUPEDLIST_H)
