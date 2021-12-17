#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandAddRemoveEntities.h>
#include <toki/game/editor/Editor.h>
#include <toki/level/LevelData.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandAddRemoveEntitiesPtr CommandAddRemoveEntities::createForAdd(
		Editor*                               p_editor,
		const level::entity::EntityInstances& p_entities)
{
	TT_NULL_ASSERT(p_editor);
	TT_ASSERTMSG(p_entities.empty() == false, "No entities specified to add.");
	if (p_editor == 0 || p_entities.empty())
	{
		return CommandAddRemoveEntitiesPtr();
	}
	
	return CommandAddRemoveEntitiesPtr(new CommandAddRemoveEntities(p_editor, p_entities, Mode_Add));
}


CommandAddRemoveEntitiesPtr CommandAddRemoveEntities::createForRemove(
		Editor*                               p_editor,
		const level::entity::EntityInstances& p_entities)
{
	TT_NULL_ASSERT(p_editor);
	TT_ASSERTMSG(p_entities.empty() == false, "No entities specified to remove.");
	if (p_editor == 0 || p_entities.empty())
	{
		return CommandAddRemoveEntitiesPtr();
	}
	
	return CommandAddRemoveEntitiesPtr(new CommandAddRemoveEntities(p_editor, p_entities, Mode_Remove));
}


CommandAddRemoveEntities::~CommandAddRemoveEntities()
{
}


void CommandAddRemoveEntities::redo()
{
	// Do not perform the add/remove operation the first time, since this was already handled at creation time
	// (so that there is instant visual feedback for edit operations)
	if (m_addedToStack)
	{
		if (m_mode == Mode_Add)
		{
			addEntitiesToLevelData(m_entities);
		}
		else
		{
			removeEntitiesFromLevelData(m_entities);
		}
	}
	
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
}


void CommandAddRemoveEntities::undo()
{
	if (m_mode == Mode_Add)
	{
		removeEntitiesFromLevelData(m_entities);
	}
	else
	{
		addEntitiesToLevelData(m_entities);
	}
}


void CommandAddRemoveEntities::addEntity(const level::entity::EntityInstancePtr& p_entity)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call addEntity after this command has already been added to an undo stack.");
		return;
	}
	
	addEntities(level::entity::EntityInstances(1, p_entity));
}


void CommandAddRemoveEntities::addEntities(const level::entity::EntityInstances& p_entities)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call addEntities after this command has already been added to an undo stack.");
		return;
	}
	
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::entity::EntityInstances::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		// Only add entities that weren't known to the command yet
		if (std::find(m_entities.begin(), m_entities.end(), *it) == m_entities.end())
		{
			if (m_mode == Mode_Add)
			{
				levelData->addEntity(*it);
			}
			else
			{
				levelData->removeEntity(*it);
			}
			
			m_entities.push_back(*it);
		}
	}
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandAddRemoveEntities::CommandAddRemoveEntities(Editor*                               p_editor,
                                                   const level::entity::EntityInstances& p_entities,
                                                   Mode                                  p_mode)
:
tt::undo::UndoCommand(p_mode == Mode_Add ? L"Add Entities" : L"Remove Entities"),
m_addedToStack(false),
m_editor(p_editor),
m_entities(p_entities),
m_mode(p_mode)
{
	// Immediately apply the desired operation on the entities, so that the GUI has instant feedback
	if (m_mode == Mode_Add)
	{
		addEntitiesToLevelData(m_entities);
	}
	else
	{
		removeEntitiesFromLevelData(m_entities);
	}
}


void CommandAddRemoveEntities::addEntitiesToLevelData(const level::entity::EntityInstances& p_entities)
{
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::entity::EntityInstances::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		levelData->addEntity(*it);
	}
}


void CommandAddRemoveEntities::removeEntitiesFromLevelData(const level::entity::EntityInstances& p_entities)
{
	const level::LevelDataPtr& levelData(m_editor->getLevelData());
	for (level::entity::EntityInstances::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		levelData->removeEntity(*it);
	}
}

// Namespace end
}
}
}
}
