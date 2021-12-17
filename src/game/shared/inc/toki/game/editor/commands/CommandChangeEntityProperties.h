#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGEENTITYPROPERTIES_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGEENTITYPROPERTIES_H


#include <map>

#include <tt/undo/UndoCommand.h>

#include <toki/level/entity/EntityInstance.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandChangeEntityProperties;
typedef tt_ptr<CommandChangeEntityProperties>::shared CommandChangeEntityPropertiesPtr;


class CommandChangeEntityProperties : public tt::undo::UndoCommand
{
public:
	static CommandChangeEntityPropertiesPtr create(const level::entity::EntityInstanceSet& p_entitiesToChange);
	virtual ~CommandChangeEntityProperties();
	
	virtual void redo();
	virtual void undo();
	
	// Operates on all entities at once:
	void setPropertyValue(const std::string& p_name, const std::string& p_value);
	void removeProperty(const std::string& p_name);
	
	// Per entity:
	void setPropertyValue(const level::entity::EntityInstancePtr& p_entity,
	                      const std::string& p_name, const std::string& p_value);
	void removeProperty(const level::entity::EntityInstancePtr& p_entity,
	                    const std::string& p_name);
	
	bool hasPropertyChanges() const;
	
private:
	struct Props
	{
		level::entity::EntityInstance::Properties originalProperties;
		level::entity::EntityInstance::Properties replacementProperties;
	};
	
	typedef std::map<level::entity::EntityInstancePtr, Props> Properties;
	
	
	CommandChangeEntityProperties(const level::entity::EntityInstanceSet& p_entitiesToChange);
	
	
	bool       m_addedToStack;
	Properties m_properties;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDCHANGEENTITYPROPERTIES_H)
