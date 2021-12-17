#if !defined(INC_TOKITORI_SCRIPT_SCRIPTMGR_H)
#define INC_TOKITORI_SCRIPT_SCRIPTMGR_H


#include <json/json.h>
#include <string>


#include <tt/script/ScriptEngine.h>

#include <toki/script/serialization/fwd.h>
#include <toki/serialization/fwd.h>

namespace toki {
namespace script {


class ScriptMgr
{
public:
	static void init();
	static bool isInitialized();
	static void deinit();
	static void reset();
	static void update();
	
	static bool loadScript(const std::string& p_filename);
	static void initLevel(const std::string& p_filename);
	static void runScriptBuf(const std::string& p_bufferedScript);
	static inline tt::script::VirtualMachinePtr getVM() { return ms_vm; }
	static void loadIncludes(const std::string& p_path);
	static void pushMetaData();
	static void pushJSON(const Json::Value& p_value);
	static void pushJSONFromString(const std::string& p_file);
	static void pushJSONFromFile(const std::string& p_file);
	
	static void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr);
	static void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	// no ctor or dtor.
	ScriptMgr();
	~ScriptMgr();
	
	static bool loadLevelScript(const std::string& p_filename);
	
	static bool ms_initialized;
	static HSQOBJECT ms_initRootTable;
	static HSQOBJECT ms_metaData;
	static tt::script::VirtualMachinePtr ms_vm;
	static script::serialization::SQCachePtr ms_serializationCachePtr;
};


// Namespace end
}
}


#endif // !defined(INC_TOKITORI_SCRIPT_SCRIPTMGR_H)
