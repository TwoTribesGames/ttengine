#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_HANDTOOL_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_HANDTOOL_H


#include <tt/math/Vector2.h>

#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

/*! \brief Allows scrolling around the world. */
class HandTool : public Tool
{
public:
	explicit HandTool(Editor* p_editor);
	virtual ~HandTool();
	
	virtual void onActivate();
	virtual void onDeactivate();
	
	virtual void onPointerHover       (const PointerState& p_pointerState);
	virtual void onPointerLeftPressed (const PointerState& p_pointerState);
	virtual void onPointerLeftDown    (const PointerState& p_pointerState);
	virtual void onPointerLeftReleased(const PointerState& p_pointerState);
	
	virtual void onPointerMiddlePressed (const PointerState& p_pointerState);
	virtual void onPointerMiddleDown    (const PointerState& p_pointerState);
	virtual void onPointerMiddleReleased(const PointerState& p_pointerState);
	
	virtual std::wstring getHelpText() const;
	
private:
	tt::math::Vector2 m_dragStartWorldPos;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_HANDTOOL_H)
