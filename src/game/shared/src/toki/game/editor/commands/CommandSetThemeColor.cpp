#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandSetThemeColor.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandSetThemeColorPtr CommandSetThemeColor::create(
		Editor*                                p_editor,
		level::skin::SkinConfigType            p_skinConfig,
		level::ThemeType                       p_theme,
		const tt::engine::renderer::ColorRGBA& p_newColor)
{
	TT_NULL_ASSERT(p_editor);
	const bool validSkinConfig = level::skin::isValidSkinConfigType(p_skinConfig);
	const bool validTheme      = level::isValidThemeType(p_theme);
	
	TT_ASSERTMSG(validSkinConfig, "Invalid skin config: %d", p_skinConfig);
	TT_ASSERTMSG(validTheme,      "Invalid theme: %d",       p_theme);
	if (p_editor == 0 || validSkinConfig == false || validTheme == false)
	{
		return CommandSetThemeColorPtr();
	}
	
	return CommandSetThemeColorPtr(new CommandSetThemeColor(
			p_editor,
			p_skinConfig,
			p_theme,
			p_editor->getLevelData()->getThemeColor(p_skinConfig, p_theme),
			p_newColor));
}


CommandSetThemeColor::~CommandSetThemeColor()
{
}


void CommandSetThemeColor::redo()
{
	m_editor->getLevelData()->setThemeColor(m_skinConfig, m_theme, m_newColor);
	m_editor->onThemeColorChanged(m_skinConfig, m_theme);
}


void CommandSetThemeColor::undo()
{
	m_editor->getLevelData()->setThemeColor(m_skinConfig, m_theme, m_originalColor);
	m_editor->onThemeColorChanged(m_skinConfig, m_theme);
}


bool CommandSetThemeColor::hasChanges() const
{
	return m_newColor != m_originalColor;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandSetThemeColor::CommandSetThemeColor(Editor*                                p_editor,
                                           level::skin::SkinConfigType            p_skinConfig,
                                           level::ThemeType                       p_theme,
                                           const tt::engine::renderer::ColorRGBA& p_originalColor,
                                           const tt::engine::renderer::ColorRGBA& p_newColor)
:
tt::undo::UndoCommand(L"Set Theme Color"),
m_editor       (p_editor),
m_skinConfig   (p_skinConfig),
m_theme        (p_theme),
m_originalColor(p_originalColor),
m_newColor     (p_newColor)
{
}

// Namespace end
}
}
}
}
