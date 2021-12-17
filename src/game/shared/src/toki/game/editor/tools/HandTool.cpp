#include <toki/game/editor/tools/HandTool.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/game/Camera.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

//--------------------------------------------------------------------------------------------------
// Public member functions

HandTool::HandTool(Editor* p_editor)
:
Tool(p_editor)
{
}


HandTool::~HandTool()
{
}


void HandTool::onActivate()
{
	//TT_Printf("HandTool::onActivate: Activate!\n");
}


void HandTool::onDeactivate()
{
	//TT_Printf("HandTool::onDeactivate: Deactivate!\n");
	getEditor()->restoreDefaultCursor();
}


void HandTool::onPointerHover(const PointerState& /*p_pointerState*/)
{
	getEditor()->setCursor(EditCursor_OpenHand);
}


void HandTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	//TT_Printf("HandTool::onPointerLeftPressed: Pressed.\n");
	getEditor()->setCursor(EditCursor_ClosedHand);
	m_dragStartWorldPos = p_pointerState.worldPos;
}


void HandTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	//TT_Printf("HandTool::onPointerLeftDown: Down.\n");
	const tt::math::Vector2 distanceWorld(m_dragStartWorldPos - p_pointerState.worldPos);
	Camera& cam(getEditor()->getEditorCamera());
	cam.setPosition(cam.getCurrentPosition() + distanceWorld, true);
}


void HandTool::onPointerLeftReleased(const PointerState& p_pointerState)
{
	//TT_Printf("HandTool::onPointerLeftReleased: Released.\n");
	
	tt::math::Vector2 diff(p_pointerState.screenPosCur - p_pointerState.screenPosPrev);
	if (diff.lengthSquared() > 25) // 5 pixels deadzone. (Check against 5^2). FIXME: Add to cfg.
	{
		const tt::math::Vector2 distanceWorld(m_dragStartWorldPos - p_pointerState.worldPos);
		Camera& cam(getEditor()->getEditorCamera());
		cam.initSpeedFromPos(cam.getCurrentPosition() + distanceWorld);
	}
}


void HandTool::onPointerMiddlePressed(const PointerState& p_pointerState)
{
	onPointerLeftPressed(p_pointerState);
}


void HandTool::onPointerMiddleDown(const PointerState& p_pointerState)
{
	onPointerLeftDown(p_pointerState);
}


void HandTool::onPointerMiddleReleased(const PointerState& p_pointerState)
{
	onPointerLeftReleased(p_pointerState);
}


std::wstring HandTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_HAND");
}

// Namespace end
}
}
}
}
