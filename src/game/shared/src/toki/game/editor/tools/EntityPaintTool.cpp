#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandAddRemoveEntities.h>
#include <toki/game/editor/tools/EntityPaintTool.h>
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

EntityPaintTool::EntityPaintTool(Editor* p_editor)
:
Tool(p_editor),
m_placingEntity(),
m_activeCommand(),
m_inDrawMode(true)
{
}


EntityPaintTool::~EntityPaintTool()
{
}


void EntityPaintTool::onActivate()
{
	TT_ASSERTMSG(m_activeCommand == 0, "Active entity paint command wasn't cleaned up properly.");
}


void EntityPaintTool::onDeactivate()
{
	commitUndoCommand();
	getEditor()->restoreDefaultCursor();
}


bool EntityPaintTool::canDeactivate() const
{
	return m_activeCommand == 0;
}


void EntityPaintTool::onPointerHover(const PointerState& p_pointerState)
{
	if (p_pointerState.ctrl.down)
	{
		getEditor()->setCursor(getEditor()->hasEntityAtWorldPos(p_pointerState.worldPos) ?
			EditCursor_GenericMove : EditCursor_NormalArrow);
	}
	else
	{
		getEditor()->setCursor(getEditor()->getPaintEntityType().empty() ?
			EditCursor_NotAllowed : EditCursor_DrawEntity);
	}
}


void EntityPaintTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	if (p_pointerState.ctrl.down)
	{
		if (getEditor()->hasEntityAtWorldPos(p_pointerState.worldPos))
		{
			getEditor()->getLevelData()->deselectAllEntities();
			getEditor()->switchToToolUntilPointerReleased(ToolID_EntityMove);
		}
		return;
	}
	
	
	if (getEditor()->getPaintEntityType().empty())
	{
		return;
	}
	
	getEditor()->setCursor(EditCursor_DrawEntity);
	
	if (m_inDrawMode == false && m_activeCommand != 0)
	{
		commitUndoCommand();
	}
	
	// Only allow editing within level rect
	if (level::tileToWorld(getEditor()->getLevelData()->getLevelRect()).contains(p_pointerState.worldPos) == false)
	{
		return;
	}
	
	// Clicking places a new entity and moves it around for as long as the pointer remains pressed
	m_placingEntity = getEditor()->createEntity(getEditor()->getPaintEntityType());
	if (m_placingEntity == 0)
	{
		return;
	}
	
	const tt::math::Vector2 entitySize(getEditor()->getEntitySize(m_placingEntity));
	tt::math::Vector2 entityPos(p_pointerState.worldPos);
	if (p_pointerState.alt.down == false)
	{
		// Snap to grid position
		entityPos = level::snapToTilePos(entityPos,
			tt::math::Vector2(-entitySize.x * 0.5f, 0.0f));
	}
	
	m_placingEntity->setPosition(entityPos);
	m_activeCommand = commands::CommandAddRemoveEntities::createForAdd(
		getEditor(), level::entity::EntityInstances(1, m_placingEntity));
	
	// Select only the newly placed entity (after adding it to the level,
	// since selected entities are required to be part of the level)
	level::entity::EntityInstanceSet selection;
	selection.insert(m_placingEntity);
	getEditor()->getLevelData()->setSelectedEntities(selection);
	
	m_inDrawMode = true;
}


void EntityPaintTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	if (m_inDrawMode && m_placingEntity != 0)
	{
		// Only allow editing within edit rect
		// FIXME: This check should probably be more sophisticated (take size of entities into account)
		if (level::tileToWorld(getEditor()->getLevelData()->getLevelRect()).contains(p_pointerState.worldPos))
		{
			const tt::math::Vector2 entitySize(getEditor()->getEntitySize(m_placingEntity));
			tt::math::Vector2 entityPos(p_pointerState.worldPos);
			if (p_pointerState.alt.down == false)
			{
				// Snap to grid position
				entityPos = level::snapToTilePos(entityPos,
					tt::math::Vector2(-entitySize.x * 0.5f, 0.0f));
			}
			
			m_placingEntity->setPosition(entityPos);
		}
	}
}


void EntityPaintTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	if (m_inDrawMode)
	{
		// If moving entity until release: commit command here. Otherwise, do nothing
		commitUndoCommand();
	}
	
	m_placingEntity.reset();
}


void EntityPaintTool::onPointerRightPressed(const PointerState& p_pointerState)
{
	if (m_inDrawMode && m_activeCommand != 0)
	{
		commitUndoCommand();
	}
	
	Editor* editor = getEditor();
	
	editor->setCursor(EditCursor_Erase);
	
	/*
	// Only allow editing within level rect
	if (level::tileToWorld(editor->getEditableRect()).contains(p_pointerState.worldPos) == false)
	{
		return;
	}
	*/
	
	level::entity::EntityInstances entitiesToDelete(editor->findEntitiesAtWorldPos(p_pointerState.worldPos));
	if (entitiesToDelete.empty() == false)
	{
		m_activeCommand = commands::CommandAddRemoveEntities::createForRemove(editor, entitiesToDelete);
	}
	
	m_inDrawMode = false;
}


void EntityPaintTool::onPointerRightDown(const PointerState& p_pointerState)
{
	if (m_inDrawMode == false)
	{
		Editor* editor = getEditor();
		
		/*
		// Only allow editing within edit rect
		// FIXME: This check should probably be more sophisticated (take size of entities into account)
		if (level::tileToWorld(editor->getEditableRect()).contains(p_pointerState.worldPos) == false)
		{
			return;
		}
		*/
		
		level::entity::EntityInstances entitiesToDelete(editor->findEntitiesAtWorldPos(p_pointerState.worldPos));
		if (entitiesToDelete.empty() == false)
		{
			if (m_activeCommand == 0)
			{
				m_activeCommand = commands::CommandAddRemoveEntities::createForRemove(
					editor, entitiesToDelete);
			}
			else
			{
				m_activeCommand->addEntities(entitiesToDelete);
			}
		}
	}
}


void EntityPaintTool::onPointerRightReleased(const PointerState& /*p_pointerState*/)
{
	if (m_inDrawMode == false)
	{
		commitUndoCommand();
	}
}


std::wstring EntityPaintTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_ENTITYDRAW");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityPaintTool::commitUndoCommand()
{
	if (m_activeCommand != 0 && m_activeCommand->hasEntities())
	{
		getEditor()->pushUndoCommand(m_activeCommand);
	}
	
	m_placingEntity.reset();
	m_activeCommand.reset();
}

// Namespace end
}
}
}
}
