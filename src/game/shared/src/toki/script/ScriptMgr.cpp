#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
#include <tt/app/Application.h>
#include <tt/args/CmdLine.h>
#endif

#include <squirrel/sqbind.h>
#include <squirrel/sqstdio.h>

#if !defined(TT_BUILD_FINAL)
#include <squirrel/sqprofiler/sqprofiler.h>
#endif


#include <tt/code/bufferutils.h>
#include <tt/fs/utils/utils.h>
#include <tt/fs/Dir.h>
#include <tt/fs/DirEntry.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/script/bindings/bindings.h>
#include <tt/str/common.h>
#include <tt/system/Time.h>

#include <toki/game/hud/types.h>
#include <toki/game/script/wrappers/AudioWrapper.h>
#include <toki/game/script/wrappers/CameraEffectWrapper.h>
#include <toki/game/script/wrappers/CameraWrapper.h>
#include <toki/game/script/wrappers/CheckPointMgrWrapper.h>
#include <toki/game/script/wrappers/DarknessWrapper.h>
#include <toki/game/script/wrappers/DebugViewWrapper.h>
#include <toki/game/script/wrappers/EffectMgrWrapper.h>
#include <toki/game/script/wrappers/EffectRectWrapper.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/wrappers/FluidSettingsWrapper.h>
#include <toki/game/script/wrappers/LevelWrapper.h>
#include <toki/game/script/wrappers/LightWrapper.h>
#include <toki/game/script/wrappers/MusicTrackWrapper.h>
#include <toki/game/script/wrappers/OptionsWrapper.h>
#include <toki/game/script/wrappers/ParticleEffectWrapper.h>
#include <toki/game/script/wrappers/PhysicsSettingsWrapper.h>
#include <toki/game/script/wrappers/PointerEventWrapper.h>
#include <toki/game/script/wrappers/PowerBeamGraphicWrapper.h>
#include <toki/game/script/wrappers/PresentationObjectWrapper.h>
#include <toki/game/script/wrappers/PresentationStartSettingsWrapper.h>
#include <toki/game/script/wrappers/ResolutionChangerWrapper.h>
#include <toki/game/script/wrappers/ResolutionPickerWrapper.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/wrappers/ShoeboxPlaneDataWrapper.h>
#include <toki/game/script/wrappers/SoundCueWrapper.h>
#include <toki/game/script/wrappers/StatsWrapper.h>
#include <toki/game/script/wrappers/TextureWrapper.h>
#include <toki/game/script/wrappers/TextLabelWrapper.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/wrappers/WorkshopLevelPickerWrapper.h>
#include <toki/game/script/Bindings.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/EntityScriptMgr.h>
#include <toki/game/script/fwd.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/script/serialization/SQCache.h>
#include <toki/script/serialization/SQSerializer.h>
#include <toki/script/serialization/SQUnserializer.h>
#include <toki/script/ScriptMgr.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace script {


bool                              ScriptMgr::ms_initialized = false;
HSQOBJECT                         ScriptMgr::ms_initRootTable;
HSQOBJECT                         ScriptMgr::ms_metaData;
tt::script::VirtualMachinePtr     ScriptMgr::ms_vm;
script::serialization::SQCachePtr ScriptMgr::ms_serializationCachePtr;

//--------------------------------------------------------------------------------------------------
// Helper functions

class nocaseCompare : public std::less<std::string>
{
	bool operator()(const std::string& p_first, const std::string& p_second) const
	{
		return tt::str::toLower(p_first) < tt::str::toLower(p_second);
	}
};

//--------------------------------------------------------------------------------------------------
// Public member functions


void ScriptMgr::init()
{
	TT_ASSERT(ms_initialized == false);
#if !defined(TT_BUILD_FINAL)
	u64 scriptInitStart = tt::system::Time::getInstance()->getMilliSeconds();
#endif // #if !defined(TT_BUILD_FINAL)
	
	TT_ASSERT(ms_vm == 0);
	TT_ASSERT(ms_initialized ==  false);
	
	s32 debugPort = 0;
	// Set rootpath (postfix with "/") and setup debug server on if sqDebugPort > 0
#if defined(TT_PLATFORM_WIN) && !defined(TT_BUILD_FINAL)
	if (tt::app::hasApplication())
	{
		const tt::args::CmdLine& cmdLine(tt::app::getApplication()->getCmdLine());
		
		if (cmdLine.exists("sqDebugPort"))
		{
			const s32 sqDebugPort = cmdLine.getInteger("sqDebugPort");
			if (sqDebugPort  > 0)
			{
				debugPort = sqDebugPort;
			}
		}
	}
	
	// non-final windows build. (test/pack build)
	const tt::script::VMCompileMode compileMode = (AppGlobal::shouldCompileSquirrel()) ? 
	                                              tt::script::VMCompileMode_NutToBnut  :
	                                              tt::script::VMCompileMode_NutOnly;
#elif defined(TT_BUILD_FINAL)
	// Final build
	const tt::script::VMCompileMode compileMode = tt::script::VMCompileMode_BnutOnly;
#else
	// Other builds
	const tt::script::VMCompileMode compileMode = tt::script::VMCompileMode_NutOnly;
#endif
	
	ms_vm = tt::script::ScriptEngine::createVM("scripts/", debugPort, compileMode);
	
	ms_initialized = true;
	sq_resetobject(&ms_initRootTable);
	sq_resetobject(&ms_metaData);
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	sq_registerscriptprofiler(ms_vm->getVM());
#endif
	
	// Bind all ttdev wrappers
	{
		using namespace tt::script::bindings;
		bindColorRGB(ms_vm);
		bindColorRGBA(ms_vm);
		bindVector2(ms_vm);
		bindVectorRect(ms_vm);
		bindRandom(ms_vm);
		bindLeaderboardsTypes(ms_vm);
		bindStatsTypes(ms_vm);
		bindKeyTypes(ms_vm);
		bindInterpolation(ms_vm);
		bindTime(ms_vm);
		bindInput(ms_vm);
		bindPlatform(ms_vm);
		bindSharedWrappers(ms_vm);
	}
	
	// Bind all wrappers
	{
		game::hud::ListBoxColorScheme::bind(ms_vm);
		
		using namespace game::script::wrappers;
		AudioWrapper::bind(ms_vm);
		CameraEffectWrapper::bind(ms_vm);
		CameraWrapper::bind(ms_vm);
		CheckPointMgrWrapper::bind(ms_vm);
		DrcCameraWrapper::bind(ms_vm);
		DarknessWrapper::bind(ms_vm);
		DebugViewWrapper::bind(ms_vm);
		EffectMgrWrapper::bind(ms_vm);
		EffectRectWrapper::bind(ms_vm);
		EventWrapper::bind(ms_vm);
		FluidSettingsWrapper::bind(ms_vm);
		LevelWrapper::bind(ms_vm);
		LightWrapper::bind(ms_vm);
		MusicTrackWrapper::bind(ms_vm);
		OptionsWrapper::bind(ms_vm);
		ParticleEffectWrapper::bind(ms_vm);
		PhysicsSettingsWrapper::bind(ms_vm);
		PointerEventWrapper::bind(ms_vm);
		PowerBeamGraphicWrapper::bind(ms_vm);
		PresentationObjectWrapper::bind(ms_vm);
		PresentationStartSettingsWrapper::bind(ms_vm);
		ResolutionChangerWrapper::bind(ms_vm);
		ResolutionPickerWrapper::bind(ms_vm);
		SensorWrapper::bind(ms_vm);
		ShapeWrapper::bind(ms_vm);
		ShoeboxPlaneDataWrapper::bind(ms_vm);
		SoundCueWrapper::bind(ms_vm);
		StatsWrapper::bind(ms_vm);
		TextureWrapper::bind(ms_vm);
		TextLabelWrapper::bind(ms_vm);
		TileSensorWrapper::bind(ms_vm);
		WorkshopLevelPickerWrapper::bind(ms_vm);
		
		game::script::Bindings::bindAll(ms_vm);
	}
	
	// Load metadata
	level::MetaDataGenerator& generator = AppGlobal::getMetaDataGenerator();
	generator.loadFromBinaryFile("metadata/levels.ttmeta");
	
	// Create metadata squirrel object.
	if (generator.isLoaded())
	{
		// Helpers
		HSQUIRRELVM v =  ms_vm->getVM();
		tt::script::SqTopRestorerHelper helper(v, true);
		// Push metadata to stack and clone.
		generator.getMetaData().pushOnStack(v);
		// Assign to ms_metaData.
		sq_release(    v,     &ms_metaData);
		sq_resetobject(&ms_metaData);
		sq_getstackobj(v, -1, &ms_metaData);
		sq_addref(     v,     &ms_metaData);
		sq_pop(        v,     1);
	}
	
	// Load all the script includes
	loadIncludes("includes");
	
	// Init the entity script mgr (and all entity scripts)
	game::script::EntityScriptMgr& entityScriptMgr = AppGlobal::getEntityScriptMgr();
	entityScriptMgr.init("entities");
	
	loadIncludes("post_includes"); // includes after entities.
	
#if !defined(TT_BUILD_FINAL)
	if (AppGlobal::isInDeveloperMode())
	{
		loadIncludes("test_includes");
	}
#endif
	
	// END SCRIPT LOADING
	/////////////////////
	
	// Register all Entity attributes; do this after all scripts have been loaded and compiled!
	entityScriptMgr.registerAttributes();
	
	// Save roottable.
	{
		tt::script::SqTopRestorerHelper helper(ms_vm->getVM());
		sq_pushroottable(ms_vm->getVM());
		const SQRESULT result = sq_clone(ms_vm->getVM(), -1);
		TT_ASSERT(SQ_SUCCEEDED(result));
		sq_release(    ms_vm->getVM(),     &ms_initRootTable);
		sq_resetobject(&ms_initRootTable);
		sq_getstackobj(ms_vm->getVM(), -1, &ms_initRootTable);
		sq_addref(     ms_vm->getVM(),     &ms_initRootTable);
	}
	
	// Build cache
	ms_serializationCachePtr.reset(new serialization::SQCache(ms_vm->getVM()));
	
#if !defined(TT_BUILD_FINAL)
	u64 scriptInitEnd = tt::system::Time::getInstance()->getMilliSeconds();
	TT_Printf("ScriptMgr::init took %u milliseconds.\n", u32(scriptInitEnd - scriptInitStart));
#endif // #if !defined(TT_BUILD_FINAL)
}


bool ScriptMgr::isInitialized()
{
	return ms_initialized && ms_vm->isInitialized();
}


void ScriptMgr::deinit()
{
	TT_Printf("ScriptMgr::deinit\n");
	if (ms_initialized == false)
	{
		return;
	}
	
	// Deinit the entity script mgr
	AppGlobal::getEntityScriptMgr().deinit();
	
	ms_vm.reset();
	
	// uninitialize all bindings
	SqBind_UnInitAll();
	
	tt::script::ScriptEngine::destroy();
	ms_initialized = false;
}


void ScriptMgr::reset()
{
	// Clear old root table
	const HSQUIRRELVM v = ms_vm->getVM();
	{
		tt::script::SqTopRestorerHelper tmp(v, true);
		
		sq_pushroottable(v);
		
		// Save the surviving _perm_table first
		HSQOBJECT permTable;
		sq_resetobject(&permTable);
		sq_pushstring(v, "_perm_table", -1);
		if (SQ_SUCCEEDED(sq_get(v, -2)))
		{
			sq_getstackobj(v, -1, &permTable);
			sq_addref(v, &permTable);
			sq_poptop(v);
		}
		
		// Now we can clear the roottable
		sq_clear(v, -1);
		
		// Restore initial root table.
		sq_pushobject(v, ms_initRootTable);
		sq_pushnull(v);
		while(SQ_SUCCEEDED(sq_next(v, -2)))
		{
			SQRESULT result = sq_newslot(v, -5, SQFalse);
			TT_ASSERT(SQ_SUCCEEDED(result));
		}
		sq_pop(v, 2); // pop null iterator and ms_initRootTable
		
		// And finally insert the _perm_table again
		if (sq_isnull(permTable) == false)
		{
			sq_pushstring(v, "_perm_table", -1);
			sq_pushobject(v, permTable);
			SQRESULT result = sq_newslot(v, -3, SQFalse);
			TT_ASSERT(SQ_SUCCEEDED(result));
		}
		
		sq_poptop(v); // pop actual roottable
	}
	
	ms_vm->reset();
}


void ScriptMgr::update()
{
	ms_vm->update();
	
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	sq_updateprofiler(ms_vm->getVM());
#endif
}


bool ScriptMgr::loadScript(const std::string& p_filename)
{
	if (ms_vm->loadAndRunScript(p_filename) == false)
	{
		TT_PANIC("Running script '%s' failed", p_filename.c_str());
		return false;
	}
	return true;
}


void ScriptMgr::initLevel(const std::string& p_filename)
{
	loadLevelScript(p_filename);
}


void ScriptMgr::runScriptBuf(const std::string& p_bufferedScript)
{
	TT_ASSERT(isInitialized());
	ms_vm->runScriptBuf(p_bufferedScript);
}


void ScriptMgr::loadIncludes(const std::string& p_path)
{
	const std::string path(ms_vm->getRoot() + p_path);
	
	if (tt::fs::dirExists(path) == false)
	{
		return;
	}
	
	tt::fs::DirPtr dir(tt::fs::openDir(path));
	if (dir == 0)
	{
		return;
	}
	
	// Use a set so we can sort the script names as we insert them
	nocaseCompare cmp;
	tt::str::StringSet file_list(cmp);

	HSQUIRRELVM v = ms_vm->getVM();
	tt::fs::DirEntry entry;
	while (dir->read(entry))
	{
		const std::string& fileName(entry.getName());
		
		if (entry.isDirectory())
		{
			if (fileName != "." && fileName != "..")
			{
				file_list.insert(p_path + "/" + fileName + "/");
			}
			continue;
		}
		
		if (tt::fs::utils::getExtension(fileName) != "nut"  &&
		    tt::fs::utils::getExtension(fileName) != "bnut" )
		{
			continue;
		}
		
		file_list.insert(p_path + "/" + fileName);
	}
	
	for (tt::str::StringSet::const_iterator it = file_list.begin(); it != file_list.end(); ++it)
	{
		const std::string& filePath = *it;
		
		if (filePath[filePath.length() - 1] == '/')
		{
			loadIncludes(filePath);
		}
		else
		{
			sq_pushstring(v, filePath.c_str(), -1);
			tt::script::VirtualMachine::tt_include(v);
			sq_poptop(v);
		}
	}
}


void ScriptMgr::pushMetaData()
{
	HSQUIRRELVM v = ms_vm->getVM();
	tt::script::SqTopRestorerHelper helper(v, true, 1); // We return the metadata object on the stack.
	// Push metadata to stack and clone.
	sq_pushobject(v, ms_metaData);
	const SQRESULT result = sq_clone(v, -1);
	TT_ASSERT(SQ_SUCCEEDED(result));
	sq_remove(v, -2); // Remove original (and keep clone on stack.)
}


void ScriptMgr::pushJSONFromString(const std::string& p_string)
{
	HSQUIRRELVM v = ms_vm->getVM();
	tt::script::SqTopRestorerHelper helper(v, true, 1); // We return the JSON object on the stack.
	
	if (p_string.empty() == false)
	{
		const char* jsonDataBegin = p_string.c_str();
		const char* jsonDataEnd   = jsonDataBegin + p_string.size();
		Json::Value  rootNode;
		Json::Reader reader;
		if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false))
		{
			pushJSON(rootNode);
			return;
		}
		else
		{
			TT_PANIC("string could not be parsed as JSON.\nError Message:\n%s",
				reader.getFormatedErrorMessages().c_str());
			sq_pushnull(v);
			return;
		}
	}
	
	TT_PANIC("Invalid or empty JSON string '%s'");
	sq_pushnull(v);
	return;
}


void ScriptMgr::pushJSONFromFile(const std::string& p_filename)
{
	HSQUIRRELVM v = ms_vm->getVM();
	tt::script::SqTopRestorerHelper helper(v, true, 1); // We return the JSON object on the stack.
	
	if (tt::fs::fileExists(p_filename) == false)
	{
		TT_PANIC("JSON file '%s' does not exist.", p_filename.c_str());
		sq_pushnull(v);
		return;
	}
	
	tt::fs::FilePtr file = tt::fs::open(p_filename, tt::fs::OpenMode_Read);
	if (file == 0)
	{
		TT_PANIC("Opening file '%s' for reading failed.", p_filename.c_str());
		sq_pushnull(v);
		return;
	}
	
	tt::code::BufferPtr fileContent = file->getContent();
	if (fileContent != 0 && file->getLength() > 0)
	{
		const char* jsonDataBegin = reinterpret_cast<const char*>(fileContent->getData());
		const char* jsonDataEnd   = jsonDataBegin + fileContent->getSize();
		Json::Value  rootNode;
		Json::Reader reader;
		file.reset(); // close file here
		if (reader.parse(jsonDataBegin, jsonDataEnd, rootNode, false))
		{
			pushJSON(rootNode);
			return;
		}
		else
		{
			TT_PANIC("'%s' file could not be parsed as JSON.\nError Message:\n%s", p_filename.c_str(),
				reader.getFormatedErrorMessages().c_str());
			sq_pushnull(v);
			return;
		}
	}
	
	TT_PANIC("Invalid or empty JSON file '%s'", p_filename.c_str());
	sq_pushnull(v);
	return;
}


void ScriptMgr::pushJSON(const Json::Value& p_value)
{
	HSQUIRRELVM v = ms_vm->getVM();
	
	// Taken from http://forum.squirrel-lang.org/default.aspx?g=posts&m=4737
	s32 i, num;
	Json::Value::Members keys;
	Json::Value::Members::iterator it;
	
	switch(p_value.type())
	{
	case Json::nullValue:
		sq_pushnull(v);
		break;
		
	case Json::intValue:
		sq_pushinteger(v, static_cast<SQInteger>(p_value.asInt()));
		break;
		
	case Json::uintValue:
		sq_pushinteger(v, static_cast<SQInteger>(p_value.asUInt()));
		break;
		
	case Json::realValue:
		sq_pushfloat(v, static_cast<SQFloat>(p_value.asDouble()));
		break;
		
	case Json::stringValue:
		sq_pushstring(v, p_value.asCString(), -1);
		break;
		
	case Json::booleanValue:
		sq_pushbool(v, static_cast<SQBool>(p_value.asBool()));
		break;
		
	case Json::arrayValue:
		num = p_value.size();
		sq_newarray(v, num);
		for (i = 0; i < num; i++)
		{
			sq_pushinteger(v, static_cast<SQInteger>(i));
			pushJSON(p_value[i]);
			
			/* now array element is on the top of the stack */
			sq_set(v, -3);
		}
		break;
		
	case Json::objectValue:
		num = p_value.size();
		sq_newtable(v);
		keys = p_value.getMemberNames();
		for (it = keys.begin(); it != keys.end(); ++it)
		{
			sq_pushstring(v, it->c_str(), -1);
			pushJSON(p_value[*it]);
			
			/* now table field value element is on the top of the stack */
			sq_newslot(v, -3, false);
		}
		break;
		
	default:
		TT_PANIC("Unhandled JSON Type '%d'", p_value.type());
		break;
	}
}


void ScriptMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr)
{
	HSQUIRRELVM v = ms_vm->getVM();
	
	////////////////////////////////
	// PREPROCESS FOR SERIALIZATION
	
	serialization::SQSerializer serializer(ms_serializationCachePtr);
	
	// Process roottable
	serializer.processRoottable(v);
	
	using namespace game::entity;
	EntityMgr& mgr = AppGlobal::getGame()->getEntityMgr();
	
	// Process all entities (squirrel part)
	{
		const Entity* entity = mgr.getFirstEntity();
		for (s32 i = 0; i < mgr.getActiveEntitiesCount(); ++i, ++entity)
		{
			entity->getEntityScript()->addObjectToSQSerializer(serializer);
		}
	}
	
	////////////////////////////////
	// SERIALIZATION
	
	const toki::serialization::SerializerPtr& section = 
		p_serializationMgr.getSection(toki::serialization::Section_Squirrel);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Squirrel data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	// Serialize the preprocessed Squirrel part
	serializer.serialize(&context);
	
	{
		const Entity* entity  = mgr.getFirstEntity();
		const u32 entityCount = static_cast<u32>(mgr.getActiveEntitiesCount());
		tt::code::bufferutils::put(entityCount, &context);
		
		for (u32 i = 0; i < entityCount ; ++i, ++entity)
		{
			entity->getEntityScript()->serializeSQSerializerObjects(serializer, &context);
		}
	}
	
	AppGlobal::getEntityScriptMgr().serialize(&context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void ScriptMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	// Reset the VM to original state
	reset();
	
	serialization::SQUnserializer unserializer(ms_vm->getVM(), ms_serializationCachePtr);
	
	const toki::serialization::SerializerPtr& section =
		p_serializationMgr.getSection(toki::serialization::Section_Squirrel);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Squirrel data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	unserializer.unserialize(&context);
	
	{
		using namespace game::entity;
		EntityMgr& mgr = AppGlobal::getGame()->getEntityMgr();
		
		const u32 entityCount = tt::code::bufferutils::get<u32>(&context);
		
		// FIXME: What todo if this triggers? (The entity ptr below will become invalid!) (Should still consume the bytes in context as client code expects.)
		TT_ASSERT(entityCount == static_cast<u32>(mgr.getActiveEntitiesCount()));
		
		Entity* entity = mgr.getFirstEntity();
		for (u32 i = 0; i < entityCount; ++i, ++entity)
		{
			entity->getEntityScript()->unserializeSQSerializerObjects(unserializer, &context);
		}
	}
	
	AppGlobal::getEntityScriptMgr().unserialize(&context);
	
#if DEBUG_SQUIRREL_ENTITY_BASE_LIFETIME
	HSQUIRRELVM v = getVM()->getVM();
	SQRESULT result = sq_resurrectunreachable(v);
	TT_ASSERT(SQ_SUCCEEDED(result));
	const bool foundUnreachables = sq_gettype(v, -1) != OT_NULL;
	sq_poptop(v);
	if (foundUnreachables)
	{
		TT_Printf("ScriptMgr unserialize unreachables:\n");
		
		getVM()->callSqFun("checkAndPrintUnreachables");
		TT_NONFATAL_PANIC("Found resurrectunreachable squirrel objects after unserialize!");
		sq_collectgarbage(v);
	}
#endif
}


//--------------------------------------------------------------------------------------------------
// Private member functions

bool ScriptMgr::loadLevelScript(const std::string& p_filename)
{
	if (ms_vm->loadAndRunScriptNoRootPath(p_filename) == false)
	{
		TT_WARNING("Running level script '%s' failed", p_filename.c_str());
		return false;
	}
	return true;
}


// Namespace end
}
}
