#include <tt/code/bufferutils.h>
#include <tt/math/math.h>
#include <tt/platform/tt_error.h>
#include <tt/script/VirtualMachine.h>


#include <toki/AppGlobal.h>
#include <toki/game/DemoMgr.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/script/ScriptMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {

//--------------------------------------------------------------------------------------------------
// Public member functions


DemoMgr::DemoMgr()
:
m_countdown(0.0f),
m_countdownMinutesLastCallback(0)
{
}


void DemoMgr::init()
{
	m_countdown = cfg()->getRealDirect("toki.demo.duration") * 60.0f;
}


void DemoMgr::update(real p_deltaTime)
{
	if (m_countdownEnabled)
	{
		m_countdown -= p_deltaTime;
	}
	s32 minutesLeft = static_cast<s32>(tt::math::ceil(m_countdown / 60.0f));
	if (minutesLeft != m_countdownMinutesLastCallback)
	{
		m_countdownMinutesLastCallback = minutesLeft;
		if (AppGlobal::isInDemoMode())
		{
			tt::script::VirtualMachinePtr vmPtr = toki::script::ScriptMgr::getVM();
			if (vmPtr->callSqFun("onDemoMinuteChanged",
			                     m_countdownMinutesLastCallback > 0 ?
			                     m_countdownMinutesLastCallback : 0) == false)
			{
				TT_PANIC("onDemoMinuteChanged call failed, resetting demo");
				AppGlobal::resetDemo();
			}
		}
	}
}


void DemoMgr::serialize(serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_DemoMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Demo data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_countdown, &context);
	bu::put(hash(m_countdown), &context);
	
	context.flush();
}


bool DemoMgr::unserialize(const serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_DemoMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the Demo data.");
		return AppGlobal::isInDemoMode() == false;
		// if we're in demo mode and the demo section doesn't exist, someone's messing with the data and we should wipe it.
		// if we're in full mode and the demo section doesn't exist, then we're just on backward compatibility mode - no need to wipe.
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	real countdown = bu::get<real>(&context);  // stick some obfuscation/checksum on this
	u32 checksum = bu::get<u32>(&context);
	
	if (AppGlobal::isInDemoMode())
	{
		if (checksum != hash(countdown))
		{
			return false;
		}
	}
	
	if (countdown < m_countdown)
	{
		m_countdown = countdown;
	}
	
	return true;
}


void DemoMgr::resetCountdown()
{
	m_countdown = cfg()->getRealDirect("toki.demo.duration") * 60.0f;
	m_countdownMinutesLastCallback = 0;
}


//--------------------------------------------------------------------------------------------------
// Private member functions


u32 DemoMgr::hash(real p_value) const
{
	const s32 bufferSize = 8;
	
	u8 buffer[bufferSize];
	real numEnts = static_cast<real>(AppGlobal::getGame()->getEntityMgr().getActiveEntitiesCount()) * 0.42f;
	
	
	u8* bufferPtrPut = buffer;
	size_t bufferSizePut = bufferSize;
	tt::code::bufferutils::put(p_value, bufferPtrPut, bufferSizePut);
	tt::code::bufferutils::put(numEnts, bufferPtrPut, bufferSizePut);
	
	
	const u8* bufferPtrGet = buffer;
	size_t bufferSizeGet = bufferSize;
	u32 a = tt::code::bufferutils::get<u32>(bufferPtrGet, bufferSizeGet);
	u32 b = tt::code::bufferutils::get<u32>(bufferPtrGet, bufferSizeGet);
	
	return (a ^ b);
}


// Namespace end
}
}
