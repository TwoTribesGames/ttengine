#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_FLOODFILLTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_FLOODFILLTOOL_H


#include <toki/game/editor/commands/CommandPaintTiles.h>
#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

class FloodFillTool : public Tool
{
public:
	explicit FloodFillTool(Editor* p_editor);
	virtual ~FloodFillTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	
	virtual void onPointerHover       (const PointerState& p_pointerState);
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerRightPressed(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
	void floodFill(const tt::math::Point2& p_startTilePos,
	               level::CollisionType    p_paintTile);
	void floodFill(const tt::math::Point2& p_startTilePos,
	               level::ThemeType        p_paintTile);
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_FLOODFILLTOOL_H)
