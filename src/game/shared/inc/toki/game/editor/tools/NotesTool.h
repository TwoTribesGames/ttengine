#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_NOTESTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_NOTESTOOL_H


#include <tt/math/Rect.h>

#include <toki/game/editor/commands/CommandAddRemoveNotes.h>
#include <toki/game/editor/commands/CommandChangeNoteRect.h>
#include <toki/game/editor/tools/Tool.h>
#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class NotesTool : public Tool
{
public:
	explicit NotesTool(Editor* p_editor);
	virtual ~NotesTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	virtual bool canDeactivate() const;
	
	virtual void onPointerHover(const PointerState& p_pointerState);
	
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual void onPointerRightPressed (const PointerState& p_pointerState);
	virtual void onPointerRightDown    (const PointerState& p_pointerState);
	virtual void onPointerRightReleased(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
	enum Mode
	{
		Mode_Add,
		Mode_Remove,
		Mode_Move,
		Mode_Resize,
		
		Mode_None
	};
	
	enum DragEdge
	{
		DragEdge_None,
		DragEdge_TopLeft,
		DragEdge_Top,
		DragEdge_TopRight,
		DragEdge_Right,
		DragEdge_BottomRight,
		DragEdge_Bottom,
		DragEdge_BottomLeft,
		DragEdge_Left
	};
	
	
	void commitUndoCommand();
	DragEdge getNoteResizeEdge(const level::NotePtr& p_note, const PointerState& p_pointerState) const;
	DragEdge getDragEdge(const tt::math::Vector2& p_worldPos) const;
	void     setCursorForDragEdge(DragEdge p_edge);
	
	
	Mode m_mode;
	
	bool                 m_startedDrag;
	DragEdge             m_dragEdge;
	tt::math::Vector2    m_dragStartWorldPos;   // when resizing note
	tt::math::Point2     m_dragStartScreenPos;  // when resizing note
	tt::math::VectorRect m_startingNoteRect;
	
	tt::math::Vector2   m_dragOffset;  // when moving note
	
	level::NotePtr m_activeNote;  // Note that the tool is currently operating on (add, move, resize, edit)
	
	commands::CommandAddRemoveNotesPtr m_addRemoveCommand;
	commands::CommandChangeNoteRectPtr m_changeRectCommand;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_NOTESTOOL_H)
