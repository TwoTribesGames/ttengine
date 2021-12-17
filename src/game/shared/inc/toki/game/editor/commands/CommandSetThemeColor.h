#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETTHEMECOLOR_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETTHEMECOLOR_H


#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/skin/types.h>
#include <toki/level/types.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandSetThemeColor;
typedef tt_ptr<CommandSetThemeColor>::shared CommandSetThemeColorPtr;


class CommandSetThemeColor : public tt::undo::UndoCommand
{
public:
	static CommandSetThemeColorPtr create(Editor*                                p_editor,
	                                      level::skin::SkinConfigType            p_skinConfig,
	                                      level::ThemeType                       p_theme,
	                                      const tt::engine::renderer::ColorRGBA& p_newColor);
	virtual ~CommandSetThemeColor();
	
	virtual void redo();
	virtual void undo();
	
	bool hasChanges() const;
	
private:
	CommandSetThemeColor(Editor*                                p_editor,
	                     level::skin::SkinConfigType            p_skinConfig,
	                     level::ThemeType                       p_theme,
	                     const tt::engine::renderer::ColorRGBA& p_originalColor,
	                     const tt::engine::renderer::ColorRGBA& p_newColor);
	
	
	Editor* const                         m_editor;
	const level::skin::SkinConfigType     m_skinConfig;
	const level::ThemeType                m_theme;
	const tt::engine::renderer::ColorRGBA m_originalColor;
	const tt::engine::renderer::ColorRGBA m_newColor;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDSETTHEMECOLOR_H)
