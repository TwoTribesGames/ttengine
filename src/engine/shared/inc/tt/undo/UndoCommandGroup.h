#if !defined(INC_TT_UNDO_UNDOCOMMANDGROUP_H)
#define INC_TT_UNDO_UNDOCOMMANDGROUP_H


#include <vector>

#include <tt/undo/UndoCommand.h>


namespace tt {
namespace undo {

/*! \brief Groups one or more undo commands together (so they form one undo/redo action). */
class UndoCommandGroup : public UndoCommand
{
public:
	static UndoCommandGroupPtr create(const std::wstring& p_displayName);
	virtual ~UndoCommandGroup();
	
	/*! \brief Add an UndoCommand to the group.
	           Can only be called if this group has not been added to an UndoStack yet.
	           Cannot add the same command more than once.
	    \param p_command The command to add. Must be a valid pointer. */
	void addCommand(const UndoCommandPtr& p_command);
	
	virtual void redo();
	virtual void undo();
	
private:
	typedef std::vector<UndoCommandPtr> UndoCommands;
	
	explicit UndoCommandGroup(const std::wstring& p_displayName);
	
	
	bool         m_addedToStack; //!< For sanity checking: whether this command has been added to the undo stack
	UndoCommands m_commands;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_UNDO_UNDOCOMMANDGROUP_H)
