#include <tt/math/math.h>
#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandApplyLevelSection.h>
#include <toki/game/editor/tools/BoxSelectTool.h>
#include <toki/game/editor/Editor.h>
#include <toki/game/editor/helpers.h>
#include <toki/level/helpers.h>
#include <toki/level/LevelData.h>
#include <toki/level/LevelSection.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

//--------------------------------------------------------------------------------------------------
// Public member functions

BoxSelectTool::BoxSelectTool(Editor* p_editor)
:
Tool(p_editor),
m_mode(Mode_CreateSelection),
m_startedSelection(false),
m_startWorldPos(-1.0f, -1.0f),
m_startTilePos(-1, -1)
{
}


BoxSelectTool::~BoxSelectTool()
{
}


void BoxSelectTool::onActivate()
{
	//TT_Printf("BoxSelectTool::onActivate: Activate!\n");
	m_startedSelection = false;
}


void BoxSelectTool::onDeactivate()
{
	//TT_Printf("BoxSelectTool::onDeactivate: Deactivate!\n");
	getEditor()->restoreDefaultCursor();
}


void BoxSelectTool::onPointerHover(const PointerState& p_pointerState)
{
	const tt::math::Point2 tilePos(level::worldToTile(p_pointerState.worldPos));
	Editor* editor = getEditor();
	if (editor->getSelectionRect().contains(tilePos))
	{
		if (editor->hasFloatingSection())
		{
			editor->setCursor((p_pointerState.ctrl.down && p_pointerState.alt.down) ?
				EditCursor_SelectionContentClone : EditCursor_SelectionContentMove);
		}
		else if (p_pointerState.ctrl.down)
		{
			editor->setCursor(p_pointerState.alt.down ?
				EditCursor_SelectionContentClone : EditCursor_SelectionContentCut);
		}
		else if (p_pointerState.alt.down)
		{
			editor->setCursor(EditCursor_SelectionContentCut);
		}
		else
		{
			editor->setCursor(EditCursor_SelectionRectMove);
		}
	}
	else
	{
		editor->setCursor(p_pointerState.ctrl.down ? EditCursor_NormalArrow : EditCursor_SelectionRectCreate);
	}
}


void BoxSelectTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	//TT_Printf("BoxSelectTool::onPointerLeftPressed: Pressed.\n");
	
	const tt::math::Point2 tilePos(level::worldToTile(p_pointerState.worldPos));
	
	m_startedSelection = false;
	m_startWorldPos    = p_pointerState.worldPos;
	
	Editor* editor = getEditor();
	
	if (editor->getSelectionRect().contains(tilePos))
	{
		if (editor->hasFloatingSection())
		{
			// Continue moving the existing floating section
			m_mode = Mode_MoveContents;
		}
		else if (p_pointerState.ctrl.down || p_pointerState.alt.down)
		{
			m_mode = Mode_MoveContents;
		}
		else
		{
			m_mode = Mode_MoveSelection;
		}
		
		// When moving the selection, "start tile pos" is the offset within the rectangle
		m_startTilePos  = editor->getSelectionRect().getPosition() - tilePos;
	}
	else
	{
		m_mode         = Mode_CreateSelection;
		m_startTilePos = tilePos;
		editor->clearSelectionRect(p_pointerState.ctrl.down == false);
		
		// Also modify the entity selection at this point
		level::LevelDataPtr levelData(editor->getLevelData());
		level::entity::EntityInstances entitiesUnderPointer(
			editor->findEntitiesAtWorldPos(p_pointerState.worldPos));
		for (level::entity::EntityInstances::iterator it = entitiesUnderPointer.begin();
		     it != entitiesUnderPointer.end(); ++it)
		{
			if (levelData->isEntitySelected(*it))
			{
				levelData->removeEntityFromSelection(*it);
			}
			else
			{
				levelData->addEntityToSelection(*it);
			}
		}
	}
}


void BoxSelectTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	//TT_Printf("BoxSelectTool::onPointerLeftDown: Down.\n");
	
	switch (m_mode)
	{
	case Mode_CreateSelection: handleCreateSelection(p_pointerState); break;
	case Mode_MoveSelection:   handleMoveSelection  (p_pointerState); break;
	case Mode_MoveContents:    handleMoveContents   (p_pointerState); break;
		
	default:
		TT_PANIC("Unsupported selection mode: %d", m_mode);
		break;
	}
}


void BoxSelectTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	//TT_Printf("BoxSelectTool::onPointerLeftReleased: Released.\n");
}


void BoxSelectTool::onPointerEnter()
{
	//TT_Printf("BoxSelectTool::onPointerEnter: Pointer is on the drawing canvas.\n");
}


void BoxSelectTool::onPointerLeave()
{
	//TT_Printf("BoxSelectTool::onPointerLeave: Pointer left the drawing canvas.\n");
}


std::wstring BoxSelectTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_BOXSELECT");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void BoxSelectTool::handleCreateSelection(const PointerState& p_pointerState)
{
	if (m_startedSelection == false &&
	    tt::math::distance(m_startWorldPos, p_pointerState.worldPos) < (level::tileToWorld(1) * 0.5f))
	{
		// Pointer did not move enough yet to start a selection
		return;
	}
	
	m_startedSelection = true;
	
	tt::math::Point2 minPos(m_startTilePos);
	tt::math::Point2 maxPos(level::worldToTile(p_pointerState.worldPos));
	
	
	if (maxPos.x < minPos.x) std::swap(maxPos.x, minPos.x);
	if (maxPos.y < minPos.y) std::swap(maxPos.y, minPos.y);
	
	getEditor()->setSelectionRect(tt::math::PointRect(minPos, maxPos));
}


void BoxSelectTool::handleMoveSelection(const PointerState& p_pointerState)
{
	tt::math::PointRect selection(getEditor()->getSelectionRect());
	selection.setPosition(m_startTilePos + level::worldToTile(p_pointerState.worldPos));
	getEditor()->setSelectionRect(selection);
}


void BoxSelectTool::handleMoveContents(const PointerState& p_pointerState)
{
	if (m_startedSelection == false)
	{
		if (tt::math::distance(m_startWorldPos, p_pointerState.worldPos) < (level::tileToWorld(1) * 0.5f))
		{
			// Pointer did not move enough yet to start the movement
			return;
		}
		
		// Perform the initial step for the contents copy or move
		Editor* editor = getEditor();
		if (editor->hasFloatingSection())
		{
			// Operating on an existing floating section
			if (p_pointerState.ctrl.down && p_pointerState.alt.down)
			{
				// Create a new copy
				editor->cloneFloatingSection();
			}
		}
		else if (p_pointerState.ctrl.down || p_pointerState.alt.down)
		{
			if (p_pointerState.ctrl.down && p_pointerState.alt.down)
			{
				editor->createFloatingSectionFromSelection(level::LevelSection::Type_All);
			}
			else
			{
				level::LevelSection::Type sectionType = p_pointerState.alt.down ?
					level::LevelSection::Type_Entities : level::LevelSection::Type_All;
				
				editor->createFloatingSectionFromSelection(sectionType);
				
				// Moving the selection contents: erase the tiles at the selection position
				editor->pushUndoCommand(commands::CommandApplyLevelSection::create(
					getEditor()->getLevelData(),
					level::LevelSection::createEmpty(editor->getSelectionRect(), sectionType)));
			}
		}
		
		m_startedSelection = true;
	}
	
	tt::math::PointRect selection(getEditor()->getSelectionRect());
	selection.setPosition(m_startTilePos + level::worldToTile(p_pointerState.worldPos));
	getEditor()->setSelectionRect(selection);
	getEditor()->setFloatingSectionPosition(selection.getPosition());
}

// Namespace end
}
}
}
}
