#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVEENTITIES_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVEENTITIES_H


#include <tt/undo/UndoCommand.h>

#include <toki/game/editor/fwd.h>
#include <toki/level/entity/EntityInstance.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandAddRemoveEntities;
typedef tt_ptr<CommandAddRemoveEntities>::shared CommandAddRemoveEntitiesPtr;


class CommandAddRemoveEntities : public tt::undo::UndoCommand
{
public:
	static CommandAddRemoveEntitiesPtr createForAdd   (Editor* p_editor, const level::entity::EntityInstances& p_entitiesToAdd);
	static CommandAddRemoveEntitiesPtr createForRemove(Editor* p_editor, const level::entity::EntityInstances& p_entitiesToRemove);
	virtual ~CommandAddRemoveEntities();
	
	virtual void redo();
	virtual void undo();
	
	/*! \brief Adds one or more entities for the command to operate on (either add or remove,
	           depending on whether the command was created to add or remove entities). */
	void addEntity(const level::entity::EntityInstancePtr& p_entity);
	void addEntities(const level::entity::EntityInstances& p_entities);
	
	inline bool hasEntities() const { return m_entities.empty() == false; }
	
private:
	enum Mode
	{
		Mode_Add,
		Mode_Remove
	};
	
	
	CommandAddRemoveEntities(Editor* p_editor, const level::entity::EntityInstances& p_entities, Mode p_mode);
	void addEntitiesToLevelData     (const level::entity::EntityInstances& p_entities);
	void removeEntitiesFromLevelData(const level::entity::EntityInstances& p_entities);
	
	
	bool                           m_addedToStack;
	Editor*                        m_editor;
	level::entity::EntityInstances m_entities;
	const Mode                     m_mode;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDADDREMOVEENTITIES_H)
