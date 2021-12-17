#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_BOXSELECTTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_BOXSELECTTOOL_H


#include <tt/math/Point2.h>
#include <tt/math/Rect.h>

#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class BoxSelectTool : public Tool
{
public:
	explicit BoxSelectTool(Editor* p_editor);
	virtual ~BoxSelectTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	
	virtual void onPointerHover       (const PointerState& p_pointerState);
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual void onPointerEnter();
	virtual void onPointerLeave();
	
	virtual std::wstring getHelpText() const;
	
private:
	enum Mode
	{
		Mode_CreateSelection,
		Mode_MoveSelection,
		Mode_MoveContents
	};
	
	
	void handleCreateSelection(const PointerState& p_pointerState);
	void handleMoveSelection  (const PointerState& p_pointerState);
	void handleMoveContents   (const PointerState& p_pointerState);
	
	
	Mode              m_mode;
	bool              m_startedSelection;
	tt::math::Vector2 m_startWorldPos;
	tt::math::Point2  m_startTilePos;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_BOXSELECTTOOL_H)
