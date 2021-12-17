#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTETEXT_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTETEXT_H


#include <string>

#include <tt/undo/UndoCommand.h>

#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandChangeNoteText;
typedef tt_ptr<CommandChangeNoteText>::shared CommandChangeNoteTextPtr;


class CommandChangeNoteText : public tt::undo::UndoCommand
{
public:
	static CommandChangeNoteTextPtr create(const level::NotePtr& p_noteToChange);
	virtual ~CommandChangeNoteText();
	
	virtual void redo();
	virtual void undo();
	
	void setText(const std::wstring& p_text);
	
	bool isTextChanged() const;
	
	inline const std::wstring& getOriginalText() const { return m_originalText; }
	
private:
	explicit CommandChangeNoteText(const level::NotePtr& p_noteToChange);
	
	
	bool               m_addedToStack;
	level::NotePtr     m_note;
	const std::wstring m_originalText;
	std::wstring       m_newText;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTETEXT_H)
