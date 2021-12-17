#if !defined(TT_INC_TOKI_UNITTEST_SQUIRREL_COMPILE_UNITTESTS_H)
#define TT_INC_TOKI_UNITTEST_SQUIRREL_COMPILE_UNITTESTS_H

#include <unittestpp/unittestpp.h>

#include <tt/fs/fs.h>
#include <tt/fs/StdFileSystem.h>
#include <tt/script/ScriptEngine.h>
#include <tt/script/VirtualMachine.h>


SUITE(SquirrelCompile)
{

// ------------------------------------------------------------------------------------------------
// Squirrel

/*! \brief Fixture to setup VM and load some test scripts. */
struct SquirrelFixture
{
	tt::script::VirtualMachinePtr vmPtr;
	std::string testScript;
	std::string otherScript;
	
	// FS setup
	const tt::fs::identifier    stdFSID;
	const tt::fs::FileSystemPtr stdFSPtr;
	
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
	           "}\n"
	           "\n"
	           "function test()\n"
	           "{\n"
//	           "	print(\"test() is called!\\n\");\n"
//	           "	print(\"test() enumTest:\" + EnumTest.One + \"\\n\");\n"
//	           "	hello();"
	           "}\n"
//	           "hello();\n"
	           "\n"),
	otherScript("\n"
	            "enum EnumTest\n"
	            "{\n"
	            "	One,\n"
	            "	Two,\n"
	            "	Three,\n"
	            "}\n"
	            "function otherTest()\n"
	            "{\n"
//	            "	print(\"otherTest() is called!\\n\");\n"
//	            "	print(\"otherTest() enumTest:\" + EnumTest.One + \"\\n\");\n"
	            "}\n"
	            "\n"),
	stdFSID(0),
	stdFSPtr(tt::fs::StdFileSystem::instantiate(stdFSID))
	{
	}
	
private:
	const SquirrelFixture& operator=(const SquirrelFixture& p_rhs); // Not implemented.
};


TEST_FIXTURE( SquirrelFixture, Squirrel )
{
	TT_NULL_ASSERT(vmPtr);
	HSQUIRRELVM vm = vmPtr->getVM();
	tt::script::SqTopRestorerHelper helper(vm);
	
	// ------- Compile Other script first. This one simulates includes. ----
	if (SQ_FAILED(sq_compilebuffer(vm, otherScript.c_str(), static_cast<int>(otherScript.length()),
	              "other_script_from_buffer", TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Compiling buffer failed!");
	}
	
	// ------- run other script so the enum is added to const table ------
	sq_pushroottable(vm); // Add root table as parameter for sq_call.
	if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
	{
		TT_PANIC("Failed to run closure which was just loaded.");
	}
	CHECK_EQUAL(true, vmPtr->callSqFun("otherTest"));
	
	// ------- Compile Buffer -------
	if (SQ_FAILED(sq_compilebuffer(vm, testScript.c_str(), static_cast<int>(testScript.length()),
	              "script_from_buffer", TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Compiling buffer failed!");
	}
	
	sq_pushroottable(vm); // Add root table as parameter for sq_call.
	if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
	{
		TT_PANIC("Failed to run closure which was just loaded.");
	}
	CHECK_EQUAL(true, vmPtr->callSqFun("test"));
	
	// Because the unittest is run from "code/windows" we put this test file in the "obj" dir.
	const std::string path("obj/squirrel_compile_unit_test.bnut");
	
	// ------ Save ------
	CHECK_EQUAL(true, vmPtr->saveClosure(path));
	sq_poptop(vm);
	
	// ------ Reset ------
	vmPtr->reset();
	
	if (SQ_FAILED(sq_compilebuffer(vm, "function hello() { print(\"hello function\\n\"); } ", static_cast<int>(testScript.length()),
	              "hello_fun_from_buffer", TT_SCRIPT_RAISE_ERROR)))
	{
		TT_PANIC("Compiling buffer failed!");
	}
	sq_pushroottable(vm); // Add root table as parameter for sq_call.
	
	if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
	{
		TT_PANIC("Failed to run closure which was just loaded.");
	}
	
	// ------ Load ------
	CHECK_EQUAL(true, vmPtr->loadClosure(path));
	
	sq_pushroottable(vm); // Add root table as parameter for sq_call.
	
	if (SQ_FAILED(sq_call(vm, 1, SQFalse, SQTrue)))
	{
		TT_PANIC("Failed to run closure which was just loaded.");
	}
	CHECK_EQUAL(true, vmPtr->callSqFun("test"));
}


// Suite end
};


#endif // !defined(TT_INC_TOKI_UNITTEST_SQUIRREL_COMPILE_UNITTESTS_H)
