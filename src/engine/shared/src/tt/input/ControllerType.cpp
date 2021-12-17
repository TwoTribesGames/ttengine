#include <tt/app/Platform.h>
#include <tt/input/ControllerType.h>
#include <tt/system/Time.h>

namespace tt {
namespace input {

static u64 s_lastUsedTime[ControllerType_Count] = { 0 };

bool           g_currentControllerTypeSet = false;
ControllerType g_currentControllerType    = ControllerType_Invalid;

void updateCurrentControllerType()
{
	if (g_currentControllerTypeSet)
	{
		return;
	}
	
	// The default controller depends on the platform.
	g_currentControllerType = ControllerType_Keyboard;
	u64            lastUsedTime = 1; // Make sure we're higher than s_lastUsedTime's default (0).
	
	for (s32 i = 0; i < ControllerType_Count; ++i)
	{
		ControllerType type = static_cast<ControllerType>(i);
		if (s_lastUsedTime[type] >= lastUsedTime)
		{
			g_currentControllerType = type;
			lastUsedTime            = s_lastUsedTime[type];
		}
	}
}


void setCurrentControllerType(ControllerType p_type)
{
	g_currentControllerType = p_type;
	g_currentControllerTypeSet = true;
}


void resetCurrentControllerType()
{
	g_currentControllerType = ControllerType_Invalid;
	g_currentControllerTypeSet = false;
	updateCurrentControllerType();
}


ControllerType getCurrentControllerType()
{
	return g_currentControllerType;
}


void onControllerTypeUsed(ControllerType p_type)
{
	if (isValidControllerType(p_type))
	{
		s_lastUsedTime[p_type] = system::Time::getInstance()->getSeconds();
	}
}


}
}

