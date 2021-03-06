#if !defined(INC_TT_MENU_ELEMENTS_ELEMENTLAYOUT_H)
#define INC_TT_MENU_ELEMENTS_ELEMENTLAYOUT_H


#include <tt/engine/renderer/ColorRGBA.h>


namespace tt {
namespace menu {
namespace elements {

enum ElementTextureCoordinates
{
	WINDOW_X = 0,
	WINDOW_Y = 0,
	WINDOW_W = 42,
	WINDOW_H = 42,
	
	MARKER_X = WINDOW_W,
	MARKER_Y = 0,
	MARKER_W = 16,
	MARKER_H = 18,
	
	SUNKENBORDER_X = 0,
	SUNKENBORDER_Y = WINDOW_H,
	SUNKENBORDER_W = 21,
	SUNKENBORDER_H = 21,
	
	SCROLLBAR_X = WINDOW_W,
	SCROLLBAR_Y = MARKER_H,
	SCROLLBAR_W = 16,
	SCROLLBAR_H = 27,
	
	SCROLLBAR_PICKER_X = 0,
	SCROLLBAR_PICKER_Y = 0,
	SCROLLBAR_PICKER_W = 12,
	SCROLLBAR_PICKER_H = 23,
	
	SCROLLBAR_ARROW_X = 0,
	SCROLLBAR_ARROW_Y = 0,
	SCROLLBAR_ARROW_W = 16,
	SCROLLBAR_ARROW_H = 26,
	
	WINDOW_POLE_X = 0,
	WINDOW_POLE_Y = 72,
	WINDOW_POLE_W = 29,
	WINDOW_POLE_H = 56,
	WINDOW_POLE_TOPS = 3,
	
	// SelectionCursor coordinates
	CURSOR_LEFT_X      = 0,
	CURSOR_LEFT_Y      = 0,
	CURSOR_LEFT_WIDTH  = 28,
	CURSOR_LEFT_HEIGHT = 21,
	
	CURSOR_RIGHT_X      = 28,
	CURSOR_RIGHT_Y      = 0,
	CURSOR_RIGHT_WIDTH  = 28,
	CURSOR_RIGHT_HEIGHT = 21,
	
	// System back button coordinates
	BACKBUTTON_X      = 0,
	BACKBUTTON_Y      = 0,
	BACKBUTTON_WIDTH  = 25,
	BACKBUTTON_HEIGHT = 26,
	
	// Filesystem blocks bar UV coordinates
	// - Background texture
	FILEBLOCKSBAR_BACKGROUND_X      = 0,
	FILEBLOCKSBAR_BACKGROUND_Y      = 0,
	FILEBLOCKSBAR_BACKGROUND_WIDTH  = 12,
	FILEBLOCKSBAR_BACKGROUND_HEIGHT = 16,
	
	// - Overlay texture
	FILEBLOCKSBAR_OVERLAY_X      = 0,
	FILEBLOCKSBAR_OVERLAY_Y      = 0,
	FILEBLOCKSBAR_OVERLAY_WIDTH  = 12,
	FILEBLOCKSBAR_OVERLAY_HEIGHT = 16,
	
	// Filesystem blocks bar UV coordinates
	// - Overlay texture
	BUSYBAR_OVERLAY_X      = 0,
	BUSYBAR_OVERLAY_Y      = 0,
	BUSYBAR_OVERLAY_WIDTH  = 12,
	BUSYBAR_OVERLAY_HEIGHT = 16
};


enum LabelSettings
{
	LABEL_SHADOW_OFFSET_X        = 2,
	LABEL_SHADOW_OFFSET_Y        = 2,
	LABEL_SCROLL_DELAY           = 30
};


extern engine::renderer::ColorRGBA LABEL_DEFAULT_TEXT_COLOR;
extern engine::renderer::ColorRGBA LABEL_DEFAULT_SHADOW_COLOR;
extern engine::renderer::ColorRGBA LABEL_DEFAULT_SELECTED_COLOR;
extern engine::renderer::ColorRGBA LABEL_DEFAULT_DISABLED_COLOR;

// Namespace end
}
}
}


#endif  // !defined(INC_TT_MENU_ELEMENTS_ELEMENTLAYOUT_H)
