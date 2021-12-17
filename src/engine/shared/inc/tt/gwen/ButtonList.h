#if !defined(INC_TT_GWEN_BUTTONLIST_H)
#define INC_TT_GWEN_BUTTONLIST_H


#include <vector>

#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/ScrollControl.h>
//#include <Gwen/Gwen.h>


namespace tt {
namespace gwen {

/*! \brief A "list" filled with buttons instead of list items. Can have multiple items per row. */
class ButtonList : public Gwen::Controls::ScrollControl
{
public:
	GWEN_CONTROL(ButtonList, Gwen::Controls::ScrollControl);
	virtual ~ButtonList();
	
	virtual void Clear();
	
	virtual void Layout(Gwen::Skin::Base* p_skin);
	virtual void Render(Gwen::Skin::Base* p_skin);
	
	void AddItem(Gwen::Controls::Button* p_item);
	Gwen::Controls::Button* AddItem(const Gwen::TextObject& p_label,
	                                const Gwen::TextObject& p_imageName = Gwen::TextObject(),
	                                const Gwen::String&     p_name      = Gwen::String());
	void RemoveItem(Gwen::Controls::Button* p_item);
	
	void SetSelectedRow(Gwen::Controls::Button* p_item);
	bool SelectByName(const Gwen::String& p_name);
	void UnselectAll();
	
	Gwen::Controls::Button* GetRow(unsigned int p_index);
	unsigned int RowCount() const;
	
	void SetItemSpacing(int p_spacingBetweenColumns, int p_spacingBetweenRows);
	
	inline Gwen::Controls::Button* GetSelectedRow() { return m_selectedItem; }
	
	
	Gwen::Event::Caller onRowSelected;
	
private:
	typedef std::vector<Gwen::Controls::Button*> Items;
	
	
	void ReflowItems();
	bool HasItem(Gwen::Controls::Button* p_item) const;
	void OnItemToggledOn (Gwen::Controls::Base* p_sender);
	void OnItemToggledOff(Gwen::Controls::Base* p_sender);
	
	// No copying
	ButtonList(const ButtonList&);
	ButtonList& operator=(const ButtonList&);
	
	
	Items                   m_items;
	Gwen::Controls::Button* m_selectedItem;
	int                     m_spacingX;
	int                     m_spacingY;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_BUTTONLIST_H)
