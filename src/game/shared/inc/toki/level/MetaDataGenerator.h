#if !defined(INC_TOKI_LEVEL_METADATAGENERATOR_H)
#define INC_TOKI_LEVEL_METADATAGENERATOR_H

#include <string>

#include <squirrel/squirrel.h>

#include <tt/script/ScriptValue.h>

#include <toki/level/entity/EntityInstance.h>

#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
#	define TT_ALLOW_METADATA_GENERATE 1
#else
#	define TT_ALLOW_METADATA_GENERATE 0 // Should be 0!
#endif

namespace toki {
namespace level {

class MetaDataGenerator
{
public:
	MetaDataGenerator();
	
#if TT_ALLOW_METADATA_GENERATE
	bool generate(const std::string& p_levelFolder);
	bool saveToBinaryFile(const std::string& p_filename);
	bool saveToTextFile(const std::string& p_filename);
#endif
	
	bool loadFromBinaryFile(const std::string& p_filename);
	
	inline bool isLoaded() const { return m_isLoaded; }
	
	inline const tt::script::ScriptValue& getMetaData() const { return m_metaData; }
	
private:
	bool shouldGenerateMetaData(HSQUIRRELVM p_vm, const std::string& p_levelName);
	void callGenerateMetaData(HSQUIRRELVM p_vm, const HSQOBJECT& p_levelData, 
	                          const HSQOBJECT& p_resultData);
	void validateMetaData(HSQUIRRELVM p_vm, const HSQOBJECT& p_newData, const HSQOBJECT& p_oldData);
	void processLevel (const std::string& p_levelFilePath,
	                   HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable);
	void processEntity(const std::string& p_classType,
	                   const entity::EntityInstance::Properties& p_properties,
	                   HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable);
	void addEntityInstance(const entity::EntityInstancePtr& p_entity,
	                       HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable);
	
	tt::script::ScriptValue m_metaData;
	bool                    m_isLoaded;
};


// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_METADATAGENERATOR_H)
