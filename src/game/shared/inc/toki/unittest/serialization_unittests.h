#if !defined(TT_INC_TOKI_UNITTEST_SERIALIZATION_UNITTESTS_H)
#define TT_INC_TOKI_UNITTEST_SERIALIZATION_UNITTESTS_H


#include <set>

#include <unittestpp/unittestpp.h>

#include <tt/code/bufferutils.h>
#include <tt/script/ScriptEngine.h>
#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/Entity.h>
#include <toki/game/entity/EntityTiles.h>
#include <toki/game/script/Registry.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/serialization/utils.h>


SUITE(Serialization)
{

// ------------------------------------------------------------------------------------------------
// Squirrel

/*! \brief Fixture to setup VM and load some test scripts. */
struct SquirrelFixture
{
	tt::script::VirtualMachinePtr vmPtr;
	std::string testScript;
	
	SquirrelFixture()
	:
	vmPtr(tt::script::ScriptEngine::createVM("")),
	testScript("\n"
	           "class A\n"
	           "{\n"
	           "	m_string = \"hoi hoi\";\n"
	           "	m_bool = true;\n"
	           "	m_array = [1, 2, \"sdd\"];\n"
	           "	m_table = {a = 2, b = \"dfdf\", c = null};\n"
	           "	m_int = 10;\n"
	           "	m_tableRef = null;\n"
	           "	m_arrayRef = null;\n"
	           "	m_stringRef = null;\n"
	           "	m_boolRef = null;\n"
	           "	\n"
	           "	function init()\n"
	           "	{\n"
	           "		m_array.append(m_array);\n"
	           "		m_array.append(m_table);\n"
	           "		m_table[\"c\"] = m_array;\n"
	           "		m_arrayRef = m_array;\n"
	           "		m_tableRef = m_table;\n"
	           "		m_stringRef = m_string;\n"
	           "		m_boolRef = m_bool;\n"
	           "		m_stringRef = \"fdfdf\";\n"
	           "	}\n"
	           "}\n"
	           "\n"
	           "class B\n"
	           "{\n"
	           "	m_bool = false;\n"
	           "	m_instance = A();\n"
	           "}\n")
	{
		TT_NULL_ASSERT(vmPtr);
		bool result = vmPtr->runScriptBuf(testScript);
		TT_ASSERT(result);
	}
	
	bool runInit()
	{
		return vmPtr->runScriptBuf("local testB = B();\n"
		                           "testB.m_instance.init();\n");
	}
};


TEST_FIXTURE( SquirrelFixture, Squirrel )
{
	CHECK(runInit());
	
	// Serialize VM.
	// Clear VM
	// Unserialize VM.
	// Check result.
}


// ------------------------------------------------------------------------------------------------
// Registry Tests

/*! \brief Fixture to setup Registery and create a map with some test values. */
struct RegistryFixture
{
	typedef std::map<std::string, std::string> TestValues;
	TestValues testValues;
	
	toki::serialization::SerializationMgr serializationMgr;
	
	
	RegistryFixture()
	{
		// Prepare the Registry for use (by clearing it)
		/*
		toki::game::script::Registry::clear();
		
		testValues["keyA"] = "valueA";
		testValues["keyB"] = "valueB";
		testValues["keyC"] = "valueC";
		testValues["keyD"] = "valueD";
		*/
	}
};

TEST_FIXTURE( RegistryFixture, Registry )
{
	// Add some test values to the registry
	/*
	for (TestValues::iterator it = testValues.begin(); it != testValues.end(); ++it)
	{
		toki::game::script::Registry::set((*it).first, (*it).second);
	}

	
	// Serialize the registry
	toki::game::script::Registry::serialize(serializationMgr);
	
	// Clear the registry (pretend all was reset)
	toki::game::script::Registry::clear();
	
	// Unserialize registry (restore from data)
	toki::game::script::Registry::unserialize(serializationMgr);
	
	// Verify all values were restored (and also verify that this is the ONLY data that was restored)
	const size_t testValueCount = static_cast<size_t>(testValues.size());
	CHECK_EQUAL(testValueCount, toki::game::script::Registry::size());
	
	for (TestValues::iterator it = testValues.begin(); it != testValues.end(); ++it)
	{
		const std::string& key((*it).first);
		const std::string& value((*it).second);
		
		CHECK(toki::game::script::Registry::exists(key));
		CHECK_EQUAL(value, toki::game::script::Registry::get(key));
		
		toki::game::script::Registry::erase(key);
	}
	
	CHECK_EQUAL(0u, toki::game::script::Registry::size());
	*/
}


// ------------------------------------------------------------------------------------------------
// Entity

struct EntityFixture
{
	const toki::game::entity::Entity::CreationParams entityCreationParams;
	const toki::game::entity::EntityHandle           entityHandle;
	toki::game::entity::Entity                       entity;
	tt::code::AutoGrowBufferPtr                      autoGrowBuffer;
	
	
	EntityFixture()
	:
	entityCreationParams(),
	entityHandle(toki::game::entity::EntityHandle::createForTesting(0, 1)),
	entity(entityCreationParams, entityHandle),
	autoGrowBuffer(tt::code::AutoGrowBuffer::create(256, 256))
	{
		// Set the inverse of the default values. (Because we run load on a default constructed entity.)
		entity.setDetectableByLight(entity.isDetectableByLight() == false);
		entity.setDetectableBySight(entity.isDetectableBySight() == false);
		entity.setDetectableByTouch(entity.isDetectableByTouch() == false);
		entity.setFlowToTheRight( entity.flowToTheRight()    == false);
	}
	
private:
	EntityFixture(const EntityFixture&);                  // No copy
	const EntityFixture& operator=(const EntityFixture&); // No assigment
};


TEST_FIXTURE( EntityFixture, Entity )
{
	using namespace toki::game::entity;
	
	namespace bu = tt::code::bufferutils;
	
	{
		Entity copy = entity;
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		bu::putHandle(copy.getHandle(), &context);
		copy.serializeCreationParams(&context);
		copy.serialize(&context);
		context.flush();
		
		copy.invalidateTempCopy();
	}
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		EntityHandle handle = bu::getHandle<Entity>(&context);
		Entity::CreationParams params(Entity::unserializeCreationParams(&context));
		Entity scratchEntity(params, handle);
		
		scratchEntity.unserialize(&context);
		
		CHECK_EQUAL(entity.getHandle()        , scratchEntity.getHandle()        );
		CHECK_EQUAL(entity.getPosition()      , scratchEntity.getPosition()      );
		CHECK_EQUAL(entity.isInitialized()    , scratchEntity.isInitialized()    );
		CHECK_EQUAL(entity.isDetectableByLight(), scratchEntity.isDetectableByLight());
		CHECK_EQUAL(entity.isInLight()        , scratchEntity.isInLight()        );
		CHECK_EQUAL(entity.isDetectableBySight(), scratchEntity.isDetectableBySight());
		CHECK_EQUAL(entity.isDetectableByTouch(), scratchEntity.isDetectableByTouch());
		CHECK_EQUAL(entity.flowToTheRight()   , scratchEntity.flowToTheRight()   );
	}
}


// ------------------------------------------------------------------------------------------------
// EntityTiles

struct EntityTilesFixture
{
	tt::code::AutoGrowBufferPtr            autoGrowBuffer;
	
	toki::game::entity::EntityTilesPtr     nullTiles;
	toki::game::entity::EntityTilesPtr     testTiles;
	const std::string                      tileString;
	const tt::math::Point2                 tilePos;
	const bool                             tilesAreActive;
	const toki::game::entity::EntityHandle ownerHandle;
	
	
	EntityTilesFixture()
	:
	autoGrowBuffer(tt::code::AutoGrowBuffer::create(256, 256)),
	nullTiles(),
	testTiles(),
	tileString("ssss\nssss\nssss\nssss"),
	tilePos(5, 3),
	tilesAreActive(true),
	ownerHandle(toki::game::entity::EntityHandle::createForTesting(0, 1))
	{
		testTiles = toki::game::entity::EntityTiles::create(
				tileString, tilePos, tilesAreActive, ownerHandle);
	}
	
private:
	EntityTilesFixture(const EntityTilesFixture&);                  // No copy
	const EntityTilesFixture& operator=(const EntityTilesFixture&); // No assigment
};


TEST_FIXTURE( EntityTilesFixture, EntityTiles )
{
	using namespace toki::game::entity;
	
	namespace bu = tt::code::bufferutils;
	
	// First ensure the test EntityTiles instance has the values we expect
	CHECK_EQUAL(EntityTilesPtr(), nullTiles);
	
	CHECK_EQUAL(4,                    testTiles->getWidth());
	CHECK_EQUAL(4,                    testTiles->getHeight());
	CHECK_EQUAL(tilePos,              testTiles->getPos());
	CHECK_EQUAL(tilesAreActive,       testTiles->isActive());
	CHECK_EQUAL(ownerHandle,          testTiles->getOwner());
	
	// Serialize
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		
		EntityTiles::serialize(nullTiles, &context);
		EntityTiles::serialize(testTiles, &context);
		
		context.flush();
	}
	
	// Unserialize
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		
		EntityTilesPtr unserializedNullTiles = EntityTiles::unserialize(&context);
		EntityTilesPtr unserializedTestTiles = EntityTiles::unserialize(&context);
		
		CHECK_EQUAL(EntityTilesPtr(), unserializedNullTiles);
		
		CHECK_EQUAL(testTiles->getWidth(),                    unserializedTestTiles->getWidth());
		CHECK_EQUAL(testTiles->getHeight(),                   unserializedTestTiles->getHeight());
		CHECK_EQUAL(testTiles->getPos(),                      unserializedTestTiles->getPos());
		CHECK_EQUAL(testTiles->isActive(),                    unserializedTestTiles->isActive());
		CHECK_EQUAL(testTiles->getOwner(),                    unserializedTestTiles->getOwner());
	}
}


// ------------------------------------------------------------------------------------------------
// Serialization Section Save IDs

TEST( SerializationSectionSaveIDs )
{
	// Unit test to ensure the serialization Section to "save ID" mapping stays valid
	
	typedef std::set<u32> SaveIDs;
	SaveIDs saveIDs;
	
	using namespace toki;
	
	for (s32 i = 0; i < serialization::Section_Count; ++i)
	{
		const serialization::Section section = static_cast<serialization::Section>(i);
		
		// Must be a valid enum value
		CHECK(serialization::isValid(section));
		
		// Ensure this section has a corresponding save ID (getSectionSaveID will panic if it has no save ID)
		const u32 sectionSaveID = serialization::getSectionSaveID(section);
		CHECK(sectionSaveID != 0);
		
		// Ensure there are no duplicate save IDs for sections
		CHECK(saveIDs.find(sectionSaveID) == saveIDs.end());
		saveIDs.insert(sectionSaveID);
		
		// Also ensure that translating the save ID back to a Section enum value yields the correct result
		const serialization::Section sectionFromSaveID = serialization::getSectionFromSaveID(sectionSaveID);
		CHECK_EQUAL(section, sectionFromSaveID);
	}
}


// ------------------------------------------------------------------------------------------------
// Tags


TEST( Tags )
{
	using namespace toki::game::entity;
	
	namespace bu = tt::code::bufferutils;
	
	tt::code::AutoGrowBufferPtr autoGrowBuffer(tt::code::AutoGrowBuffer::create(256, 256));
	
	tt::pres::Tags testTags;
	testTags.insert(tt::pres::Tag("Hallo"));
	testTags.insert(tt::pres::Tag("Dit"));
	testTags.insert(tt::pres::Tag("is_een_test"));
	
	{
		tt::code::BufferWriteContext context = autoGrowBuffer->getAppendContext();
		toki::serialization::serializePresTags(testTags, &context);
		context.flush();
	}
	
	{
		tt::code::BufferReadContext context = autoGrowBuffer->getReadContext();
		tt::pres::Tags loadedTags = toki::serialization::unserializePresTags(&context);
		
		CHECK_EQUAL(testTags.size(), loadedTags.size());
		
		tt::pres::Tags::const_iterator testIt = testTags.begin();
		tt::pres::Tags::const_iterator loadIt = loadedTags.begin();
		for (;
		     loadIt != loadedTags.end() && testIt != testTags.end();
		     ++testIt, ++loadIt)
		{
			CHECK_EQUAL((*testIt).getValue(), (*loadIt).getValue());
			CHECK_EQUAL((*testIt).getName(),  (*loadIt).getName());
		}
		CHECK(loadIt == loadedTags.end());
		CHECK(testIt == testTags.end());
	}
}


// ------------------------------------------------------------------------------------------------
// TimerMgr

/*! \brief Fixture to setup TimerMgr. */
struct TimerMgrFixture
{
	TimerMgrFixture()
	{
		
	}
};

TEST_FIXTURE( TimerMgrFixture, TimerMgr )
{
	
}

// Suite end
};


#endif // !defined(TT_INC_TOKI_UNITTEST_SERIALIZATION_UNITTESTS_H)
