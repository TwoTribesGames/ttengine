#include <tt/script/helpers.h>

#include <toki/game/script/wrappers/ShoeboxPlaneDataWrapper.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


void ShoeboxPlaneDataWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	TT_SQBIND_INIT_NAME(ShoeboxPlaneDataWrapper, "ShoeboxPlaneData");
	TT_SQBIND_SET_CONSTRUCTOR(ShoeboxPlaneDataWrapper, ShoeboxPlaneDataWrapper_constructor);
	
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setPositionXY);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getPositionXY);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setPositionZ);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getPositionZ);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setRotation);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getRotation);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setWidth);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getWidth);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setHeight);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getHeight);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setId);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getId);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTextureFilename);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTextureFilename);
	
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setHidden);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getHidden);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setIgnoreFog);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getIgnoreFog);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexTopLeftU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexTopLeftU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexTopLeftV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexTopLeftV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexTopRightU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexTopRightU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexTopRightV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexTopRightV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexBottomLeftU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexBottomLeftU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexBottomLeftV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexBottomLeftV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexBottomRightU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexBottomRightU);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setTexBottomRightV);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getTexBottomRightV);
	
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, setPriority);
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, getPriority);
	
	/*
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, );
	TT_SQBIND_METHOD(ShoeboxPlaneDataWrapper, );
	
	*/
}


ShoeboxPlaneDataWrapper* ShoeboxPlaneDataWrapper_constructor(HSQUIRRELVM v) 
{
	const SQInteger params = sq_gettop(v) - 1; // Stack has params + context.
	
	if (params != 1)
	{
		TT_PANIC("ShoeboxPlaneDataWrapper incorrect number of parameters. (Got: %d, expected: 2)", params);
		return 0;
	}
	
	const SQChar* textureFileSQ = "";
	if (SQ_FAILED(sq_getstring(v, 2, &textureFileSQ)))
	{
		TT_PANIC("ShoeboxPlaneDataWrapper 1nd parameter is not a string value");
		return 0;
	}
	
	tt::engine::scene2d::shoebox::PlaneData data;
	data.textureFilename = textureFileSQ;
	
	return new ShoeboxPlaneDataWrapper(data);
}

// Namespace end
}
}
}
}
