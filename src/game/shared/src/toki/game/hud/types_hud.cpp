#include <tt/script/helpers.h>

#include <toki/game/hud/types.h>


namespace toki {
namespace game {
namespace hud {

void ListBoxColorScheme::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT(ListBoxColorScheme);
	
	// Bind members
	using tt::engine::renderer::ColorRGBA;
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, background);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, backgroundHover);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemOdd);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemEven);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemSelected);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemHover);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemText);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemTextSelected);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, itemTextHover);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, scrollBar);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, scrollBarGrip);
	TT_SQBIND_MEMBER(ListBoxColorScheme, ColorRGBA, scrollBarHover);
};

// Namespace end
}
}
}
