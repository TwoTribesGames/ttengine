#if !defined(INC_TOKI_GAME_EDITOR_UI_SETTHEMECOLORUI_H)
#define INC_TOKI_GAME_EDITOR_UI_SETTHEMECOLORUI_H


#include <string>

#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/WindowControl.h>

#include <tt/gwen/ColorButton.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

class SetThemeColorUI : public Gwen::Controls::WindowControl
{
public:
	GWEN_CONTROL(SetThemeColorUI, Gwen::Controls::WindowControl);
	virtual ~SetThemeColorUI();
	
	static SetThemeColorUI* create(Editor*               p_editor,
	                               const std::string&    p_title,
	                               Gwen::Controls::Base* p_parent);
	
	void onThemeColorChanged(level::skin::SkinConfigType p_skinConfig, level::ThemeType p_theme);
	
private:
	void createUi();
	Gwen::Controls::Base* addThemeUI(level::ThemeType p_theme, Gwen::Controls::Base* p_parent);
	
	// UI handlers
	void onColorButtonClicked(Gwen::Controls::Base* p_sender);
	void onColorPickerOK     (Gwen::Controls::Base* p_sender);
	void onColorPickerCancel (Gwen::Controls::Base* p_sender);
	
	// No copying
	SetThemeColorUI(const SetThemeColorUI&);
	SetThemeColorUI& operator=(const SetThemeColorUI&);
	
	
	Editor*                m_editor;
	tt::gwen::ColorButton* m_colorButtons[level::skin::SkinConfigType_Count][level::ThemeType_Count];
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_UI_SETTHEMECOLORUI_H)
