#if !defined(INC_TT_MENU_MENUSOUND_H)
#define INC_TT_MENU_MENUSOUND_H


namespace tt {
namespace menu {

/*! \brief The sounds that can be triggered by menu elements. */
enum MenuSound
{
	MenuSound_None,
	
	MenuSound_ButtonClicked,
	MenuSound_MenuTransition,
	MenuSound_PickerChange,
	
	// FIXME: Worms-specific sound effects here. These do not belong in dslib!
	// - Front-end
	MenuSound_FE_Accept,
	MenuSound_FE_HighlightShift,
	MenuSound_FE_Reject,
	MenuSound_FE_SaveComplete,
	MenuSound_FE_SaveFail,
	MenuSound_FE_Select,
	
	// - Shop
	MenuSound_Shop_CatSelect,
	MenuSound_Shop_InsFunds,
	MenuSound_Shop_PurchaseItem,
	
	// - Flag editor
	MenuSound_FlagEditor_Brush,
	MenuSound_FlagEditor_Fill,
	MenuSound_FlagEditor_Template,
	MenuSound_FlagEditor_Trash,
	
	// - Network
	MenuSound_Net_Connecting,
	MenuSound_Net_Connected,
	MenuSound_Net_Disconnect
};

// Namespace end
}
}


#endif  // !defined(INC_TT_MENU_MENUSOUND_H)
