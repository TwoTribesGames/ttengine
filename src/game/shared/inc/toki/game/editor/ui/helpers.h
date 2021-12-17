#if !defined(INC_TOKI_GAME_EDITOR_UI_HELPERS_H)
#define INC_TOKI_GAME_EDITOR_UI_HELPERS_H


#include <Gwen/Controls/ListBox.h>

#include <tt/gwen/ButtonList.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

bool selectItemByName(Gwen::Controls::ListBox* p_listBox,    const std::string& p_itemName);
bool selectItemByName(tt::gwen::ButtonList*    p_buttonList, const std::string& p_itemName);

Gwen::Controls::Button* addCollisionTypesToPicker(tt::gwen::ButtonList* p_list,
                                                  const std::string&    p_nameToSelect);

/*! \brief Removes an item from a list box.
           If the item was selected, moves the selection to the next item.
           If there is no next item, moves the selection to the previous item. */
void removeItemKeepingSelection(Gwen::Controls::ListBox*          p_listBox,
                                Gwen::Controls::Layout::TableRow* p_item);


/*! \brief Searches for a list box item by its userdata value. */
template<typename T>
inline Gwen::Controls::Layout::TableRow* findItemByUserData(Gwen::Controls::ListBox* p_listBox,
                                                            const Gwen::String&      p_userDataKey,
                                                            T                        p_userDataValue)
{
	TT_NULL_ASSERT(p_listBox);
	if (p_listBox == 0)
	{
		return 0;
	}
	
	Gwen::Controls::Base::List& children = p_listBox->GetTable()->GetChildren();
	
	for (Gwen::Controls::Base::List::iterator it = children.begin(); it != children.end(); ++it)
	{
		Gwen::Controls::Layout::TableRow* item = gwen_cast<Gwen::Controls::Layout::TableRow>(*it);
		if (item != 0 &&
		    item->UserData.Exists(p_userDataKey) &&
		    item->UserData.Get<T>(p_userDataKey) == p_userDataValue)
		{
			return item;
		}
	}
	
	return 0;
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_HELPERS_H)
