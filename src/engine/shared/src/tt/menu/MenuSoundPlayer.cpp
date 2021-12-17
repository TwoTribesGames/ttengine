#include <tt/platform/tt_error.h>
//#include <tt/memory/HeapMgr.h>
#include <tt/menu/MenuSoundPlayer.h>


namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Public member functions

MenuSoundPlayer::MenuSoundPlayer()
{
}


MenuSoundPlayer::~MenuSoundPlayer()
{
}


/*
void* MenuSoundPlayer::operator new(std::size_t p_blockSize)
{
	using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
	u32 foo = 0;
	asm	{    mov     foo, lr}
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
}


void MenuSoundPlayer::operator delete(void* p_block)
{
	memory::HeapMgr::freeToHeap(p_block);
}
//*/


MenuSound MenuSoundPlayer::getSoundEnum(const std::string& p_name)
{
	if (p_name == "none")
	{
		return MenuSound_None;
	}
	else if (p_name == "button_clicked")
	{
		return MenuSound_ButtonClicked;
	}
	else if (p_name == "menu_transition")
	{
		return MenuSound_MenuTransition;
	}
	else if (p_name == "picker_change")
	{
		return MenuSound_PickerChange;
	}
	
	// FIXME: Worms-specific sound effects here. These do not belong in dslib!
	// - Front-end
	else if (p_name == "fe_accept")
	{
		return MenuSound_FE_Accept;
	}
	else if (p_name == "fe_highlight_shift")
	{
		return MenuSound_FE_HighlightShift;
	}
	else if (p_name == "fe_reject")
	{
		return MenuSound_FE_Reject;
	}
	else if (p_name == "fe_save_complete")
	{
		return MenuSound_FE_SaveComplete;
	}
	else if (p_name == "fe_save_fail")
	{
		return MenuSound_FE_SaveFail;
	}
	else if (p_name == "fe_select")
	{
		return MenuSound_FE_Select;
	}
	
	// - Shop
	else if (p_name == "shop_category_select")
	{
		return MenuSound_Shop_CatSelect;
	}
	else if (p_name == "shop_insufficient_funds")
	{
		return MenuSound_Shop_InsFunds;
	}
	else if (p_name == "shop_purchase_item")
	{
		return MenuSound_Shop_PurchaseItem;
	}
	
	// - Flag editor
	else if (p_name == "flageditor_brush")
	{
		return MenuSound_FlagEditor_Brush;
	}
	else if (p_name == "flageditor_fill")
	{
		return MenuSound_FlagEditor_Fill;
	}
	else if (p_name == "flageditor_template")
	{
		return MenuSound_FlagEditor_Template;
	}
	else if (p_name == "flageditor_trash")
	{
		return MenuSound_FlagEditor_Trash;
	}
	
	// - Network
	else if (p_name == "net_connecting")
	{
		return MenuSound_Net_Connecting;
	}
	else if (p_name == "net_connected")
	{
		return MenuSound_Net_Connected;
	}
	else if (p_name == "net_disconnect")
	{
		return MenuSound_Net_Disconnect;
	}
	
	TT_PANIC("Invalid sound name: '%s'", p_name.c_str());
	return MenuSound_None;
}


std::string MenuSoundPlayer::getSoundName(MenuSound p_enum)
{
	switch (p_enum)
	{
	case MenuSound_None:           return "none";            break;
	case MenuSound_ButtonClicked:  return "button_clicked";  break;
	case MenuSound_MenuTransition: return "menu_transition"; break;
	case MenuSound_PickerChange:   return "picker_change";   break;
		
		// FIXME: Worms-specific sound effects here. These do not belong in dslib!
		// - Front-end
	case MenuSound_FE_Accept:         return "fe_accept";          break;
	case MenuSound_FE_HighlightShift: return "fe_highlight_shift"; break;
	case MenuSound_FE_Reject:         return "fe_reject";          break;
	case MenuSound_FE_SaveComplete:   return "fe_save_complete";   break;
	case MenuSound_FE_SaveFail:       return "fe_save_fail";       break;
	case MenuSound_FE_Select:         return "fe_select";          break;
		
		// - Shop
	case MenuSound_Shop_CatSelect:    return "shop_category_select";    break;
	case MenuSound_Shop_InsFunds:     return "shop_insufficient_funds"; break;
	case MenuSound_Shop_PurchaseItem: return "shop_purchase_item";      break;
		
		// - Flag editor
	case MenuSound_FlagEditor_Brush:    return "flageditor_brush";    break;
	case MenuSound_FlagEditor_Fill:     return "flageditor_fill";     break;
	case MenuSound_FlagEditor_Template: return "flageditor_template"; break;
	case MenuSound_FlagEditor_Trash:    return "flageditor_trash";     break;
	
		// - Network
	case MenuSound_Net_Connecting: return "net_connecting"; break;
	case MenuSound_Net_Connected:  return "net_connected";  break;
	case MenuSound_Net_Disconnect: return "net_disconnect"; break;
	}
	
	TT_PANIC("Invalid sound identifier: %d", p_enum);
	return "";
}

// Namespace end
}
}
