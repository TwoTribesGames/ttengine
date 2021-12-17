#if !defined(INC_TT_UNDO_UNDOCOMMAND_H)
#define INC_TT_UNDO_UNDOCOMMAND_H


#include <string>

#include <tt/platform/tt_types.h>
#include <tt/undo/fwd.h>


namespace tt {
namespace undo {

/*! \brief Base class for commands that can be undone via the undo framework.
           Derive from this class to create undoable operations. */
class UndoCommand
{
public:
	explicit UndoCommand(const std::wstring& p_displayName = std::wstring());
	virtual ~UndoCommand();
	
	/*! \brief Applies the command. */
	virtual void redo() = 0;
	
	/*! \brief Undoes the changes made by the command. */
	virtual void undo() = 0;
	
	inline const std::wstring& getDisplayName() const { return m_displayName; }
	
protected:
	inline void setDisplayName(const std::wstring& p_displayName) { m_displayName = p_displayName; }
	
private:
	// No copying
	UndoCommand(const UndoCommand&);
	UndoCommand& operator=(const UndoCommand&);
	
	
	std::wstring m_displayName;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_UNDO_UNDOCOMMAND_H)
