#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandAddRemoveNotes.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>
#include <toki/level/Note.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandAddRemoveNotesPtr CommandAddRemoveNotes::createForAdd(Editor* p_editor)
{
	TT_NULL_ASSERT(p_editor);
	return CommandAddRemoveNotesPtr(new CommandAddRemoveNotes(p_editor, Mode_Add));
}


CommandAddRemoveNotesPtr CommandAddRemoveNotes::createForRemove(Editor* p_editor)
{
	TT_NULL_ASSERT(p_editor);
	return CommandAddRemoveNotesPtr(new CommandAddRemoveNotes(p_editor, Mode_Remove));
}


CommandAddRemoveNotes::~CommandAddRemoveNotes()
{
}


void CommandAddRemoveNotes::redo()
{
	// Do not perform the add/remove operation the first time, since this was already handled at creation time
	// (so that there is instant visual feedback for edit operations)
	if (m_addedToStack)
	{
		if (m_mode == Mode_Add)
		{
			addNotesToLevelData(m_notes);
		}
		else
		{
			removeNotesFromLevelData(m_notes);
		}
	}
	
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
}


void CommandAddRemoveNotes::undo()
{
	if (m_mode == Mode_Add)
	{
		removeNotesFromLevelData(m_notes);
	}
	else
	{
		addNotesToLevelData(m_notes);
	}
}


void CommandAddRemoveNotes::addNote(const level::NotePtr& p_note)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call addNote after this command has already been added to an undo stack.");
		return;
	}
	
	addNotes(level::Notes(1, p_note));
}


void CommandAddRemoveNotes::addNotes(const level::Notes& p_notes)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call addNotes after this command has already been added to an undo stack.");
		return;
	}
	
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::Notes::const_iterator it = p_notes.begin(); it != p_notes.end(); ++it)
	{
		const level::NotePtr& note(*it);
		TT_NULL_ASSERT(note);
		
		// Only add notes that weren't known to the command yet
		if (std::find(m_notes.begin(), m_notes.end(), note) == m_notes.end())
		{
			if (m_mode == Mode_Add)
			{
				levelData->addNote(note);
			}
			else
			{
				levelData->removeNote(note);
				note->destroyVisual();
			}
			
			m_notes.push_back(note);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandAddRemoveNotes::CommandAddRemoveNotes(Editor* p_editor,
                                             Mode    p_mode)
:
tt::undo::UndoCommand(p_mode == Mode_Add ? L"Add Note" : L"Remove Notes"),
m_addedToStack(false),
m_editor(p_editor),
m_notes(),
m_mode(p_mode)
{
}


void CommandAddRemoveNotes::addNotesToLevelData(const level::Notes& p_notes)
{
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::Notes::const_iterator it = p_notes.begin(); it != p_notes.end(); ++it)
	{
		levelData->addNote(*it);
	}
}


void CommandAddRemoveNotes::removeNotesFromLevelData(const level::Notes& p_notes)
{
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::Notes::const_iterator it = p_notes.begin(); it != p_notes.end(); ++it)
	{
		const level::NotePtr& note(*it);
		TT_NULL_ASSERT(note);
		levelData->removeNote(note);
		note->destroyVisual();
	}
}

// Namespace end
}
}
}
}
