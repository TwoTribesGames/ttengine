#if !defined(INC_TT_UNDO_FWD_H)
#define INC_TT_UNDO_FWD_H


#include <tt/platform/tt_types.h>


namespace tt {
namespace undo {

class UndoCommand;
typedef tt_ptr<UndoCommand      >::shared UndoCommandPtr;
typedef tt_ptr<const UndoCommand>::shared ConstUndoCommandPtr;

class UndoCommandGroup;
typedef tt_ptr<UndoCommandGroup>::shared UndoCommandGroupPtr;

class UndoStack;
typedef tt_ptr<UndoStack>::shared UndoStackPtr;

// Namespace end
}
}


#endif  // !defined(INC_TT_UNDO_FWD_H)
