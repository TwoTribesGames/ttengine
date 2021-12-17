#include <tt/app/Application.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/fs.h>
#include <tt/fs/File.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>

#include <toki/game/script/EntityScriptClass.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/level/LevelData.h>
#include <toki/level/MetaDataGenerator.h>
#include <toki/script/ScriptMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace level {

//--------------------------------------------------------------------------------------------------
// Public member functions

MetaDataGenerator::MetaDataGenerator()
:
m_metaData(),
m_isLoaded(false)
{
}


#if TT_ALLOW_METADATA_GENERATE
bool MetaDataGenerator::generate(const std::string& p_levelFolder)
{
	tt::script::VirtualMachinePtr vm(toki::script::ScriptMgr::getVM());
	if (vm == 0)
	{
		TT_PANIC("ScriptMgr::getVM() returns 0. No Virtual Machine present.");
		return false;
	}
	
	// path out of output and back into source.
	// We don't want to ship these files we don't put them in output.
	const std::string path(tt::app::getApplication()->getAssetRootDir() + "../../source/shared/metadata_generation/");
	
	TT_ASSERT(vm->getCompileMode() == tt::script::VMCompileMode_NutOnly); // We don't want the following nut file to be replaced by a bnut.
	
	// Load the metadata script
	vm->loadAndRunScriptNoRootPath(path + "metadata");
	
	HSQUIRRELVM v = vm->getVM();
	
	if (tt::fs::dirExists(p_levelFolder) == false)
	{
		TT_PANIC("Folder '%s' doesn't exists.", p_levelFolder.c_str());
		return false;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(p_levelFolder, "*.ttlvl"));
	if (dir == 0)
	{
		TT_PANIC("Failed to open folder '%s' for processing levels", p_levelFolder.c_str());
		return false;
	}
	
	tt::script::SqTopRestorerHelper helper(v, true);
	
	HSQOBJECT resultData;
	sq_newtable(v);
	
	sq_getstackobj(v, -1, &resultData);
	
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		if (entry.isDirectory())
		{
			continue;
		}
		
		const std::string& levelName(tt::fs::utils::getFileTitle(entry.getName()));
		
		if (shouldGenerateMetaData(v, levelName) == false)
		{
			continue;
		}
		
		const std::string filePath(p_levelFolder + "/" + entry.getName());
		processLevel(filePath, v, resultData);
	}
	
	m_metaData = tt::script::ScriptValue::create(v, -1);
	m_isLoaded = true;
	
	MetaDataGenerator oldDataGenerator;
	oldDataGenerator.loadFromBinaryFile(path + "cat_v1_1.ttmeta");
	if (oldDataGenerator.isLoaded() == false)
	{
		sq_poptop(v); // cleanup resultData.
		return false;
	}
	
	HSQOBJECT oldData;
	oldDataGenerator.getMetaData().pushOnStack(v);
	sq_getstackobj(v, -1, &oldData);
	
	// Validate will only panic on errors.
	validateMetaData(v, resultData, oldData);
	
	sq_pop(v, 2); // cleanup resultData and oldData
	
	return true;
}


bool MetaDataGenerator::saveToBinaryFile(const std::string& p_relativeFilename)
{
	const std::string fullPath(tt::fs::getWorkingDir() + p_relativeFilename);
	tt::fs::FilePtr file = tt::fs::open(fullPath, tt::fs::OpenMode_Write);
	if (file == 0)
	{
		TT_PANIC("fullPath '%s'\nWorking Dir '%s'\nRel filename '%s'\n",
			fullPath.c_str(), tt::fs::getWorkingDir().c_str(), p_relativeFilename.c_str());
		return false;
	}
	
	tt::code::AutoGrowBufferPtr writeBuffer =
		tt::code::AutoGrowBuffer::create(2048, 1024);
	
	tt::code::BufferWriteContext context(writeBuffer->getAppendContext());
	m_metaData.serialize(&context);
	
	context.flush();
	
	bool saveOk = true;
	
	// Save the data's total (used/written) size
	saveOk = saveOk && tt::fs::writeInteger(file, static_cast<u32>(writeBuffer->getUsedSize()));
	
	// Save all data blocks
	const s32 blockCount = writeBuffer->getBlockCount();
	for (s32 i = 0; i < blockCount; ++i)
	{
		const tt::fs::size_type blockSize = static_cast<tt::fs::size_type>(writeBuffer->getBlockSize(i));
		saveOk = saveOk && (file->write(writeBuffer->getBlock(i), blockSize) == blockSize);
	}
	
	return saveOk;
}


bool MetaDataGenerator::saveToTextFile(const std::string& p_relativeFilename)
{
	tt::fs::FilePtr file = tt::fs::open(p_relativeFilename, tt::fs::OpenMode_Write);
	if (file == 0)
	{
		return false;
	}
	
	std::string output = m_metaData.toString();
	
	const tt::fs::size_type size =  static_cast<tt::fs::size_type>(output.size());
	return tt::fs::write(file, output.c_str(), size) != size;
}
#endif


bool MetaDataGenerator::loadFromBinaryFile(const std::string& p_relativeFilename)
{
	m_isLoaded = false;
	
	if (tt::fs::fileExists(p_relativeFilename) == false)
	{
		TT_PANIC("Couldn't load meta data file '%s'", p_relativeFilename.c_str());
		return false;
	}
	
	tt::fs::FilePtr file = tt::fs::open(p_relativeFilename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		return false;
	}
	
	u32 dataSize = 0;
	if (tt::fs::readInteger(file, &dataSize) == false)
	{
		TT_PANIC("Failed to read from '%s'.", p_relativeFilename.c_str());
		return false;
	}
	
	tt::code::BufferPtrForCreator buffer(new tt::code::Buffer(dataSize));
	if (tt::fs::read(file, buffer->getData(), dataSize) != static_cast<tt::fs::size_type>(dataSize))
	{
		TT_PANIC("Failed to read from '%s'.", p_relativeFilename.c_str());
		return false;
	}
	
	tt::code::BufferReadContext context(buffer->getReadContext());
	
	m_metaData = tt::script::ScriptValue::unserialize(&context);
	m_isLoaded = true;
	
	return true;
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool MetaDataGenerator::shouldGenerateMetaData(HSQUIRRELVM p_vm, const std::string& p_levelName)
{
	tt::script::SqTopRestorerHelper helper(p_vm);
	
	sq_pushroottable(p_vm);
	sq_pushstring(p_vm, "shouldGenerateMetaData", -1);
	sq_get(p_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(p_vm, sq_gettop(p_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return true;
	}
	
	sq_pushroottable(p_vm); // this (function environment object)
	sq_pushstring(p_vm, p_levelName.c_str(), -1);
	
	if (SQ_FAILED(sq_call(p_vm, 2, SQTrue, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Error calling shouldGenerateMetaData()");
		return true;
	}
	
	SQBool rVal = SQTrue;
	if (sq_gettype(p_vm, -1) != OT_BOOL)
	{
		TT_PANIC("shouldGenerateMetaData should return a boolean value.");
		return true;
	}
	
	sq_getbool(p_vm, -1, &rVal);
	return rVal == SQTrue;
}


void MetaDataGenerator::callGenerateMetaData(HSQUIRRELVM p_vm, const HSQOBJECT& p_levelData, 
                                             const HSQOBJECT& p_resultData)
{
	TT_ASSERT(p_levelData._type == OT_TABLE);
	TT_ASSERT(p_resultData._type == OT_TABLE);
	
	tt::script::SqTopRestorerHelper helper(p_vm);
	
	sq_pushroottable(p_vm);
	sq_pushstring(p_vm, "generateMetaData", -1);
	sq_get(p_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(p_vm, sq_gettop(p_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return;
	}
	
	sq_pushroottable(p_vm); // this (function environment object)
	sq_pushobject(p_vm, p_levelData);
	sq_pushobject(p_vm, p_resultData);
	
	if (SQ_FAILED(sq_call(p_vm, 3, SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Error calling generateMetaData()");
	}
}


void MetaDataGenerator::validateMetaData(HSQUIRRELVM p_vm, const HSQOBJECT& p_newData, const HSQOBJECT& p_oldData)
{
	TT_ASSERT(p_newData._type  == OT_TABLE);
	TT_ASSERT(p_oldData._type == OT_TABLE);
	
	tt::script::SqTopRestorerHelper helper(p_vm);
	
	sq_pushroottable(p_vm);
	sq_pushstring(p_vm, "validateMetaData", -1);
	sq_get(p_vm, -2); //get the function from the root table
	
	const SQObjectType type(sq_gettype(p_vm, sq_gettop(p_vm)));
	if (type != OT_CLOSURE && type != OT_NATIVECLOSURE)
	{
		return;
	}
	
	sq_pushroottable(p_vm); // this (function environment object)
	sq_pushobject(p_vm, p_newData);
	sq_pushobject(p_vm, p_oldData);
	
	if (SQ_FAILED(sq_call(p_vm, 3, SQFalse, TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Error calling validateMetaData()");
	}
}


void MetaDataGenerator::processLevel(const std::string& p_levelFilePath,
                                     HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable)
{
	LevelDataPtr level(LevelData::loadLevel(p_levelFilePath));
	
	if (level == 0 || level->getLevelFilename().empty())
	{
		TT_PANIC("Trying to process level '%s' but it doesn't exist or has an empty filename.",
		         p_levelFilePath.c_str());
		return;
	}
	
	// Level size
	const tt::math::PointRect levelRect(level->getLevelRect());
	
	// Level properties table
	HSQOBJECT levelData;
	sq_newtable(p_vm);
	{
		sq_getstackobj(p_vm, -1, &levelData);
		sq_pushstring(p_vm, "properties", -1);
		sq_newtable(p_vm);
		{
			// Name
			sq_pushstring(p_vm, "name", -1);
			sq_pushstring(p_vm, level->getLevelFilename().c_str(), -1);
			sq_newslot(p_vm, -3, SQFalse);
			
			// Width
			sq_pushstring(p_vm, "width", -1);
			sq_pushinteger(p_vm, levelRect.getWidth());
			sq_newslot(p_vm, -3, SQFalse);
			
			// Height
			sq_pushstring(p_vm, "height", -1);
			sq_pushinteger(p_vm, levelRect.getHeight());
			sq_newslot(p_vm, -3, SQFalse);
		}
		sq_newslot(p_vm, -3, SQFalse);
		
		sq_pushstring(p_vm, "entities", -1);
		sq_newtable(p_vm);
		{
			HSQOBJECT entityData;
			sq_getstackobj(p_vm, -1, &entityData);
			const entity::EntityInstances& entities = level->getAllEntities();
			for (entity::EntityInstances::const_iterator it = entities.begin(); it != entities.end(); ++it)
			{
				addEntityInstance(*it, p_vm, entityData);
			}
		}
		sq_newslot(p_vm, -3, SQFalse);
		
		callGenerateMetaData(p_vm, levelData, p_resultTable);
	}
	sq_poptop(p_vm);
}


void MetaDataGenerator::processEntity(const std::string& p_classType,
                                      const entity::EntityInstance::Properties& p_properties,
                                      HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable)
{
	tt::script::SqTopRestorerHelper helper(p_vm, true);
	
	TT_ASSERT(p_resultTable._type == OT_TABLE);
	
	const level::entity::EntityInfo* info =
		AppGlobal::getEntityLibrary().getEntityInfo(p_classType);
	if (info == 0)
	{
		// Invalid entitytype (perhaps the entity is deprecated. No need to assert here)
		return;
	}
	
	const game::script::EntityScriptMgr& mgr = AppGlobal::getEntityScriptMgr();
	game::script::EntityScriptClassPtr scriptClass(mgr.getClass(p_classType));
	if (scriptClass == 0)
	{
		TT_PANIC("Cannot retrieve EntityScriptClass for class '%s'", p_classType.c_str());
		return;
	}
	
	using namespace toki::script::attributes;
	const ClassAttributes& attributes = scriptClass->getAttributes();
	const MemberAttributesCollection& members =
		attributes.getMemberAttributesCollection();
	
	// Push result table to stack
	sq_pushobject(p_vm, p_resultTable);
	
	for (MemberAttributesCollection::const_iterator it = members.begin(); it != members.end(); ++it)
	{
		// skip root attributes
		if ((*it).first.empty() )
		{
			continue;
		}
		
		entity::EntityProperty prop((*it).first, (*it).second);
		
		Attribute value = prop.getDefault();
		
		if (Attribute::isArrayType(value.getType()))
		{
			TT_WARN("Skipping array type member of class '%s'. This is not yet properly supported in the editor",
			        p_classType.c_str());
			continue;
		}
		
		entity::EntityInstance::Properties::const_iterator findIt = p_properties.find((*it).first);
		if (findIt != p_properties.end())
		{
			using namespace entity;
			prop.validate((*findIt).second);
			switch (prop.getType())
			{
			case EntityProperty::Type_Float:
				value.setFloat(tt::str::parseReal((*findIt).second, 0));
				break;
				
			case EntityProperty::Type_Bool:
				value.setBool(tt::str::parseBool((*findIt).second, 0));
				break;
				
			case EntityProperty::Type_String:
			case EntityProperty::Type_ColorRGB:
			case EntityProperty::Type_ColorRGBA:
				value.setString((*findIt).second);
				break;
				
			case EntityProperty::Type_Integer:
			case EntityProperty::Type_Entity:
			case EntityProperty::Type_EntityID:
			case EntityProperty::Type_DelayedEntityID:
				value.setInteger(tt::str::parseS32((*findIt).second, 0));
				break;
				
			case EntityProperty::Type_EntityArray:
			case EntityProperty::Type_EntityIDArray:
			case EntityProperty::Type_DelayedEntityIDArray:
				{
					script::attributes::Attribute::IntegerArray entityArray;
					
					tt::str::Strings ids(tt::str::explode((*findIt).second, ","));
					for (tt::str::Strings::iterator valIt = ids.begin(); valIt != ids.end(); ++valIt)
					{
						entityArray.push_back(tt::str::parseS32(*valIt, 0));
					}
					value.setIntegerArray(entityArray);
				}
				break;
				
			default:
				TT_PANIC("Unhandled property type '%s'", EntityProperty::getTypeName(prop.getType()));
				break;
			}
		}
		
		// Push key (name of member)
		sq_pushstring(p_vm, prop.getName().c_str(), -1);
		
		// Push value
		switch (value.getType())
		{
		case Attribute::Type_Null:    sq_pushnull(p_vm); break;
		case Attribute::Type_Integer: sq_pushinteger(p_vm, value.getInteger()); break;
		case Attribute::Type_Float:   sq_pushfloat  (p_vm, value.getFloat());   break;
		case Attribute::Type_Bool:    sq_pushbool   (p_vm, value.getBool() == true ? SQTrue : SQFalse); break;
		case Attribute::Type_String:  sq_pushstring (p_vm, value.getString().c_str(), -1); break;
		case Attribute::Type_IntegerArray:
			{
				using script::attributes::Attribute;
				const Attribute::IntegerArray& entityArray = value.getIntegerArray();
				
				sq_newarray(p_vm, static_cast<SQInteger>(entityArray.size()));
				
				for (Attribute::IntegerArray::const_iterator elemIt = entityArray.begin();
				     elemIt != entityArray.end(); ++elemIt)
				{
					sq_pushinteger(p_vm, *elemIt);
					sq_arrayappend(p_vm, -2);
				}
			}
			break;
		//case Attribute::Type_FloatArray:
		//case Attribute::Type_BoolArray:
		//case Attribute::Type_StringArray:
		
		default:
			TT_PANIC("Unhandled attribute type '%s'", Attribute::getTypeName(value.getType()));
			sq_pushnull(p_vm);
			break;
		}
		
		sq_newslot(p_vm, -3, SQFalse);
	}
	
	// Pop result table from stack
	sq_poptop(p_vm);
}


void MetaDataGenerator::addEntityInstance(const entity::EntityInstancePtr& p_entity,
                                          HSQUIRRELVM p_vm, const HSQOBJECT& p_resultTable)
{
	if (p_entity == 0)
	{
		return;
	}
	
	tt::script::SqTopRestorerHelper helper(p_vm, true);
	
	sq_pushobject(p_vm, p_resultTable);
	
	// Table entry key
	sq_pushinteger(p_vm, p_entity->getID());
	
	// Table enty value (a new table
	sq_newtable(p_vm);
	{
		// Type
		sq_pushstring(p_vm, "type", -1);
		sq_pushstring(p_vm, p_entity->getType().c_str(), -1);
		sq_newslot(p_vm, -3, SQFalse);
		
		// Position
		sq_pushstring(p_vm, "position", -1);
		sq_newtable(p_vm);
		{
			// x
			sq_pushstring(p_vm, "x", -1);
			sq_pushfloat(p_vm, p_entity->getPosition().x);
			sq_newslot(p_vm, -3, SQFalse);
			
			// y
			sq_pushstring(p_vm, "y", -1);
			sq_pushfloat(p_vm, p_entity->getPosition().y);
			sq_newslot(p_vm, -3, SQFalse);
		}
		sq_newslot(p_vm, -3, SQFalse);
		
		// Properties
		sq_pushstring(p_vm, "properties", -1);
		sq_newtable(p_vm);
		{
			HSQOBJECT table;
			sq_getstackobj(p_vm, -1, &table);
			const entity::EntityInstance::Properties& properties = p_entity->getProperties();
			processEntity(p_entity->getType(), properties, p_vm, table);
		}
		sq_newslot(p_vm, -3, SQFalse);
	}
	
	// Set key/value and pop table
	sq_newslot(p_vm, -3, SQFalse);
	sq_poptop(p_vm);
}


// Namespace end
}
}
