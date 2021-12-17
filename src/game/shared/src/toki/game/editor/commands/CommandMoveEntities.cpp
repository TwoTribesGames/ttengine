#include <tt/math/math.h>
#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandMoveEntities.h>
#include <toki/game/editor/Editor.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandMoveEntitiesPtr CommandMoveEntities::create(const level::entity::EntityInstancePtr& p_entity)
{
	return create(level::entity::EntityInstances(1, p_entity));
}


CommandMoveEntitiesPtr CommandMoveEntities::create(const level::entity::EntityInstances& p_entities)
{
	TT_ASSERTMSG(p_entities.empty() == false, "No entities specified to move.");
	if (p_entities.empty())
	{
		return CommandMoveEntitiesPtr();
	}
	
	return CommandMoveEntitiesPtr(new CommandMoveEntities(p_entities));
}


CommandMoveEntities::~CommandMoveEntities()
{
}


void CommandMoveEntities::redo()
{
	// UndoStack::push calls redo(): this is a good indicator that this command has been added to an undo stack
	m_addedToStack = true;
	
	for (EntityPositions::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		(*it).first->setPosition((*it).second.newPos);
	}
}


void CommandMoveEntities::undo()
{
	for (EntityPositions::iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		(*it).first->setPosition((*it).second.oldPos);
	}
}


/*
void CommandMoveEntities::addEntity(const EditorEntityPtr& p_entity)
{
	addEntities(EditorEntities(1, p_entity));
}


void CommandMoveEntities::addEntities(const EditorEntities& p_entities)
{
	for (EditorEntities::const_iterator it = p_entities.begin(); it != p_entities.end(); ++it)
	{
		// Only add entities that weren't known to the command yet
		if (std::find(m_entities.begin(), m_entities.end(), *it) == m_entities.end())
		{
			if (m_mode == Mode_Add)
			{
				m_editor->addEntity(*it);
			}
			else
			{
				m_editor->removeEntity(*it);
			}
			
			m_entities.push_back(*it);
		}
	}
}
// */


void CommandMoveEntities::setEntityPosition(const level::entity::EntityInstancePtr& p_entity,
                                            const tt::math::Vector2&                p_pos)
{
	if (m_addedToStack)
	{
		TT_PANIC("Should not call setEntityPosition after this command has already been added to an undo stack.");
		return;
	}
	
	TT_NULL_ASSERT(p_entity);
	EntityPositions::iterator it = m_entities.find(p_entity);
	if (it == m_entities.end())
	{
		TT_PANIC("Entity 0x%08X is not known to this undo command: cannot change its position.",
		         p_entity.get());
		return;
	}
	
	(*it).second.newPos = p_pos;
	p_entity->setPosition(p_pos);
}


bool CommandMoveEntities::isAnyPositionChanged() const
{
	for (EntityPositions::const_iterator it = m_entities.begin(); it != m_entities.end(); ++it)
	{
		if (tt::math::realEqual((*it).second.oldPos.x, (*it).second.newPos.x) == false ||
		    tt::math::realEqual((*it).second.oldPos.y, (*it).second.newPos.y) == false)
		{
			return true;
		}
	}
	
	return false;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandMoveEntities::CommandMoveEntities(const level::entity::EntityInstances& p_entities)
:
tt::undo::UndoCommand(L"Move Entities"),
m_addedToStack(false),
m_entities()
{
	for (level::entity::EntityInstances::const_iterator it = p_entities.begin();
	     it != p_entities.end(); ++it)
	{
		const level::entity::EntityInstancePtr& entity(*it);
		
		TT_NULL_ASSERT(entity);
		TT_ASSERTMSG(m_entities.find(entity) == m_entities.end(),
		             "Entity 0x%08X has been specified more than once.", entity.get());
		
		EntityPos pos;
		pos.oldPos = entity->getPosition();
		pos.newPos = pos.oldPos;
		m_entities[entity] = pos;
	}
}

// Namespace end
}
}
}
}
