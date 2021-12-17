#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVENOTES_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVENOTES_H


#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandAddRemoveNotes;
typedef tt_ptr<CommandAddRemoveNotes>::shared CommandAddRemoveNotesPtr;


class CommandAddRemoveNotes : public tt::undo::UndoCommand
{
public:
	static CommandAddRemoveNotesPtr createForAdd   (Editor* p_editor);
	static CommandAddRemoveNotesPtr createForRemove(Editor* p_editor);
	virtual ~CommandAddRemoveNotes();
	
	virtual void redo();
	virtual void undo();
	
	/*! \brief Adds a note for the command to operate on (either add or remove,
	           depending on whether the command was created to add or remove notes). */
	void addNote (const level::NotePtr& p_note);
	void addNotes(const level::Notes&   p_notes);
	
	inline bool hasNotes() const { return m_notes.empty() == false; }
	
private:
	enum Mode
	{
		Mode_Add,
		Mode_Remove
	};
	
	
	CommandAddRemoveNotes(Editor* p_editor, Mode p_mode);
	void addNotesToLevelData     (const level::Notes& p_notes);
	void removeNotesFromLevelData(const level::Notes& p_notes);
	
	
	bool         m_addedToStack;
	Editor*      m_editor;
	level::Notes m_notes;
	const Mode   m_mode;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVENOTES_H)
