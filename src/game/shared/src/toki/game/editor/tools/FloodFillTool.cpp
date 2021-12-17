#include <stack>

#include <toki/game/editor/tools/FloodFillTool.h>
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

FloodFillTool::FloodFillTool(Editor* p_editor)
:
Tool(p_editor)
{
}


FloodFillTool::~FloodFillTool()
{
}


void FloodFillTool::onActivate()
{
}


void FloodFillTool::onDeactivate()
{
	getEditor()->restoreDefaultCursor();
}


void FloodFillTool::onPointerHover(const PointerState& /*p_pointerState*/)
{
	getEditor()->setCursor(EditCursor_FloodFill);
}


void FloodFillTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	getEditor()->setCursor(EditCursor_FloodFill);
	switch (getEditor()->getTileLayerMode())
	{
	case Editor::TileLayerMode_CollisionType:
		{
			const level::CollisionType paintTile = getEditor()->getPaintTile();
			const bool                 validType = level::isValidCollisionType(paintTile);
			TT_ASSERT(validType);
			if (validType)
			{
				floodFill(level::worldToTile(p_pointerState.worldPos), paintTile);
			}
		}
		break;
		
	case Editor::TileLayerMode_ThemeType:
		{
			const level::ThemeType paintTile = getEditor()->getPaintTheme();
			const bool             validType = level::isValidThemeType(paintTile);
			TT_ASSERT(validType);
			if (validType)
			{
				floodFill(level::worldToTile(p_pointerState.worldPos), paintTile);
			}
		}
		break;
		
	default:
		TT_PANIC("Unsupported tile layer mode: %d. Do not know how to flood fill.",
		         getEditor()->getTileLayerMode());
		break;
	}
}


void FloodFillTool::onPointerRightPressed(const PointerState& p_pointerState)
{
	getEditor()->setCursor(EditCursor_FloodFill);
	switch (getEditor()->getTileLayerMode())
	{
	case Editor::TileLayerMode_CollisionType:
		floodFill(level::worldToTile(p_pointerState.worldPos), level::CollisionType_Air);
		break;
		
	case Editor::TileLayerMode_ThemeType:
		floodFill(level::worldToTile(p_pointerState.worldPos), level::ThemeType_UseLevelDefault);
		break;
		
	default:
		TT_PANIC("Unsupported tile layer mode: %d. Do not know how to flood fill.",
		         getEditor()->getTileLayerMode());
		break;
	}
}


std::wstring FloodFillTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_FLOODFILL");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void FloodFillTool::floodFill(const tt::math::Point2& p_startTilePos,
                              level::CollisionType    p_paintTile)
{
	const tt::math::PointRect validRect(getEditor()->getEditableRect());
	
	if (validRect.contains(p_startTilePos) == false)
	{
		return;
	}
	
	level::AttributeLayerPtr attribs(getEditor()->getLevelData()->getAttributeLayer());
	
	// Get the existing property at the target position
	const level::CollisionType startTile = attribs->getCollisionType(p_startTilePos);
	
	// Abort early if the target is already the proper color
	if (startTile == p_paintTile)
	{
		return;
	}
	
	// Create an undo command to do the actual painting
	commands::CommandPaintTilesPtr undoCommand(commands::CommandPaintTiles::create(attribs, p_paintTile));
	
	// Flood-fill (node, target-color, replacement-color):
	
	// 1. Set Q to the empty queue.
	typedef std::stack<tt::math::Point2> NodeStack;
	NodeStack nodes;
	
	
	// 2. If the color of node is not equal to target-color, return.
	/*
	if ((attribs[(p_y * p_layerWidth) + p_x] & p_checkMask) != p_targetTile)
	{
		return;
	}
	*/
	
	// 3. Add node to Q.
	nodes.push(p_startTilePos);
	
	// 4. For each element n of Q:
	while (nodes.empty() == false)
	{
		const tt::math::Point2 n(nodes.top());
		nodes.pop();
		
		// 5.  If the color of n is equal to target-color:
		if (attribs->getCollisionType(n) == startTile)
		{
			//  6.   Set w and e equal to n.
			tt::math::Point2 west(n);
			tt::math::Point2 east(n);
			
			//  7.   Move w to the west until the color of the node to the west
			//       of w no longer matches target-color.
			while (west.x > validRect.getLeft() && attribs->getCollisionType(tt::math::Point2(west.x - 1, west.y)) == startTile)
			{
				--west.x;
			}
			
			//  8.   Move e to the east until the color of the node to the east
			//       of e no longer matches target-color.
			while (east.x < validRect.getRight() && attribs->getCollisionType(tt::math::Point2(east.x + 1, east.y)) == startTile)
			{
				++east.x;
			}
			
			// 10.   For each node x between w and e:
			for ( ; west.x <= east.x; ++west.x)
			{
				//  9.   Set the color of nodes between w and e to replacement-color.
				undoCommand->addTile(west);
				
				// 11.    If the color of the node to the north of x is
				//        target-color, add that node to Q.
				const tt::math::Point2 north(west.x, west.y - 1);
				if (validRect.contains(north) && attribs->getCollisionType(north) == startTile)
				{
					nodes.push(north);
				}
				
				//        If the color of the node to the south of x is
				//        target-color, add that node to Q.
				const tt::math::Point2 south(west.x, west.y + 1);
				if (validRect.contains(south) && attribs->getCollisionType(south) == startTile)
				{
					nodes.push(south);
				}
			}
		}
	}
	
	// 12. Continue looping until Q is exhausted.
	// 13. Return.
	
	// Only add the undo command to the stack if any tiles were painted
	if (undoCommand->hasTiles())
	{
		getEditor()->pushUndoCommand(undoCommand);
	}
}


void FloodFillTool::floodFill(const tt::math::Point2& p_startTilePos,
                              level::ThemeType        p_paintTile)
{
	// FIXME: This code is practically identical to the version above
	//        (except this version works on ThemeType, the one above uses CollisionType)
	
	const tt::math::PointRect validRect(getEditor()->getEditableRect());
	
	if (validRect.contains(p_startTilePos) == false)
	{
		return;
	}
	
	level::AttributeLayerPtr attribs(getEditor()->getLevelData()->getAttributeLayer());
	
	// Get the existing property at the target position
	const level::ThemeType startTile = attribs->getThemeType(p_startTilePos);
	
	// Abort early if the target is already the proper color
	if (startTile == p_paintTile)
	{
		return;
	}
	
	// Create an undo command to do the actual painting
	commands::CommandPaintTilesPtr undoCommand(commands::CommandPaintTiles::create(attribs, p_paintTile));
	
	// Flood-fill (node, target-color, replacement-color):
	
	// 1. Set Q to the empty queue.
	typedef std::stack<tt::math::Point2> NodeStack;
	NodeStack nodes;
	
	
	// 2. If the color of node is not equal to target-color, return.
	/*
	if ((attribs[(p_y * p_layerWidth) + p_x] & p_checkMask) != p_targetTile)
	{
		return;
	}
	*/
	
	// 3. Add node to Q.
	nodes.push(p_startTilePos);
	
	// 4. For each element n of Q:
	while (nodes.empty() == false)
	{
		const tt::math::Point2 n(nodes.top());
		nodes.pop();
		
		// 5.  If the color of n is equal to target-color:
		if (attribs->getThemeType(n) == startTile)
		{
			//  6.   Set w and e equal to n.
			tt::math::Point2 west(n);
			tt::math::Point2 east(n);
			
			//  7.   Move w to the west until the color of the node to the west
			//       of w no longer matches target-color.
			while (west.x > validRect.getLeft() && attribs->getThemeType(tt::math::Point2(west.x - 1, west.y)) == startTile)
			{
				--west.x;
			}
			
			//  8.   Move e to the east until the color of the node to the east
			//       of e no longer matches target-color.
			while (east.x < validRect.getRight() && attribs->getThemeType(tt::math::Point2(east.x + 1, east.y)) == startTile)
			{
				++east.x;
			}
			
			// 10.   For each node x between w and e:
			for ( ; west.x <= east.x; ++west.x)
			{
				//  9.   Set the color of nodes between w and e to replacement-color.
				undoCommand->addTile(west);
				
				// 11.    If the color of the node to the north of x is
				//        target-color, add that node to Q.
				const tt::math::Point2 north(west.x, west.y - 1);
				if (validRect.contains(north) && attribs->getThemeType(north) == startTile)
				{
					nodes.push(north);
				}
				
				//        If the color of the node to the south of x is
				//        target-color, add that node to Q.
				const tt::math::Point2 south(west.x, west.y + 1);
				if (validRect.contains(south) && attribs->getThemeType(south) == startTile)
				{
					nodes.push(south);
				}
			}
		}
	}
	
	// 12. Continue looping until Q is exhausted.
	// 13. Return.
	
	// Only add the undo command to the stack if any tiles were painted
	if (undoCommand->hasTiles())
	{
		getEditor()->pushUndoCommand(undoCommand);
	}
}

// Namespace end
}
}
}
}
