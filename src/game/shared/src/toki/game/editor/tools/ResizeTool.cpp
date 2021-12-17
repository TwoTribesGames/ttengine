#include <limits>

#include <tt/math/math.h>

#include <toki/game/editor/commands/CommandResizeLevel.h>
#include <toki/game/editor/tools/ResizeTool.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

//--------------------------------------------------------------------------------------------------
// Public member functions

ResizeTool::ResizeTool(Editor* p_editor)
:
Tool(p_editor),
m_startedDrag(false),
m_finishedDrag(false),
m_dragEdge(DragEdge_None),
m_dragStartWorldPos(-1.0f, -1.0f),
m_startingLevelRect(tt::math::Point2(0, 0), 0, 0)
{
}


ResizeTool::~ResizeTool()
{
}


void ResizeTool::onActivate()
{
	m_finishedDrag = false;
}


void ResizeTool::onDeactivate()
{
	getEditor()->restoreDefaultCursor();
}


bool ResizeTool::canActivate(const PointerState& p_pointerState) const
{
	return getDragEdge(p_pointerState.worldPos) != DragEdge_None;
}


bool ResizeTool::canDeactivate() const
{
	return m_finishedDrag;
}


void ResizeTool::onPointerHover(const PointerState& p_pointerState)
{
	if (m_startedDrag == false)
	{
		setCursorForDragEdge(getDragEdge(p_pointerState.worldPos));
	}
}


void ResizeTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	m_dragEdge          = getDragEdge(p_pointerState.worldPos);
	m_startedDrag       = false;
	m_dragStartWorldPos = p_pointerState.worldPos;
	m_startingLevelRect = getEditor()->getLevelBorderRect();
	
	setCursorForDragEdge(m_dragEdge);
	
	if (m_dragEdge != DragEdge_None)
	{
		getEditor()->clearSelectionRect(true);
	}
}


void ResizeTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	if (m_dragEdge == DragEdge_None)
	{
		return;
	}
	
	if (m_startedDrag == false)
	{
		if (tt::math::distance(m_dragStartWorldPos, p_pointerState.worldPos) < (level::tileToWorld(1) * 0.5f))
		{
			// Pointer did not move enough yet to start the movement
			return;
		}
		
		m_startedDrag = true;
	}
	
	
	static const s32 maxSize = std::numeric_limits<s32>::max();
	
	const tt::math::Point2 tilesMoved(level::worldToTile(p_pointerState.worldPos) -
	                                  level::worldToTile(m_dragStartWorldPos));
	
	tt::math::PointRect newRect(m_startingLevelRect);
	switch (m_dragEdge)
	{
	case DragEdge_TopRight:
	case DragEdge_Right:
	case DragEdge_BottomRight:
		{
			s32 newWidth = m_startingLevelRect.getWidth() + tilesMoved.x;
			tt::math::clamp(newWidth, s32(1), maxSize);
			newRect.setWidth(newWidth);
		}
		break;
		
	case DragEdge_TopLeft:
	case DragEdge_Left:
	case DragEdge_BottomLeft:
		{
			s32 newLeft = m_startingLevelRect.getLeft() + tilesMoved.x;
			tt::math::clamp(newLeft, newRect.getMaxEdge().x - maxSize, newRect.getRight());
			newRect.setLeft(newLeft);
		}
		break;
		
	default:
		break;
	}
	
	switch (m_dragEdge)
	{
	case DragEdge_TopLeft:
	case DragEdge_Top:
	case DragEdge_TopRight:
		{
			s32 newHeight = m_startingLevelRect.getHeight() + tilesMoved.y;
			tt::math::clamp(newHeight, s32(1), maxSize);
			newRect.setHeight(newHeight);
		}
		break;
		
	case DragEdge_BottomLeft:
	case DragEdge_Bottom:
	case DragEdge_BottomRight:
		{
			s32 newTop = m_startingLevelRect.getTop() + tilesMoved.y;
			tt::math::clamp(newTop, newRect.getMaxEdge().y - maxSize, newRect.getBottom());
			newRect.setTop(newTop);
		}
		break;
		
	default:
		break;
	}
	
	
	getEditor()->overrideLevelBorderRect(newRect);
}


void ResizeTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	// If the level rectangle was changed, apply the changes now (resize the level for real)
	if (m_dragEdge != DragEdge_None &&
	    getEditor()->getLevelBorderRect() != m_startingLevelRect)
	{
		Editor* editor = getEditor();
		editor->pushUndoCommand(commands::CommandResizeLevel::create(
			editor,
			editor->getLevelData(),
			editor->getLevelBorderRect()));
	}
	else
	{
		getEditor()->resetLevelBorderRect();
	}
	
	m_startedDrag = false;
	m_finishedDrag = true;
	m_dragEdge    = DragEdge_None;
}


std::wstring ResizeTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_RESIZE");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

ResizeTool::DragEdge ResizeTool::getDragEdge(const tt::math::Vector2& p_worldPos) const
{
	DragEdge dragEdge = DragEdge_None;
	
	// Determine which part of the border was picked
	const real borderThickness = getEditor()->getLevelBorderThickness();
	level::LevelDataPtr levelData(getEditor()->getLevelData());
	
	const tt::math::PointRect levelRect(levelData->getLevelRect());
	const tt::math::Vector2   levelWorldMinPos(level::tileToWorld(levelRect.getMin()));
	const tt::math::Vector2   levelWorldMaxPos(level::tileToWorld(levelRect.getMaxEdge()));
	
	enum HitRegion
	{
		HitRegion_None,
		HitRegion_OuterMin,
		HitRegion_Inner,
		HitRegion_OuterMax
	};
	
	HitRegion hitHorz = HitRegion_None;
	HitRegion hitVert = HitRegion_None;
	
	if (p_worldPos.x <= levelWorldMinPos.x &&
	    p_worldPos.x >= (levelWorldMinPos.x - borderThickness))
	{
		hitHorz = HitRegion_OuterMin;
	}
	else if (p_worldPos.x >= levelWorldMinPos.x &&
	         p_worldPos.x <= levelWorldMaxPos.x)
	{
		hitHorz = HitRegion_Inner;
	}
	else if (p_worldPos.x >= levelWorldMaxPos.x &&
	         p_worldPos.x <= (levelWorldMaxPos.x + borderThickness))
	{
		hitHorz = HitRegion_OuterMax;
	}
	
	if (p_worldPos.y <= levelWorldMinPos.y &&
	    p_worldPos.y >= (levelWorldMinPos.y - borderThickness))
	{
		hitVert = HitRegion_OuterMin;
	}
	else if (p_worldPos.y >= levelWorldMinPos.y &&
	         p_worldPos.y <= levelWorldMaxPos.y)
	{
		hitVert = HitRegion_Inner;
	}
	else if (p_worldPos.y >= levelWorldMaxPos.y &&
	         p_worldPos.y <= (levelWorldMaxPos.y + borderThickness))
	{
		hitVert = HitRegion_OuterMax;
	}
	
	switch (hitVert)
	{
	case HitRegion_OuterMin:
	case HitRegion_OuterMax:
		switch (hitHorz)
		{
		case HitRegion_OuterMin: dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_BottomLeft  : DragEdge_TopLeft;  break;
		case HitRegion_Inner:    dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_Bottom      : DragEdge_Top;      break;
		case HitRegion_OuterMax: dragEdge = (hitVert == HitRegion_OuterMin) ? DragEdge_BottomRight : DragEdge_TopRight; break;
		default: break;
		}
		break;
		
	case HitRegion_Inner:
		if (hitHorz == HitRegion_OuterMin)
		{
			dragEdge = DragEdge_Left;
		}
		else if (hitHorz == HitRegion_OuterMax)
		{
			dragEdge = DragEdge_Right;
		}
		break;
		
	default: break;
	}
	
	return dragEdge;
}


void ResizeTool::setCursorForDragEdge(DragEdge p_edge)
{
	EditCursor cursor = EditCursor_NormalArrow;
	switch (p_edge)
	{
	//case DragEdge_None:        cursor = EditCursor_NotAllowed;               break;
	case DragEdge_None:        cursor = EditCursor_NormalArrow;              break;
	case DragEdge_TopLeft:     cursor = EditCursor_ResizeTopLeftBottomRight; break;
	case DragEdge_Top:         cursor = EditCursor_ResizeTopBottom;          break;
	case DragEdge_TopRight:    cursor = EditCursor_ResizeTopRightBottomLeft; break;
	case DragEdge_Right:       cursor = EditCursor_ResizeLeftRight;          break;
	case DragEdge_BottomRight: cursor = EditCursor_ResizeTopLeftBottomRight; break;
	case DragEdge_Bottom:      cursor = EditCursor_ResizeTopBottom;          break;
	case DragEdge_BottomLeft:  cursor = EditCursor_ResizeTopRightBottomLeft; break;
	case DragEdge_Left:        cursor = EditCursor_ResizeLeftRight;          break;
	default: break;
	}
	
	getEditor()->setCursor(cursor);
}

// Namespace end
}
}
}
}
