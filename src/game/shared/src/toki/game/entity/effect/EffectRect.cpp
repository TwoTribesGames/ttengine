#include <tt/code/bufferutils.h>

#if !defined(TT_BUILD_FINAL)
#	include <tt/engine/debug/DebugRenderer.h>
#	include <tt/engine/renderer/Renderer.h>
#	include <tt/str/toStr.h>
#endif

#include <toki/game/entity/effect/EffectRect.h>
#include <toki/game/entity/effect/EffectRectMgr.h>
#include <toki/game/entity/EntityMgr.h>
#include <toki/game/Game.h>
#include <toki/AppGlobal.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {


//--------------------------------------------------------------------------------------------------
// Public member functions

EffectRect::EffectRect(const CreationParams&   p_creationParams,
                       const EffectRectHandle& p_ownHandle)
:
m_ownHandle(p_ownHandle),
m_targetType(p_creationParams.targetType),
m_owner(p_creationParams.owner),
m_offset(0.0f, 0.0f),
m_halfRectSize(1.0f, 1.0f),
m_borderTopRight(1.0f, 1.0f),
m_borderBottomLeft(1.0f, 1.0f),
m_effectStrength(0.0f)
{
	TT_ASSERT(isValidEffectRectTarget(m_targetType));
}


void EffectRect::setSize(const tt::math::Vector2& p_size)
{
	m_halfRectSize   = p_size * 0.5f;
	m_halfRectSize.x = std::abs(m_halfRectSize.x);
	m_halfRectSize.y = std::abs(m_halfRectSize.y);
}


void EffectRect::setBorder(const tt::math::Vector2& p_borderSize)
{
	m_borderTopRight.setValues(std::abs(p_borderSize.x), std::abs(p_borderSize.y));
	m_borderBottomLeft = m_borderTopRight;
}


void EffectRect::update(real p_elapsedTime, const EffectRectContext& p_context)
{
	const entity::Entity* entity = m_owner.getPtr();
	if (entity == 0)
	{
		TT_PANIC("EffectRect found for which the owner is gone!");
		return;
	}
	
	if (m_baseStrengthTime < 1.0f)
	{
		m_baseStrengthTime += (p_elapsedTime * 2);
		if (m_baseStrengthTime >= 1.0f)
		{
			setBaseStrengthInstant(m_baseStrengthEnd);
		}
	}
	const real baseStrength = getCurrentBaseStrength();
	if (baseStrength <= 0.0f)
	{
		m_effectStrength = 0.0f;
		return;
	}
	
	const tt::math::Vector2 pos = entity->getCenterPosition() + m_offset;
	
#if !defined(TT_BUILD_FINAL)
	m_centerPos = pos;
#endif
	
	// Select checkPos based on target type.
	TT_ASSERT(m_targetType == EffectRectTarget_CameraPos ||
	          m_targetType == EffectRectTarget_ControllingEntityPos);
	const tt::math::Vector2 checkPos = (m_targetType == EffectRectTarget_CameraPos) ?
		p_context.cameraPos : p_context.controllingEntityPos;
	
	const tt::math::Vector2 distance(checkPos - pos);
	const tt::math::Vector2 absDistance( std::abs(distance.x), std::abs(distance.y) );
	
	const tt::math::Vector2 outSizeDistance = absDistance - m_halfRectSize;
	if (outSizeDistance.x <= 0.0f &&
	    outSizeDistance.y <= 0.0f )
	{
		m_effectStrength = 1.0f;
	}
	else
	{
		const tt::math::Vector2 border( (distance.x > 0.0f) ? m_borderTopRight.x : m_borderBottomLeft.x,
		                                (distance.y > 0.0f) ? m_borderTopRight.y : m_borderBottomLeft.y);
		if (outSizeDistance.x < border.x &&
		    outSizeDistance.y < border.y)
		{
			const tt::math::Vector2 strength(outSizeDistance.x / border.x,
			                                 outSizeDistance.y / border.y);
			m_effectStrength = 1.0f - std::max(strength.x, strength.y);
			tt::math::clamp(m_effectStrength, 0.0f, 1.0f);
		}
		else
		{
			m_effectStrength = 0.0f;
		}
	}
	
	m_effectStrength *= baseStrength;
}


void EffectRect::renderDebug() const
{
#if !defined(TT_BUILD_FINAL)
	const real baseStrength = getCurrentBaseStrength();
	if (baseStrength <= 0.0f)
	{
		// "Disabled". We don't have a valid position because update has an early out. So can't render.
		return;
	}
	
	using tt::engine::renderer::Renderer;
	using tt::engine::debug::DebugRendererPtr;
	const DebugRendererPtr& debug = Renderer::getInstance()->getDebug();
	
	tt::math::VectorRect rect(       m_centerPos   - m_halfRectSize    , m_centerPos       + m_halfRectSize);
	tt::math::VectorRect outsideRect(rect.getMin() - m_borderBottomLeft, rect.getMaxEdge() + m_borderTopRight);
	
	s32 strength = static_cast<s32>(m_effectStrength * 255.0f);
	tt::math::clamp(strength, s32(0), s32(255));
	const u8 channel = static_cast<u8>(strength);
	tt::engine::renderer::ColorRGBA color(channel, channel, channel, 255);
	switch (m_targetType)
	{
	case EffectRectTarget_CameraPos:            color.r = 255; break;
	case EffectRectTarget_ControllingEntityPos: color.g = 255; break;
	default:
		TT_PANIC("Unknown targetType: %d", m_targetType);
		break;
	}
	debug->renderRect(color, rect);
	color.b = 128;
	debug->renderRect(color, outsideRect);
	
	tt::math::Point2 screenPos(AppGlobal::getGame()->getCamera().worldToScreen(m_centerPos));
	const std::string strengthStr = tt::str::toStr(m_effectStrength);
	debug->renderText(strengthStr, screenPos.x, screenPos.y, color);
#endif
}


bool EffectRect::intersects(const EffectRect& p_rhs) const
{
	const entity::Entity* thisEntity = m_owner.getPtr();
	if (thisEntity == 0)
	{
		TT_PANIC("EffectRect found for which the owner is gone!");
		return false;
	}
	const entity::Entity* otherEntity = p_rhs.m_owner.getPtr();
	if (otherEntity == 0)
	{
		TT_PANIC("EffectRect found for which the owner is gone!");
		return false;
	}
	
	const tt::math::Vector2 thisPos  = thisEntity ->getCenterPosition() +       m_offset;
	const tt::math::Vector2 otherPos = otherEntity->getCenterPosition() + p_rhs.m_offset;
	
	const tt::math::Vector2 distance(otherPos - thisPos);
	const tt::math::Vector2 absDistance( std::abs(distance.x), std::abs(distance.y) );
	
	const tt::math::Vector2 outSizeDistance = absDistance - m_halfRectSize - p_rhs.m_halfRectSize;
	if (outSizeDistance.x <= 0.0f &&
	    outSizeDistance.y <= 0.0f )
	{
		return true;
	}
	else
	{
		const tt::math::Vector2 thisBorder(  (distance.x > 0.0f) ?       m_borderTopRight.x :       m_borderBottomLeft.x,
		                                     (distance.y > 0.0f) ?       m_borderTopRight.y :       m_borderBottomLeft.y);
		const tt::math::Vector2 otherBorder( (distance.x < 0.0f) ? p_rhs.m_borderTopRight.x : p_rhs.m_borderBottomLeft.x,
		                                     (distance.y < 0.0f) ? p_rhs.m_borderTopRight.y : p_rhs.m_borderBottomLeft.y);
		if (outSizeDistance.x < thisBorder.x + otherBorder.x &&
		    outSizeDistance.y < thisBorder.y + otherBorder.y)
		{
			return true;
		}
	}
	
	return false;
}


void EffectRect::serializeCreationParams(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putEnum<u8>(m_targetType, p_context);
	bu::putHandle  (m_owner,      p_context);
}


EffectRect::CreationParams EffectRect::unserializeCreationParams(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	EffectRectTarget     targetType = bu::getEnum  <u8, EffectRectTarget>(p_context);
	entity::EntityHandle owner      = bu::getHandle<entity::Entity      >(p_context);
	
	return CreationParams(targetType, owner);
}


void EffectRect::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::put(m_offset          , p_context);
	bu::put(m_halfRectSize    , p_context);
	bu::put(m_borderTopRight  , p_context);
	bu::put(m_borderBottomLeft, p_context);
	bu::put(m_baseStrengthStart,p_context);
	bu::put(m_baseStrengthEnd , p_context);
	bu::put(m_baseStrengthTime, p_context);
	bu::put(m_effectStrength  , p_context);
}


void EffectRect::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_offset           = bu::get<tt::math::Vector2>(p_context);
	m_halfRectSize     = bu::get<tt::math::Vector2>(p_context);
	m_borderTopRight   = bu::get<tt::math::Vector2>(p_context);
	m_borderBottomLeft = bu::get<tt::math::Vector2>(p_context);
	m_baseStrengthStart= bu::get<real             >(p_context);
	m_baseStrengthEnd  = bu::get<real             >(p_context);
	m_baseStrengthTime = bu::get<real             >(p_context);
	m_effectStrength   = bu::get<real             >(p_context);
}


EffectRect* EffectRect::getPointerFromHandle(const EffectRectHandle& p_handle)
{
	if (AppGlobal::hasGame()                 == false ||
	    AppGlobal::getGame()->hasEntityMgr() == false)
	{
		return 0;
	}
	
	return AppGlobal::getGame()->getEntityMgr().getEffectRectMgr().getEffectRect(p_handle);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}
}
}
