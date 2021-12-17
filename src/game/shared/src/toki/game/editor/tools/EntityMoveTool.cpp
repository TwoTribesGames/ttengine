#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <toki/game/editor/tools/EntityMoveTool.h>
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

EntityMoveTool::EntityMoveTool(Editor* p_editor)
:
Tool(p_editor),
m_entitiesToMove(),
m_activeCommand()
{
}


EntityMoveTool::~EntityMoveTool()
{
}


void EntityMoveTool::onActivate()
{
	TT_ASSERTMSG(m_activeCommand == 0, "Active entity move command wasn't cleaned up properly.");
}


void EntityMoveTool::onDeactivate()
{
	commitUndoCommand();
	getEditor()->restoreDefaultCursor();
}


void EntityMoveTool::onPointerHover(const PointerState& p_pointerState)
{
	getEditor()->setCursor(getEditor()->hasEntityAtWorldPos(p_pointerState.worldPos) ?
		EditCursor_GenericMove : EditCursor_NormalArrow);
}


void EntityMoveTool::onPointerLeftPressed(const PointerState& p_pointerState)
{
	TT_ASSERTMSG(m_activeCommand == 0, "Active entity move command wasn't cleaned up properly.");
	TT_ASSERT(m_entitiesToMove.empty());
	
	// Desired entity move behavior:
	// - If no selected entities, continue as before
	// - If selected entities AND clicked on one of the entities, move all selected entities
	
	level::LevelDataPtr levelData(getEditor()->getLevelData());
	
	level::entity::EntityInstances existingEntities(getEditor()->findEntitiesAtWorldPos(p_pointerState.worldPos));
	if (existingEntities.empty())
	{
		if (p_pointerState.ctrl.down == false)
		{
			levelData->deselectAllEntities();
		}
		return;
	}
	
	level::entity::EntityInstancePtr entityUnderPointer(existingEntities.back());
	
	if (p_pointerState.ctrl.down)
	{
		if (levelData->isEntitySelected(entityUnderPointer))
		{
			levelData->removeEntityFromSelection(entityUnderPointer);
		}
		else
		{
			levelData->addEntityToSelection(entityUnderPointer);
		}
	}
	/*
	else
	{
		levelData->deselectAllEntities();
		levelData->addEntityToSelection(entityUnderPointer);
	}
	*/
	
	// Check if one of the entities under the pointer is selected
	// (if so, move *all* of the selected entities)
	bool moveSelectedEntities = false;
	for (level::entity::EntityInstances::const_iterator it = existingEntities.begin();
	     it != existingEntities.end(); ++it)
	{
		if (levelData->isEntitySelected(*it))
		{
			moveSelectedEntities = true;
			break;
		}
	}
	
	getEditor()->setCursor(EditCursor_GenericMove);
	
	level::entity::EntityInstances moveEntities;
	
	if (moveSelectedEntities)
	{
		const level::entity::EntityInstanceSet& selectedEntities(levelData->getSelectedEntities());
		moveEntities.reserve(selectedEntities.size());
		for (level::entity::EntityInstanceSet::const_iterator it = selectedEntities.begin();
		     it != selectedEntities.end(); ++it)
		{
			moveEntities.push_back(*it);
		}
	}
	else
	{
		// Pick only a single entity to move
		moveEntities.push_back(entityUnderPointer);
		if (p_pointerState.ctrl.down == false)
		{
			levelData->deselectAllEntities();
			levelData->addEntityToSelection(entityUnderPointer);
		}
	}
	
	// Store the offsets within each entity where the move started
	for (level::entity::EntityInstances::const_iterator it = moveEntities.begin();
	     it != moveEntities.end(); ++it)
	{
		m_entitiesToMove[*it] = (*it)->getPosition() - p_pointerState.worldPos;
	}
	
	m_activeCommand = commands::CommandMoveEntities::create(moveEntities);
	
	getEditor()->clearSelectionRect(false);
}


void EntityMoveTool::onPointerLeftDown(const PointerState& p_pointerState)
{
	if (m_entitiesToMove.empty() || m_activeCommand == 0)
	{
		return;
	}
	
	const tt::math::VectorRect editWorldRect(level::tileToWorld(getEditor()->getLevelData()->getLevelRect()));
	
	for (MoveEntities::iterator it = m_entitiesToMove.begin(); it != m_entitiesToMove.end(); ++it)
	{
		const level::entity::EntityInstancePtr& entity((*it).first);
		const tt::math::Vector2&                offsetWithinEntity((*it).second);
		
		
		tt::math::Vector2 newPos(p_pointerState.worldPos + offsetWithinEntity);
		if (p_pointerState.alt.down == false)
		{
			// Snap to grid position
			newPos = level::snapToTilePos(newPos, tt::math::Vector2(-getEditor()->getEntitySize(entity).x * 0.5f, 0.0f));
		}
		
		// FIXME: This check should probably be more sophisticated (take size of entities into account)
		if (editWorldRect.contains(newPos))
		{
			m_activeCommand->setEntityPosition(entity, newPos);
		}
	}
}


void EntityMoveTool::onPointerLeftReleased(const PointerState& /*p_pointerState*/)
{
	commitUndoCommand();
}


std::wstring EntityMoveTool::getHelpText() const
{
	return translateString("HELPTEXT_TOOL_ENTITYMOVE");
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void EntityMoveTool::commitUndoCommand()
{
	if (m_activeCommand != 0 && m_activeCommand->isAnyPositionChanged())
	{
		getEditor()->pushUndoCommand(m_activeCommand);
	}
	
	m_activeCommand.reset();
	m_entitiesToMove.clear();
}

// Namespace end
}
}
}
}
