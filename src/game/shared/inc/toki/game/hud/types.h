#if !defined(INC_TOKI_GAME_HUD_TYPES_H)
#define INC_TOKI_GAME_HUD_TYPES_H


#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/script/VirtualMachine.h>


namespace toki /*! */ {
namespace game /*! */ {
namespace hud  /*! */ {

/*! \brief The color scheme for a list box. */
struct ListBoxColorScheme
{
	tt::engine::renderer::ColorRGBA background;       //!< The background color for the list.
	tt::engine::renderer::ColorRGBA backgroundHover;  //!< The background color for the list when mouse hovering.
	tt::engine::renderer::ColorRGBA itemOdd;          //!< The item background color for oddly numbered items.
	tt::engine::renderer::ColorRGBA itemEven;         //!< The item background color for even numbered items.
	tt::engine::renderer::ColorRGBA itemSelected;     //!< The item background color selected items.
	tt::engine::renderer::ColorRGBA itemHover;        //!< The item background color when mouse hovering.
	tt::engine::renderer::ColorRGBA itemText;         //!< The item text color.
	tt::engine::renderer::ColorRGBA itemTextSelected; //!< The item text color for selected items.
	tt::engine::renderer::ColorRGBA itemTextHover;    //!< The item text color when mouse hovering.
	tt::engine::renderer::ColorRGBA scrollBar;        //!< The scroll bar background color.
	tt::engine::renderer::ColorRGBA scrollBarGrip;    //!< The scroll bar grip color.
	tt::engine::renderer::ColorRGBA scrollBarHover;   //!< The scroll bar background color when mouse hovering.
	
	// Default color scheme
	inline ListBoxColorScheme()
	:
	background      (100,100,100,150),
	backgroundHover (100,100,100,255),
	itemOdd         (150,150,150,200),
	itemEven        (200,200,200,200),
	itemSelected    (100,100,250,200),
	itemHover       (200,200,250,200),
	itemText        (255, 255, 255, 255),
	itemTextSelected(255, 255, 255, 255),
	itemTextHover   (255, 255, 255, 255),
	scrollBar       (150,150,150,150),
	scrollBarGrip   (100,100,250,150),
	scrollBarHover  (100,100,250,250)
	{ }
	
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};

// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_HUD_TYPES_H)
