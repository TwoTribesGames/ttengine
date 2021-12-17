#include <tt/platform/tt_error.h>
#include <tt/str/str.h>

#include <toki/game/editor/ui/helpers.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/types.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

bool selectItemByName(Gwen::Controls::ListBox* p_listBox, const std::string& p_itemName)
{
	TT_NULL_ASSERT(p_listBox);
	if (p_listBox == 0)
	{
		return false;
	}
	
	Gwen::Controls::Layout::Table* listTable = p_listBox->GetTable();
	for (unsigned int rowIdx = 0; rowIdx < listTable->RowCount(0); ++rowIdx)
	{
		Gwen::Controls::Layout::TableRow* row = listTable->GetRow(static_cast<int>(rowIdx));
		if (p_itemName == row->GetName())
		{
			p_listBox->SetSelectedRow(row);
			return true;
		}
	}
	
	return false;
}


bool selectItemByName(tt::gwen::ButtonList* p_buttonList, const std::string& p_itemName)
{
	TT_NULL_ASSERT(p_buttonList);
	if (p_buttonList == 0)
	{
		return false;
	}
	
	return p_buttonList->SelectByName(p_itemName);
}


Gwen::Controls::Button* addCollisionTypesToPicker(tt::gwen::ButtonList* p_list,
                                                  const std::string&    p_nameToSelect)
{
	typedef std::pair<level::CollisionType, std::string> Tile;
	typedef std::vector<Tile> Tiles;
	
	const bool dm = AppGlobal::isInDeveloperMode();
	
	Tiles tiles;
	tiles.push_back(        Tile(level::CollisionType_Solid,                    "COLLISIONTYPE_SOLID"));
	tiles.push_back(        Tile(level::CollisionType_Air,                      "COLLISIONTYPE_AIR"));
	tiles.push_back(        Tile(level::CollisionType_Water_Source,             "COLLISIONTYPE_WATER_SOURCE"));
	tiles.push_back(        Tile(level::CollisionType_Water_Still,              "COLLISIONTYPE_WATER_STILL"));
	tiles.push_back(        Tile(level::CollisionType_Lava_Source,              "COLLISIONTYPE_LAVA_SOURCE"));
	tiles.push_back(        Tile(level::CollisionType_Lava_Still,               "COLLISIONTYPE_LAVA_STILL"));
	tiles.push_back(        Tile(level::CollisionType_AirPrefer,                "COLLISIONTYPE_AIR_PREFER"));
	tiles.push_back(        Tile(level::CollisionType_AirAvoid,                 "COLLISIONTYPE_AIR_AVOID"));
	tiles.push_back(        Tile(level::CollisionType_Crystal_Solid,            "COLLISIONTYPE_CRYSTAL_SOLID"));
	if (dm) tiles.push_back(Tile(level::CollisionType_FluidKill,                "COLLISIONTYPE_FLUIDKILL"));
	if (dm) tiles.push_back(Tile(level::CollisionType_Solid_FluidKill,          "COLLISIONTYPE_SOLID_FLUIDKILL"));
	
	Gwen::Controls::Button* itemToSelect = 0;
	
	s32 tileIndex = 0;
	for (Tiles::iterator it = tiles.begin(); it != tiles.end(); ++it, ++tileIndex)
	{
		const std::string typeName(level::getCollisionTypeName((*it).first));
		Gwen::Controls::Button* item = p_list->AddItem("", "editor.tiles.tile_" + typeName, typeName);
		
		std::wstring prefix;
		if (tileIndex < 9)
		{
			prefix = tt::str::widen(tt::str::toStr(tileIndex + 1) + ": ");
		}
		else if (tileIndex == 9)
		{
			prefix = L"0: ";
		}
		
		item->SetToolTip(prefix + translateString((*it).second));
		
		if (itemToSelect == 0 || typeName == p_nameToSelect)
		{
			itemToSelect = item;
		}
	}
	
	return itemToSelect;
}


void removeItemKeepingSelection(Gwen::Controls::ListBox*          p_listBox,
                                Gwen::Controls::Layout::TableRow* p_item)
{
	TT_NULL_ASSERT(p_listBox);
	TT_NULL_ASSERT(p_item);
	if (p_listBox == 0 || p_item == 0)
	{
		return;
	}
	
	// If the item to remove is currently selected, try to select a different item
	const bool itemWasSelected = (p_listBox->GetSelectedRow() == p_item);
	if (itemWasSelected)
	{
		// Try to select the next item
		Gwen::Controls::Base* listAsBase = p_listBox;
		listAsBase->OnKeyDown(true);
		if (p_listBox->GetSelectedRow() == p_item)
		{
			// Apparently there is no next item: try the previous item
			listAsBase->OnKeyUp(true);
		}
	}
	
	// Remove the item from the list
	p_listBox->RemoveItem(p_item);
	
	// Special handling in case there is now no longer a selected item:
	// notify event handlers that the selection changed (to "none")
	if (itemWasSelected && p_listBox->GetSelectedRow() == 0)
	{
		p_listBox->onRowSelected.Call(p_listBox);
	}
}

// Namespace end
}
}
}
}
