#include <toki/game/editor/tools/Tool.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

//--------------------------------------------------------------------------------------------------
// Public member functions

Tool::Tool(Editor* p_editor)
:
m_editor(p_editor)
{
}


Tool::~Tool()
{
}


void Tool::onActivate()
{
}


void Tool::onDeactivate()
{
}


bool Tool::canActivate(const PointerState&) const
{
	return true;
}


bool Tool::canDeactivate() const
{
	return true;
}


void Tool::onPointerHover(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerLeftPressed(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerLeftDown(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerMiddlePressed(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerMiddleDown(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerMiddleReleased(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerRightPressed(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerRightDown(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerRightReleased(const PointerState& /*p_pointerState*/)
{
}


void Tool::onPointerEnter()
{
}


void Tool::onPointerLeave()
{
}


std::wstring Tool::getHelpText() const
{
	return std::wstring();
}

// Namespace end
}
}
}
}
