#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CHECKPOINTMGRWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CHECKPOINTMGRWRAPPER_H


#include <tt/script/helpers.h>
#include <tt/str/str.h>

#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'CheckPointMgr' in Squirrel. */
class CheckPointMgrWrapper
{
public:
	/*! \brief Store checkpoint
	    \param p_id Unique ID that is bound to this checkpoint. */
	static void store(const std::string& p_id);
	
	/*! \brief Restore checkpoint
	    \param p_id Unique ID that is bound to the checkpoint that needs to be restored. */
	static void restore(const std::string& p_id);
	
	/*! \brief Restore checkpoint for a specific progresstype
	    \param p_id Unique ID that is bound to the checkpoint that needs to be restored.
	    \param p_progressType the specific progress type to check. */
	static void restoreEx(const std::string& p_id, ProgressType p_progressType);
	
	/*! \brief Restore checkpoint and clears that checkpoint immediately afterwards
	    \param p_id Unique ID that is bound to the checkpoint that needs to be restored and cleared. */
	static void restoreAndClear(const std::string& p_id);
	
	/*! \brief Restore checkpoint and clears that game checkpoint immediately afterwards for a specific progresstype
	    \param p_id Unique ID that is bound to the checkpoint that needs to be restored and cleared.
	    \param p_progressType the specific progress type to check. */
	static void restoreAndClearEx(const std::string& p_id, ProgressType p_progressType);
	
	/*! \brief Retrieve a list of all current checkpoint IDs */
	static tt::str::Strings getAllIDs();
	
	/*! \brief Retrieve a list of all current checkpoint IDs for a specific progress type
	    \param p_progressType the specific progress type to check. */
	static tt::str::Strings getAllIDsEx(ProgressType p_progressType);
	
	/*! \brief Check whether the id has already been stored
	    \param p_id The ID to check against */
	static bool hasID(const std::string& p_id);
	
	/*! \brief Check whether the id has already been stored for a specific progresstype
	    \param p_id The ID to check against
	    \param p_progressType the specific progress type to check. */
	static bool hasIDEx(const std::string& p_id, ProgressType p_progressType);
	
	/*! \brief Renamed an existing progress ID to a different one */
	static bool renameID(const std::string& p_oldID, const std::string& p_newID);
	
	/*! \brief Clear checkpoint
	    \param p_id Unique ID that is bound to the checkpoint that needs to be cleared. */
	static void clearID(const std::string& p_id);
	
	/*! \brief Clear all progress (checkpoints) */
	static void clearAll();
	
	/*! \brief Clear all progress (checkpoints) for a specific progresstype
	    \param p_progressType the specific progress type to check. */
	static void clearAllEx(ProgressType p_progressType);
	
	/*! \brief Returns whether there is still enough storage space to store another checkpoint */
	static bool canStore();
	
	/*! \brief Returns the number of checkpoints for a specified progress type. */
	static u32 getNumberOfCheckpoints(ProgressType p_progressType);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_CHECKPOINTMGRWRAPPER_H)
