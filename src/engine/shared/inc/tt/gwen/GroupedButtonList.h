#if !defined(INC_TT_GWEN_GROUPEDBUTTONLIST_H)
#define INC_TT_GWEN_GROUPEDBUTTONLIST_H


#include <map>
#include <vector>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/ScrollControl.h>
//#include <Gwen/Gwen.h>

#include <tt/math/hash/Hash.h>


namespace tt {
namespace gwen {

/*! \brief A "list" filled with buttons instead of list items. Can have multiple items per row.
           Items are in named groups. */
class GroupedButtonList : public Gwen::Controls::ScrollControl
{
public:
	GWEN_CONTROL(GroupedButtonList, Gwen::Controls::ScrollControl);
	virtual ~GroupedButtonList();
	
	virtual void Clear();
	
	virtual void Layout(Gwen::Skin::Base* p_skin);
	virtual void Render(Gwen::Skin::Base* p_skin);
	
	void AddItem(const Gwen::TextObject& p_group,
	             Gwen::Controls::Button* p_item);
	Gwen::Controls::Button* AddItem(const Gwen::TextObject& p_group,
	                                const Gwen::TextObject& p_label,
	                                const Gwen::TextObject& p_imageName = Gwen::TextObject(),
	                                const Gwen::String&     p_name      = Gwen::String());
	void RemoveItem(Gwen::Controls::Button* p_item);
	
	void SetSelectedRow(Gwen::Controls::Button* p_item);
	bool SelectByName(const Gwen::String& p_name);
	void UnselectAll();
	
	/*
	Gwen::Controls::Button* GetRow(unsigned int p_index);
	unsigned int RowCount() const;
	*/
	
	void SetGroupExpanded(const Gwen::TextObject& p_group, bool p_expanded);
	bool IsGroupExpanded (const Gwen::TextObject& p_group) const;
	
	typedef std::map<u32, bool> ExpansionState;  // Group ID -> expanded yes/no
	
	ExpansionState GetAllGroupExpansionState() const;
	void           SetAllGroupExpansionState(const ExpansionState& p_status);
	
	void SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows);
	
	inline Gwen::Controls::Button* GetSelectedRow() { return m_selectedItem; }
	
	
	Gwen::Event::Caller onRowSelected;
	
private:
	typedef math::hash::Hash<32> GroupID;
	
	typedef std::vector<Gwen::Controls::Button*> Items;
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
	Group*       FindGroupForItem(Gwen::Controls::Button* p_item);
	const Group* FindGroupForItem(Gwen::Controls::Button* p_item) const;
	
	void SetGroupExpanded(Group* p_group, bool p_expanded, bool p_updateUI = true);
	
	void ReflowItems();
	bool HasItem(Gwen::Controls::Button* p_item) const;
	void OnItemToggledOn     (Gwen::Controls::Base* p_sender);
	void OnItemToggledOff    (Gwen::Controls::Base* p_sender);
	void OnGroupHeaderClicked(Gwen::Controls::Base* p_sender);
	
	// No copying
	GroupedButtonList(const GroupedButtonList&);
	GroupedButtonList& operator=(const GroupedButtonList&);
	
	
	Groups                  m_groups;
	Gwen::Controls::Button* m_selectedItem;
	int                     m_spacingX;
	int                     m_spacingY;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_GROUPEDBUTTONLIST_H)
