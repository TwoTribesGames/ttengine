#include <squirrel/squirrel.h>

#include <tt/code/helpers.h>
#include <tt/platform/tt_printf.h>
#include <tt/platform/tt_error.h>

#include <tt/script/ScriptEngine.h>
#include <tt/script/VirtualMachine.h>


namespace tt {
namespace script {

ScriptEngine::VirtualMachineContainer   ScriptEngine::ms_virtualMachines;

void ScriptEngine::destroy()
{
	TT_Printf("ScriptEngine::destroy()\n");
	TT_ASSERTMSG(ms_virtualMachines.empty(), "Virtual Machine container was NOT empty! (VMs still referenced by smart pointers)");
	tt::code::helpers::freeContainer(ms_virtualMachines);
}

VirtualMachinePtr ScriptEngine::getVM(HSQUIRRELVM p_vm)
{
	VirtualMachineContainer::iterator it = ms_virtualMachines.find(p_vm);
	if(it != ms_virtualMachines.end())
	{
		VirtualMachinePtr ptr(it->second.lock());
		TT_ASSERTMSG(ptr == 0 || p_vm == ptr->getVM(), "VM of the first and second don't match.");
		return ptr;
	}
	TT_PANIC("Couldn't find p_vm in ScriptEngine!");
	return VirtualMachinePtr();
}


VirtualMachinePtr ScriptEngine::createVM(const std::string& p_rootPath, s32 p_debuggerPort, VMCompileMode p_mode)
{
	VirtualMachine* vm = VirtualMachine::create(p_rootPath, p_debuggerPort, p_mode);
	VirtualMachinePtr vmPtr(vm, remove);
	ms_virtualMachines.insert(std::make_pair(vmPtr->getVM(), vmPtr));
	return vmPtr;
}


void ScriptEngine::remove(VirtualMachine* p_virtualMachine)
{
	if(p_virtualMachine == 0) return;

	if(ms_virtualMachines.empty())
	{
		TT_PANIC("Tried to remove virtual machine from empty cache. Check (static) destruction order!");
		return;
	}

	// Search for this virtual machine in the cache
	VirtualMachineContainer::iterator it = ms_virtualMachines.find(p_virtualMachine->getVM());
	if(it != ms_virtualMachines.end())
	{
		TT_ASSERTMSG(it->second.expired(),
			"Trying to remove a virtual machine that still has references: %p.",
			p_virtualMachine->getVM());

		// Remove from cache
		ms_virtualMachines.erase(it);

		// Free memory
		delete p_virtualMachine;
	}
	else
	{
		TT_PANIC("Cannot find virtual machine in cache: %p",
			p_virtualMachine->getVM());
	}
}

// Namespace end
}
}
