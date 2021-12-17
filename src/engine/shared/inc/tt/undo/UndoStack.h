#if !defined(INC_TT_UNDO_UNDOSTACK_H)
#define INC_TT_UNDO_UNDOSTACK_H


#include <vector>

#include <tt/undo/fwd.h>


namespace tt {
namespace undo {

class UndoStack
{
public:
	/*! \param p_undoLimit The maximum number of commands on this stack.
	                       When the number of commands on a stack exceeds the stack's undo limit,
	                       commands are deleted from the bottom of the stack.
	                       The default value is 0, which means that there is no limit. */
	explicit UndoStack(s32 p_undoLimit = 0);
	~UndoStack();
	
	bool canUndo() const;
	bool canRedo() const;
	
	void undo();
	void redo();
	
	/*! \brief Removes all undo commands from the stack. */
	void clear();
	
	ConstUndoCommandPtr getCommand(s32 p_index) const;
	s32 getCommandCount() const;
	
	/*! \brief Adds a new command to the stack and applies its changes (calls redo() on the command). */
	void push(const UndoCommandPtr& p_command);
	
	inline s32 getCurrentIndex() const { return m_currentIndex; }
	void setCurrentIndex(s32 p_index);
	
	/*! \return Index of the command where the stack was last marked as "clean" (saved). */
	inline s32 getCleanIndex() const { return m_cleanIndex; }
	
	/*! \return Whether there are any unsaved changes. */
	inline bool isClean() const { return m_currentIndex == m_cleanIndex; }
	
	/*! \brief Marks the current undo command index as the "clean" (saved) state. */
	void setClean();
	
	inline s32 getUndoLimit() const { return m_undoLimit; }
	
private:
	typedef std::vector<UndoCommandPtr> UndoCommands;
	
	
	UndoCommandPtr internalGetCommand(s32 p_index);
	void enforceUndoLimit();
	
	// No copying
	UndoStack(const UndoStack&);
	UndoStack& operator=(const UndoStack&);
	
	
	const s32 m_undoLimit;
	s32       m_currentIndex; //!< Index of the current undo command.
	s32       m_cleanIndex;   //!< Index of the command where the stack was last marked as "clean" (saved).
	
	UndoCommands m_commands;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_UNDO_UNDOSTACK_H)
