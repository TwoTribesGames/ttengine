#include <Gwen/Controls/Button.h>
#include <Gwen/Controls/RGBColorPicker.h>
#include <Gwen/Controls/Label.h>
#include <Gwen/Controls/Menu.h>

#include <tt/gwen/utils.h>
#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandSetThemeColor.h>
#include <toki/game/editor/ui/SetThemeColorUI.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace ui {

//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(SetThemeColorUI)
,
m_editor(0)
{
	for (s32 skinIndex = 0; skinIndex < level::skin::SkinConfigType_Count; ++skinIndex)
	{
		for (s32 themeIndex = 0; themeIndex < level::ThemeType_Count; ++themeIndex)
		{
			m_colorButtons[skinIndex][themeIndex] = 0;
		}
	}
	
	SetMinimumSize(Gwen::Point(50, 50));
	SetSize(180, 200);  // NOTE: Default height based on theme UI will be set in createUi
	SetClosable(true);
	SetDeleteOnClose(false);
}


SetThemeColorUI::~SetThemeColorUI()
{
}


SetThemeColorUI* SetThemeColorUI::create(Editor*               p_editor,
                                         const std::string&    p_title,
                                         Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(p_editor);
	if (p_editor == 0) return 0;
	
	SetThemeColorUI* dlg = new SetThemeColorUI(p_parent);
	dlg->m_editor = p_editor;
	dlg->SetTitle(p_title);
	dlg->createUi();
	return dlg;
}


void SetThemeColorUI::onThemeColorChanged(level::skin::SkinConfigType p_skinConfig, level::ThemeType p_theme)
{
	TT_ASSERT(level::skin::isValidSkinConfigType(p_skinConfig));
	TT_ASSERT(level::isValidThemeType(p_theme));
	
	if (m_colorButtons[p_skinConfig][p_theme] != 0)
	{
		m_colorButtons[p_skinConfig][p_theme]->SetColor(tt::gwen::toGwenColor(
				m_editor->getLevelData()->getThemeColor(p_skinConfig, p_theme)));
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void SetThemeColorUI::createUi()
{
	TT_NULL_ASSERT(m_editor);
	
	Gwen::Controls::Base* themeParent = this;
	
	int totalUiHeight = 40;  // start with some height for the title bar and border
	
	// Add header labels for the skin config types
	Gwen::Controls::Base* header = new Gwen::Controls::Base(themeParent);
	
	header->SetPadding(Gwen::Padding(0, 0, 0, 5));
	
	static const int labelSpacing = 60;
	int labelX = 95;
	for (s32 i = 0; i < level::skin::SkinConfigType_Count; ++i)
	{
		const level::skin::SkinConfigType type = static_cast<level::skin::SkinConfigType>(i);
		Gwen::Controls::Label* label = new Gwen::Controls::Label(header);
		label->SetText(getSkinConfigDisplayName(type));
		label->SizeToContents();
		label->SetWidth(labelSpacing);
		label->SetPos(labelX, 0);
		labelX += labelSpacing;
	}
	
	header->SizeToChildren();
	header->Dock(Gwen::Pos::Top);
	
	totalUiHeight += header->Height();
	
	// Create the edit controls for the theme colors
	for (s32 i = 0; i < level::ThemeType_Count; ++i)
	{
		const level::ThemeType theme = static_cast<level::ThemeType>(i);
		if (theme == level::ThemeType_DoNotTheme ||
		    theme == level::ThemeType_UseLevelDefault)
		{
			// Do not allow setting colors for "Do Not Theme" or "Use Level Default"
			// (won't have any effect)
			continue;
		}
		
		Gwen::Controls::Base* ui = addThemeUI(theme, themeParent);
		totalUiHeight += ui->Height();
	}
	
	SetHeight(totalUiHeight);
}


Gwen::Controls::Base* SetThemeColorUI::addThemeUI(level::ThemeType p_theme, Gwen::Controls::Base* p_parent)
{
	TT_NULL_ASSERT(m_editor);
	
	Gwen::Controls::Base* container = new Gwen::Controls::Base(p_parent);
	
	Gwen::Controls::Label* label = new Gwen::Controls::Label(container);
	label->SetText(getThemeDisplayName(p_theme));
	label->SizeToContents();
	label->SetWidth(80);
	label->Dock(Gwen::Pos::Left);
	
	for (s32 skinIndex = 0; skinIndex < level::skin::SkinConfigType_Count; ++skinIndex)
	{
		const level::skin::SkinConfigType skinConfig = static_cast<level::skin::SkinConfigType>(skinIndex);
		
		tt::gwen::ColorButton* colorButton = new tt::gwen::ColorButton(container);
		colorButton->SetWidth(50);
		colorButton->SetColor(tt::gwen::toGwenColor(m_editor->getLevelData()->getThemeColor(skinConfig, p_theme)));
		colorButton->UserData.Set("theme",      p_theme);
		colorButton->UserData.Set("skinConfig", skinConfig);
		colorButton->onPress.Add(this, &SetThemeColorUI::onColorButtonClicked);
		colorButton->Dock(Gwen::Pos::Left);
		colorButton->SetMargin(Gwen::Margin(10, 0, 0, 0));
		m_colorButtons[skinIndex][p_theme] = colorButton;
	}
	
	container->SetHeight(30);
	container->SetPadding(Gwen::Padding(5, 5, 0, 5));
	container->Dock(Gwen::Pos::Top);
	
	return container;
}


void SetThemeColorUI::onColorButtonClicked(Gwen::Controls::Base* p_sender)
{
	tt::gwen::ColorButton* colorButton = gwen_cast<tt::gwen::ColorButton>(p_sender);
	TT_NULL_ASSERT(colorButton);
	if (colorButton == 0) return;
	
	const level::skin::SkinConfigType skinConfig = colorButton->UserData.Get<level::skin::SkinConfigType>("skinConfig");
	const level::ThemeType            theme      = colorButton->UserData.Get<level::ThemeType           >("theme");
	TT_ASSERT(level::skin::isValidSkinConfigType(skinConfig));
	TT_ASSERT(level::isValidThemeType(theme));
	
	TT_NULL_ASSERT(m_editor);
	
	// Pop up a color picker to allow setting a new theme color
	Gwen::Controls::Menu* menu = new Gwen::Controls::Menu(p_sender->GetCanvas());
	menu->SetSize(256, 180);
	menu->SetDeleteOnClose(true);
	menu->SetDisableIconMargin(true);
	
	Gwen::Controls::RGBColorPicker* picker = new Gwen::Controls::RGBColorPicker(menu);
	picker->Dock(Gwen::Pos::Fill);
	picker->SetSize(256, 128);
	
	// NOTE: Re-querying the level setting (instead of getting color from button)
	//       to minimize chance of "sync issues" between level and button
	picker->SetColor(tt::gwen::toGwenColor(m_editor->getLevelData()->getThemeColor(skinConfig, theme)), false, true);
	
	{
		Gwen::Controls::Base* dock = new Gwen::Controls::Base(menu);
		
		Gwen::Controls::Button* buttonCancel = new Gwen::Controls::Button(dock);
		buttonCancel->SetText(translateString("BUTTON_CANCEL"));
		buttonCancel->SetWidth(75);
		buttonCancel->UserData.Set("colorPicker", picker);
		buttonCancel->UserData.Set("parentMenu",  menu);
		buttonCancel->onPress.Add(this, &SetThemeColorUI::onColorPickerCancel);
		buttonCancel->Dock(Gwen::Pos::Right);
		
		Gwen::Controls::Button* buttonOk = new Gwen::Controls::Button(dock);
		buttonOk->SetText(translateString("BUTTON_OK"));
		buttonOk->SetWidth(75);
		buttonOk->SetMargin(Gwen::Margin(0, 0, 5, 0));
		buttonOk->UserData.Set("colorPicker", picker);
		buttonOk->UserData.Set("parentMenu",  menu);
		buttonOk->UserData.Set("skinConfig",  skinConfig);
		buttonOk->UserData.Set("theme",       theme);
		buttonOk->onPress.Add(this, &SetThemeColorUI::onColorPickerOK);
		buttonOk->Dock(Gwen::Pos::Right);
		
		dock->SetPadding(Gwen::Padding(0, 5, 5, 5));
		dock->SetHeight(30);
		dock->Dock(Gwen::Pos::Bottom);
	}
	
	// FIXME: Open to the left if no room remains on the right (same for the opposite situation)
	menu->Open(Gwen::Pos::Right | Gwen::Pos::Top);
	//menu->Open(Gwen::Pos::Left | Gwen::Pos::Top);
}


void SetThemeColorUI::onColorPickerOK(Gwen::Controls::Base* p_sender)
{
	TT_NULL_ASSERT(p_sender);
	TT_NULL_ASSERT(m_editor);
	if (p_sender == 0 || m_editor == 0) return;
	
	Gwen::Controls::Menu*             parentMenu  = p_sender->UserData.Get<Gwen::Controls::Menu*          >("parentMenu");
	Gwen::Controls::RGBColorPicker*   colorPicker = p_sender->UserData.Get<Gwen::Controls::RGBColorPicker*>("colorPicker");
	const level::skin::SkinConfigType skinConfig  = p_sender->UserData.Get<level::skin::SkinConfigType    >("skinConfig");
	const level::ThemeType            theme       = p_sender->UserData.Get<level::ThemeType               >("theme");
	
	if (parentMenu == 0 || colorPicker == 0)
	{
		TT_PANIC("Button does not have 'parentMenu' or 'colorPicker' user data.");
		return;
	}
	
	TT_ASSERT(level::skin::isValidSkinConfigType(skinConfig));
	TT_ASSERT(level::isValidThemeType(theme));
	
	const tt::engine::renderer::ColorRGBA selectedColor(tt::gwen::toEngineColor(colorPicker->GetColor()));
	
	parentMenu->Close();
	
	commands::CommandSetThemeColorPtr cmd = commands::CommandSetThemeColor::create(
			m_editor, skinConfig, theme, selectedColor);
	
	if (cmd != 0 && cmd->hasChanges())
	{
		m_editor->pushUndoCommand(cmd);
	}
}


void SetThemeColorUI::onColorPickerCancel(Gwen::Controls::Base* p_sender)
{
	TT_NULL_ASSERT(p_sender);
	if (p_sender == 0) return;
	
	Gwen::Controls::Menu* parentMenu = p_sender->UserData.Get<Gwen::Controls::Menu*>("parentMenu");
	TT_ASSERTMSG(parentMenu != 0, "Button does not have 'parentMenu' user data.");
	if (parentMenu != 0)
	{
		parentMenu->Close();
	}
}

// Namespace end
}
}
}
}
