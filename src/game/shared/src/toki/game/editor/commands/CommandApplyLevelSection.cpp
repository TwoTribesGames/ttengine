#include <tt/platform/tt_error.h>

#include <toki/game/editor/commands/CommandApplyLevelSection.h>
#include <toki/level/LevelData.h>
#include <toki/level/LevelSection.h>


namespace toki {
namespace game {
namespace editor {
namespace commands {

//--------------------------------------------------------------------------------------------------
// Public member functions

CommandApplyLevelSectionPtr CommandApplyLevelSection::create(
		const level::LevelDataPtr&    p_targetLevel,
		const level::LevelSectionPtr& p_sectionToApply)
{
	TT_NULL_ASSERT(p_targetLevel);
	TT_NULL_ASSERT(p_sectionToApply);
	if (p_targetLevel == 0 || p_sectionToApply == 0)
	{
		return CommandApplyLevelSectionPtr();
	}
	
	return CommandApplyLevelSectionPtr(new CommandApplyLevelSection(p_targetLevel, p_sectionToApply));
}


CommandApplyLevelSection::~CommandApplyLevelSection()
{
}


void CommandApplyLevelSection::redo()
{
	m_newLevelSection->applyToLevel(m_targetLevel);
}


void CommandApplyLevelSection::undo()
{
	m_originalLevelSection->applyToLevel(m_targetLevel);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

CommandApplyLevelSection::CommandApplyLevelSection(const level::LevelDataPtr&    p_targetLevel,
                                                   const level::LevelSectionPtr& p_sectionToApply)
:
tt::undo::UndoCommand(L"Apply Level Section"),
m_targetLevel(p_targetLevel),
m_originalLevelSection(level::LevelSection::createFromLevelRect(p_targetLevel, p_sectionToApply->getRect())),
m_newLevelSection(p_sectionToApply)
{
	TT_NULL_ASSERT(m_originalLevelSection);
	//m_originalLevelSection->debugPrint();
}

// Namespace end
}
}
}
}
