#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>
#include <tt/script/ScriptEngine.h>

#include <toki/game/entity/effect/EffectRect.h>
#include <toki/game/script/wrappers/EffectRectWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

EffectRectWrapper::EffectRectWrapper()
:
m_effectRect()
{
}


EffectRectWrapper::EffectRectWrapper(
		const entity::effect::EffectRectHandle& p_effectRect)
:
m_effectRect(p_effectRect)
{
}


void EffectRectWrapper::setSizeVec(const tt::math::Vector2& p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setSize(p_size);
}


void EffectRectWrapper::setBorderVec(const tt::math::Vector2& p_borderSize)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setBorder(p_borderSize);
}


void EffectRectWrapper::setBorderSize(real p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setBorderSize(p_size);
}


void EffectRectWrapper::setLeftBorder(real p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setLeftBorder(p_size);
}


void EffectRectWrapper::setRightBorder(real p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setRightBorder(p_size);
}


void EffectRectWrapper::setTopBorder(real p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setTopBorder(p_size);
}


void EffectRectWrapper::setBottomBorder(real p_size)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setBottomBorder(p_size);
}


void EffectRectWrapper::setBaseStrengthInstant(real p_strength)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setBaseStrengthInstant(p_strength);
}


void EffectRectWrapper::setBaseStrength(real p_strength)
{
	entity::effect::EffectRect* rect = m_effectRect.getPtr();
	if (rect == 0)
	{
		return;
	}
	rect->setBaseStrength(p_strength);
}


void EffectRectWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_effectRect, p_context);
}


void EffectRectWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_effectRect = bu::getHandle<entity::effect::EffectRect>(p_context);
}


void EffectRectWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(EffectRectWrapper, "EffectRect");
	
	TT_SQBIND_METHOD(EffectRectWrapper, setSizeVec);
	TT_SQBIND_METHOD(EffectRectWrapper, setSize);
	TT_SQBIND_METHOD(EffectRectWrapper, setBorderVec);
	TT_SQBIND_METHOD(EffectRectWrapper, setBorder);
	TT_SQBIND_METHOD(EffectRectWrapper, setBorderSize);
	TT_SQBIND_METHOD(EffectRectWrapper, setLeftBorder);
	TT_SQBIND_METHOD(EffectRectWrapper, setRightBorder);
	TT_SQBIND_METHOD(EffectRectWrapper, setTopBorder);
	TT_SQBIND_METHOD(EffectRectWrapper, setBottomBorder);
	TT_SQBIND_METHOD(EffectRectWrapper, setBaseStrengthInstant);
	TT_SQBIND_METHOD(EffectRectWrapper, setBaseStrength);
}

// Namespace end
}
}
}
}
