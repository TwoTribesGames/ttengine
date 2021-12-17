#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_RESIZETOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_RESIZETOOL_H


#include <tt/math/Rect.h>

#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class ResizeTool : public Tool
{
public:
	explicit ResizeTool(Editor* p_editor);
	virtual ~ResizeTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	virtual bool canActivate(const PointerState& p_pointerState) const;
	virtual bool canDeactivate() const;
	
	virtual void onPointerHover       (const PointerState& p_pointerState);
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
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
	
	
	DragEdge getDragEdge(const tt::math::Vector2& p_worldPos) const;
	void     setCursorForDragEdge(DragEdge p_edge);
	
	
	bool                m_startedDrag;
	bool                m_finishedDrag;
	DragEdge            m_dragEdge;
	tt::math::Vector2   m_dragStartWorldPos;
	tt::math::PointRect m_startingLevelRect;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_RESIZETOOL_H)
