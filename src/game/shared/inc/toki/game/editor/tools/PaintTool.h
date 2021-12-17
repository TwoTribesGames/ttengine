#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_PAINTTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_PAINTTOOL_H


#include <toki/game/editor/commands/CommandPaintTiles.h>
#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class PaintTool : public Tool
{
public:
	explicit PaintTool(Editor* p_editor);
	virtual ~PaintTool();
	
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
	
	virtual void onPointerLeave();
	
	virtual std::wstring getHelpText() const;
	
private:
	enum Mode
	{
		Mode_None,
		Mode_Draw,
		Mode_Erase,
		Mode_PickTile
	};
	
	
	void resetLastDrawPos();
	void drawFromLastPosTo(const tt::math::Point2& p_newTilePos);
	void drawTile(const tt::math::Point2& p_tilePos);
	void pickTile(const tt::math::Point2& p_tilePos);
	commands::CommandPaintTilesPtr createUndoCommand();
	void commitUndoCommand();
	
	
	commands::CommandPaintTilesPtr m_activeCommand;
	bool                           m_haveLastTilePos;  //!< Whether the last tile position can be used
	tt::math::Point2               m_lastTilePos;
	Mode                           m_mode; //!< The current paint mode.
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_PAINTTOOL_H)
