#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDMOVEENTITIES_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDMOVEENTITIES_H


#include <tt/undo/UndoCommand.h>

#include <toki/level/entity/EntityInstance.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandMoveEntities;
typedef tt_ptr<CommandMoveEntities>::shared CommandMoveEntitiesPtr;


class CommandMoveEntities : public tt::undo::UndoCommand
{
public:
	static CommandMoveEntitiesPtr create(const level::entity::EntityInstancePtr& p_entityToMove);
	static CommandMoveEntitiesPtr create(const level::entity::EntityInstances&   p_entitiesToMove);
	virtual ~CommandMoveEntities();
	
	virtual void redo();
	virtual void undo();
	
	/*! \brief Adds one or more entities for the command to operate on (either add or remove,
	           depending on whether the command was created to add or remove entities). */
	//void addEntity(const EditorEntityPtr& p_entity);
	//void addEntities(const EditorEntities& p_entities);
	
	void setEntityPosition(const level::entity::EntityInstancePtr& p_entity, const tt::math::Vector2& p_pos);
	
	bool isAnyPositionChanged() const;
	
	inline bool hasEntities() const { return m_entities.empty() == false; }
	
private:
	struct EntityPos
	{
		tt::math::Vector2 oldPos;
		tt::math::Vector2 newPos;
	};
	typedef std::map<level::entity::EntityInstancePtr, EntityPos> EntityPositions;
	
	
	CommandMoveEntities(const level::entity::EntityInstances& p_entities);
	
	
	bool            m_addedToStack;
	EntityPositions m_entities;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDMOVEENTITIES_H)
