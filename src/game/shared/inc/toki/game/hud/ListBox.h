#if !defined(INC_TOKI_GAME_HUD_LISTBOX_H)
#define INC_TOKI_GAME_HUD_LISTBOX_H


#include <list>
#include <string>

#include <tt/math/fwd.h>
#include <tt/engine/glyph/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/input/fwd.h>

#include <toki/game/hud/types.h>


namespace toki {
namespace game {
namespace hud {


struct ListItem
{
	std::wstring label;
	u64          id;

	ListItem(const std::wstring& p_label, u64 p_id = 0) : label(p_label), id(p_id)
	{ }
};


struct ListItemGraphic
{
	tt::engine::renderer::QuadSpritePtr textLabel;
	tt::engine::renderer::QuadSpritePtr background;
};


class ListBox
{
public:
	ListBox(const tt::engine::glyph::GlyphSetPtr& p_glyphSet, real p_pixelsPerWorldUnit = 1.0f);
	~ListBox();
	
	ListItem* addItem(const std::wstring& p_label, u64 p_id = 0);
	void removeItem(ListItem* p_item);
	void removeItemById(u64 p_id);
	void removeAll();
	
	s32 selectPrevious();
	s32 selectNext();
	s32 selectItem(ListItem* p_item);
	
	inline s32 getSelectedRow() const { return m_selectedItemIndex; }
	
	ListItem* getItemByIndex(s32 p_index);
	ListItem* getItemById   (u64 p_id);
	ListItem* getItemByLabel(const std::wstring& p_label);
	ListItem* getSelectedItem();
	
	typedef void (*ItemSelectedCallback)(ListItem* p_item, void* p_userData);
	void setItemSelectedCallback(ItemSelectedCallback p_callback, void* p_userData);
	
	bool handleInput(const tt::math::Vector3& p_currentPos,
	                 const tt::input::Button& p_action,
	                 s32 p_wheelNotches);
	
	void update();
	void render();
	
	void setColorScheme(const ListBoxColorScheme& p_colorScheme);
	inline ListBoxColorScheme& getColorScheme() { return m_colorScheme; }
	
	void setPosition(const tt::math::Vector3& p_position);
	inline const tt::math::Vector3& getPosition() const { return m_position; }
	
	void setSize(const tt::math::Vector2& p_size);
	inline const tt::math::Vector2& getSize() const { return m_size; }
	
private:
	void createGraphics();
	ListItemGraphic createGraphicForItem(const ListItem& p_item,
	                                     const tt::engine::renderer::ColorRGBA& p_color);
	void positionItems();
	void selectItemByIndex(s32 p_index, bool p_doCallbacks = true);
	void setHoverItemByIndex(s32 p_index);
	
	void scroll(real p_numberOfItems);
	
	typedef std::vector<ListItem> ItemList;
	void removeItem(const ItemList::iterator& p_iterator);
	
	void updateScrollBarGrip();
	
	
	tt::math::Vector3    m_position;
	tt::math::Vector2    m_size;
	real                 m_itemHeight;
	tt::math::Vector2    m_itemSpacing;
	real                 m_pixelsPerWorldUnit;
	
	ItemList m_items;
	s32 m_selectedItemIndex;
	s32 m_hoverItemIndex;

	ItemSelectedCallback m_itemSelectedCallback;
	void*                m_callbackUserData;
	
	ListBoxColorScheme m_colorScheme;
	
	tt::engine::renderer::QuadSpritePtr m_background;
	typedef std::vector<ListItemGraphic> ItemGraphics;
	ItemGraphics m_listItemGraphics;
	bool         m_itemsChanged;
	tt::engine::glyph::GlyphSetPtr m_glyphSet;

	// Scrolling
	real m_scrollOffset;
	real m_scrollSpeed;
	real m_autoScrollOffset;
	real m_scrollBarWidth;
	real m_previousMouseY;
	tt::engine::renderer::QuadSpritePtr m_scrollBar;
	tt::engine::renderer::QuadSpritePtr m_scrollBarGrip;
	bool m_scrollBarVisible;
	bool m_scrollBarGripActive;
	bool m_hasFocus;
};


// Namespace end
}
}
}


#endif // INC_TOKI_GAME_HUD_LISTBOX_H
