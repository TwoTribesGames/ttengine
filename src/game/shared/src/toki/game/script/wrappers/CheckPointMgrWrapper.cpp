#include <toki/game/script/wrappers/CheckPointMgrWrapper.h>
#include <toki/game/script/sqbind_bindings.h>
#include <toki/game/CheckPointMgr.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


void CheckPointMgrWrapper::store(const std::string& p_id)
{
	AppGlobal::getGame()->serializeGameState(p_id);
}


void CheckPointMgrWrapper::restore(const std::string& p_id)
{
	AppGlobal::getGame()->unserializeGameState(p_id, false);
}


void CheckPointMgrWrapper::restoreEx(const std::string& p_id, ProgressType p_progressType)
{
	AppGlobal::getGame()->unserializeGameState(p_id, false, p_progressType);
}


void CheckPointMgrWrapper::restoreAndClear(const std::string& p_id)
{
	AppGlobal::getGame()->unserializeGameState(p_id, true);
}


void CheckPointMgrWrapper::restoreAndClearEx(const std::string& p_id, ProgressType p_progressType)
{
	AppGlobal::getGame()->unserializeGameState(p_id, true, p_progressType);
}


tt::str::Strings CheckPointMgrWrapper::getAllIDs()
{
	return AppGlobal::getCheckPointMgr().getAllCheckPointIDs();
}


tt::str::Strings CheckPointMgrWrapper::getAllIDsEx(ProgressType p_progressType)
{
	return AppGlobal::getCheckPointMgr(p_progressType).getAllCheckPointIDs();
}


bool CheckPointMgrWrapper::hasID(const std::string& p_id)
{
	return AppGlobal::getCheckPointMgr().hasCheckPoint(p_id);
}


bool CheckPointMgrWrapper::hasIDEx(const std::string& p_id, ProgressType p_progressType)
{
	return AppGlobal::getCheckPointMgr(p_progressType).hasCheckPoint(p_id);
}


bool CheckPointMgrWrapper::renameID(const std::string& p_oldID, const std::string& p_newID)
{
	return AppGlobal::getCheckPointMgr().renameCheckPoint(p_oldID, p_newID);
}


void CheckPointMgrWrapper::clearID(const std::string& p_id)
{
	AppGlobal::getCheckPointMgr().resetCheckPoint(p_id);
}


void CheckPointMgrWrapper::clearAll()
{
	AppGlobal::getCheckPointMgr().resetAllCheckPoints();
}


void CheckPointMgrWrapper::clearAllEx(ProgressType p_progressType)
{
	AppGlobal::getCheckPointMgr(p_progressType).resetAllCheckPoints();
}


bool CheckPointMgrWrapper::canStore()
{
	return AppGlobal::getCheckPointMgr().canStoreProgress();
}


u32 CheckPointMgrWrapper::getNumberOfCheckpoints(ProgressType p_progressType)
{
	return AppGlobal::getCheckPointMgr(p_progressType).getNumberOfCheckpoints();
}


void CheckPointMgrWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NO_INSTANCING_NAME(CheckPointMgrWrapper, "CheckPointMgr");
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, store);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, restore);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, restoreEx);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, restoreAndClear);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, restoreAndClearEx);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, getAllIDs);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, getAllIDsEx);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, hasID);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, hasIDEx);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, renameID);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, clearID);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, clearAll);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, clearAllEx);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, canStore);
	TT_SQBIND_STATIC_METHOD(CheckPointMgrWrapper, getNumberOfCheckpoints);
}

// Namespace end
}
}
}
}
