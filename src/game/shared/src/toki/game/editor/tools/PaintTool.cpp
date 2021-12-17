#include <tt/platform/tt_error.h>

#include <toki/game/editor/tools/PaintTool.h>
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

PaintTool::PaintTool(Editor* p_editor)
:
Tool(p_editor),
m_activeCommand(),
m_haveLastTilePos(false),
m_lastTilePos(-1, -1),
m_mode(Mode_None)
{
}


PaintTool::~PaintTool()
{
}


void PaintTool::onActivate()
{
	TT_ASSERTMSG(m_activeCommand == 0, "Active paint command wasn't cleaned up properly.");
	resetLastDrawPos();
	m_mode = Mode_None;
}


void PaintTool::onDeactivate()
{
	commitUndoCommand();
	getEditor()->restoreDefaultCursor();
}


bool PaintTool::canDeactivate() const
{
	return m_activeCommand == 0;
}


void PaintTool::onPointerHover(const PointerState& p_pointerState)
{
	if (p_pointerState.ctrl.down)
	{
		getEditor()->setCursor(getEditor()->hasEntityAtWorldPos(p_pointerState.worldPos) ?
			EditCursor_GenericMove : EditCursor_NormalArrow);
	}
	else
	{
		getEditor()->setCursor(p_pointerState.alt.down ? EditCursor_TilePicker : EditCursor_Draw);
	}
}


void PaintTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	if (p_pointerState.ctrl.down)
	{
		// Ctrl+left clicking should deselect all entities
		getEditor()->getLevelData()->deselectAllEntities();
		
		if (getEditor()->hasEntityAtWorldPos(p_pointerState.worldPos))
		{
			getEditor()->switchToToolUntilPointerReleased(ToolID_EntityMove);
		}
		return;
	}
	
	
	if (p_pointerState.alt.down)
	{
		// Pick tile
		m_mode = Mode_PickTile;
		pickTile(level::worldToTile(p_pointerState.worldPos));
		getEditor()->setCursor(EditCursor_TilePicker);
		return;
	}
	
	getEditor()->setCursor(EditCursor_Draw);
	
	if (m_mode != Mode_Draw && m_activeCommand != 0)
	{
		commitUndoCommand();
	}
	
	m_mode = Mode_Draw;
	
	m_haveLastTilePos = false; // first press never has a previous position
	drawFromLastPosTo(level::worldToTile(p_pointerState.worldPos));
}


void PaintTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	if (m_mode == Mode_Draw)
	{
		drawFromLastPosTo(level::worldToTile(p_pointerState.worldPos));
	}
	else if (m_mode == Mode_PickTile)
	{
		pickTile(level::worldToTile(p_pointerState.worldPos));
	}
}


void PaintTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	if (m_mode == Mode_Draw)
	{
		commitUndoCommand();
		m_mode = Mode_None;
	}
}


void PaintTool::onPointerRightPressed(const PointerState& p_pointerState)
{
	// Ctrl+right click on an entity = erase entity (switch to entity paint tool for this)
	if (p_pointerState.ctrl.down)
	{
		getEditor()->switchToToolUntilPointerReleased(ToolID_EntityPaint);
		return;
	}
	
	getEditor()->setCursor(EditCursor_Erase);
	
	if (m_mode != Mode_Erase && m_activeCommand != 0)
	{
		commitUndoCommand();
	}
	
	m_mode = Mode_Erase;
	
	m_haveLastTilePos = false; // first press never has a previous position
	drawFromLastPosTo(level::worldToTile(p_pointerState.worldPos));
}


void PaintTool::onPointerRightDown(const PointerState& p_pointerState)
{
	if (m_mode == Mode_Erase)
	{
		drawFromLastPosTo(level::worldToTile(p_pointerState.worldPos));
	}
}


void PaintTool::onPointerRightReleased(const PointerState& /*p_pointerState*/)
{
	if (m_mode == Mode_Erase)
	{
		commitUndoCommand();
		m_mode = Mode_None;
	}
}


void PaintTool::onPointerLeave()
{
	resetLastDrawPos();
}


std::wstring PaintTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_DRAW");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void PaintTool::resetLastDrawPos()
{
	m_lastTilePos.setValues(-1, -1);
	m_haveLastTilePos = false;
}


void PaintTool::drawFromLastPosTo(const tt::math::Point2& p_newTilePos)
{
	if (m_haveLastTilePos && p_newTilePos == m_lastTilePos)
	{
		// Pointer did not move to a different tile
		return;
	}
	
	TT_ASSERT(m_mode == Mode_Draw || m_mode == Mode_Erase);
	
	/*
	TT_Printf("PaintTool::drawFromLastPosTo: Moved from tile (%d, %d) to (%d, %d) -- last pos valid: %s.\n",
	          m_lastTilePos.x, m_lastTilePos.y, p_newTilePos.x, p_newTilePos.y, m_haveLastTilePos ? "yes" : "no");
	// */
	
	if (m_haveLastTilePos)
	{
		// Draw a line from the previous to the current position
		s32 dx = p_newTilePos.x - m_lastTilePos.x;
		s32 dy = p_newTilePos.y - m_lastTilePos.y;
		
		s32 stepX = 1;
		s32 stepY = 1;
		
		if (dx < 0)
		{
			dx    = -dx;
			stepX = -1;
		}
		if (dy < 0)
		{
			dy    = -dy;
			stepY = -1;
		}
		
		tt::math::Point2 pos(m_lastTilePos);
		
		// Scan first pixel
		drawTile(pos);
		
		if (dx >= dy)  // Scan horizontalish lines
		{
			const s32 two_dxdy = 2 * (dx - dy);
			const s32 two_dy   = 2 * dy;
			s32       d        = dx - two_dy;
			while (pos.x != p_newTilePos.x)
			{
				if (d <= 0)
				{
					d     += two_dxdy;   // choose NE
					pos.y += stepY;
				}
				else
				{
					d -= two_dy;     // choose E
				}
				pos.x += stepX;
				drawTile(pos);
			}
		}
		else           // Scan verticalish lines
		{
			const s32 two_dydx = 2 * (dy - dx);
			const s32 two_dx   = 2 * dx;
			s32       d        = dy - two_dx;
			
			while (pos.y != p_newTilePos.y)
			{
				if (d <= 0)
				{
					d     += two_dydx;   // choose W
					pos.x += stepX;
				}
				else
				{
					d -= two_dx;     // choose NW
				}
				pos.y += stepY;
				drawTile(pos);
			}
		}
	}
	else
	{
		// Cannot use previous position (presumably because this is the first tile to paint):
		// do not draw a line from previous to current, but only paint the current location
		drawTile(p_newTilePos);
	}
	
	m_lastTilePos     = p_newTilePos;
	m_haveLastTilePos = true;
}


void PaintTool::drawTile(const tt::math::Point2& p_tilePos)
{
	const tt::math::PointRect validRect(getEditor()->getEditableRect());
	
	// Only draw the tile if it is within the rectangle where we're allowed to edit
	if (validRect.contains(p_tilePos))
	{
		// Create a new in-progress undo command if there wasn't one yet
		if (m_activeCommand == 0)
		{
			commands::CommandPaintTilesPtr newCmd = createUndoCommand();
			if (newCmd == 0)
			{
				return;
			}
			
			m_activeCommand = newCmd;
		}
		
		TT_NULL_ASSERT(m_activeCommand);
		m_activeCommand->addTile(p_tilePos);
	}
}


void PaintTool::pickTile(const tt::math::Point2& p_tilePos)
{
	Editor* editor = getEditor();
	level::AttributeLayerPtr attribLayer(editor->getLevelData()->getAttributeLayer());
	if (attribLayer->contains(p_tilePos))
	{
		switch (editor->getTileLayerMode())
		{
		case Editor::TileLayerMode_CollisionType:
			editor->setPaintTile(attribLayer->getCollisionType(p_tilePos));
			break;
			
		case Editor::TileLayerMode_ThemeType:
			editor->setPaintTheme(attribLayer->getThemeType(p_tilePos));
			break;
			
		default:
			TT_PANIC("Unsupported tile layer mode: %d. Don't know how to pick a tile.",
			         editor->getTileLayerMode());
			break;
		}
	}
}


commands::CommandPaintTilesPtr PaintTool::createUndoCommand()
{
	switch (getEditor()->getTileLayerMode())
	{
	case Editor::TileLayerMode_CollisionType:
		{
			const level::CollisionType paintTile = (m_mode == Mode_Draw) ?
					getEditor()->getPaintTile() :
					level::CollisionType_Air;
			const bool validType = level::isValidCollisionType(paintTile);
			TT_ASSERT(validType);
			if (validType == false)
			{
				return commands::CommandPaintTilesPtr();
			}
			
			return commands::CommandPaintTiles::create(
					getEditor()->getLevelData()->getAttributeLayer(),
					paintTile);
		}
		
	case Editor::TileLayerMode_ThemeType:
		{
			const level::ThemeType paintTile = (m_mode == Mode_Draw) ?
					getEditor()->getPaintTheme() :
					level::ThemeType_UseLevelDefault;
			const bool validType = level::isValidThemeType(paintTile);
			TT_ASSERT(validType);
			if (validType == false)
			{
				return commands::CommandPaintTilesPtr();
			}
			
			return commands::CommandPaintTiles::create(
					getEditor()->getLevelData()->getAttributeLayer(),
					paintTile);
		}
		
	default:
		TT_PANIC("Unsupported tile layer mode: %d. Cannot create an undo command to paint on this.",
		         getEditor()->getTileLayerMode());
		return commands::CommandPaintTilesPtr();
	}
}


void PaintTool::commitUndoCommand()
{
	if (m_activeCommand != 0 && m_activeCommand->hasTiles())
	{
		getEditor()->pushUndoCommand(m_activeCommand);
	}
	
	m_activeCommand.reset();
	
	resetLastDrawPos();
}

// Namespace end
}
}
}
}
