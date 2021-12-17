#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTERECT_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTERECT_H


#include <tt/math/Rect.h>
#include <tt/undo/UndoCommand.h>

#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandChangeNoteRect;
typedef tt_ptr<CommandChangeNoteRect>::shared CommandChangeNoteRectPtr;


class CommandChangeNoteRect : public tt::undo::UndoCommand
{
public:
	static CommandChangeNoteRectPtr create(const level::NotePtr& p_noteToChange);
	virtual ~CommandChangeNoteRect();
	
	virtual void redo();
	virtual void undo();
	
	void setPosition(const tt::math::Vector2& p_pos);
	void setRect(const tt::math::VectorRect& p_rect);
	
private:
	explicit CommandChangeNoteRect(const level::NotePtr& p_noteToChange);
	
	
	bool                       m_addedToStack;
	level::NotePtr             m_note;
	const tt::math::VectorRect m_originalRect;
	tt::math::VectorRect       m_newRect;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGENOTERECT_H)
