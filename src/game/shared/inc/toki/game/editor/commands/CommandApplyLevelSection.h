#if !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDAPPLYLEVELSECTION_H)
#define INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDAPPLYLEVELSECTION_H


#include <tt/undo/UndoCommand.h>

#include <toki/level/fwd.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

class CommandApplyLevelSection;
typedef tt_ptr<CommandApplyLevelSection>::shared CommandApplyLevelSectionPtr;


class CommandApplyLevelSection : public tt::undo::UndoCommand
{
public:
	static CommandApplyLevelSectionPtr create(const level::LevelDataPtr&    p_targetLevel,
	                                          const level::LevelSectionPtr& p_sectionToApply);
	virtual ~CommandApplyLevelSection();
	
	virtual void redo();
	virtual void undo();
	
private:
	CommandApplyLevelSection(const level::LevelDataPtr&    p_targetLevel,
	                         const level::LevelSectionPtr& p_sectionToApply);
	
	
	level::LevelDataPtr    m_targetLevel;
	level::LevelSectionPtr m_originalLevelSection;
	level::LevelSectionPtr m_newLevelSection;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_COMMANDS_COMMANDAPPLYLEVELSECTION_H)
