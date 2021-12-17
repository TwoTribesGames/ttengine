#include <tt/code/bufferutils.h>
#include <tt/math/ExponentialGrowth.h>

#include <toki/game/entity/effect/EffectMgr.h>
#include <toki/game/entity/effect/EffectRect.h>
#include <toki/game/entity/effect/ColorGradingEffectMgr.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/Game.h>
#include <toki/serialization/SerializationMgr.h>
#include <toki/AppGlobal.h>
#include <toki/cfg.h>


namespace toki {
namespace game {
namespace entity {
namespace effect {


template <class Type>
void serializeRectEffects(const std::vector<Type>& p_rectWithEffects,
                          tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const u32 count = static_cast<u32>(p_rectWithEffects.size());
	bu::put(count, p_context);
	for (typename std::vector<Type>::const_iterator it = p_rectWithEffects.begin();
	     it != p_rectWithEffects.end(); ++it)
	{
		it->serialize(p_context);
	}
}


template <class Type>
void unserializeRectEffects(std::vector<Type>& p_rectWithEffects,
                            tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	const u32 count = bu::get<u32>(p_context);
	p_rectWithEffects.clear();
	p_rectWithEffects.resize(count);
	for (u32 i = 0; i < count; ++i)
	{
		p_rectWithEffects[i].unserialize(p_context);
	}
}


//--------------------------------------------------------------------------------------------------
// Public member functions

EffectMgr::EffectMgr()
:
m_fogColors(),
m_fogColor(tt::engine::renderer::ColorRGB::gray),
m_fogColorStrength(0.0f),
m_fogNearFars(),
m_fogNear(0.0f),
m_fogFar(0.0f),
m_fogNearFarStrength(0.0f),
m_cameraOffsets(),
m_cameraOffset(tt::math::Vector2::zero),
m_drcCameraOffsets(),
m_drcCameraOffset(tt::math::Vector2::zero),
m_cameraPositions(),
m_cameraPosition(tt::math::Vector2::zero),
m_cameraPositionStrength(0.0f),
m_cameraPositionStrengthTarget(0.0f),
m_drcCameraPositions(),
m_drcCameraPosition(tt::math::Vector2::zero),
m_drcCameraPositionStrength(0.0f),
m_drcCameraPositionStrengthTarget(0.0f),
m_camPosDecayHandle(cfg()->getHandleReal("toki.effects.camera_position.decay")),
m_lightAmbients(),
m_lightAmbient(0.0f),
m_lightAmbientStrength(0.0f),
m_cameraFoVAmbients(),
m_cameraFoVAmbient(0.0f),
m_cameraFoVAmbientStrength(0.0f),
m_drcCameraFoVAmbients(),
m_drcCameraFoVAmbient(0.0f),
m_drcCameraFoVAmbientStrength(0.0f),
m_colorGradings()
{
}


void EffectMgr::update(real p_elapsedTime)
{
	(void) p_elapsedTime;
	
	// Fog Color
	m_fogColorStrength = 0.0f;
	m_fogColor         = tt::engine::renderer::ColorRGB::gray;
	for (RectWithFogColors::iterator it = m_fogColors.begin(); it != m_fogColors.end();)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			it = m_fogColors.erase(it);
			continue;
		}
		const real strength = rect->getEffectStrength() * it->baseStrength;
		
		if (strength > 0.0f)
		{
			m_fogColorStrength += strength;
			TT_ASSERT(m_fogColorStrength > 0.0f);
			const real normalized = (strength / m_fogColorStrength);
			m_fogColor = tt::engine::renderer::ColorRGB::mix(m_fogColor, it->color, normalized);
		}
		
		++it;
	}
	
	// Fog Near / Far
	m_fogNearFarStrength = 0.0f;
	m_fogNear            = 0.0f;
	m_fogFar             = 0.0f;
	for (RectWithFogNearFars::iterator it = m_fogNearFars.begin(); it != m_fogNearFars.end();)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			it = m_fogNearFars.erase(it);
			continue;
		}
		const real strength = rect->getEffectStrength() * it->baseStrength;
		
		if (strength > 0.0f)
		{
			m_fogNearFarStrength += strength;
			TT_ASSERT(m_fogNearFarStrength > 0.0f);
			const real normalized = (strength / m_fogNearFarStrength);
			const real oneMinus   = 1.0f - normalized;
			
			m_fogNear = (m_fogNear * oneMinus) + it->fogNear * normalized;
			m_fogFar  = (m_fogFar  * oneMinus) + it->fogFar  * normalized;
		}
		
		++it;
	}
	
	// Camera offset
	{
		real totalStrenth = 0.0f;
		m_cameraOffset    = getEffectValue(m_cameraOffsets,    totalStrenth);
		m_cameraOffset    = mixEffect(tt::math::Vector2::zero, m_cameraOffset, totalStrenth);
		m_drcCameraOffset = getEffectValue(m_drcCameraOffsets, totalStrenth);
		m_drcCameraOffset = mixEffect(tt::math::Vector2::zero, m_drcCameraOffset, totalStrenth);
	}
	
	// Camera Position
	{
		// Config decay as some value which is applied at 60 FPS.
		// Note: Don't replace with fixedFrameRate which is changed in 30 FPS mode.
		// (See: ExponentialGrowth.h)
		static const tt::math::ExponentialGrowth decayHelper(cfg()->get(m_camPosDecayHandle), 1.0f / 60.0f);
		const real decay = decayHelper.getGrowth(p_elapsedTime);
		
		m_cameraPosition    = getEffectValue(m_cameraPositions   , m_cameraPositionStrengthTarget   , m_cameraPosition);
		{
			const real diff = m_cameraPositionStrengthTarget - m_cameraPositionStrength; 
			m_cameraPositionStrength += diff * decay;
		}
		
		m_drcCameraPosition = getEffectValue(m_drcCameraPositions, m_drcCameraPositionStrengthTarget, m_drcCameraPosition);
		{
			const real diff = m_drcCameraPositionStrengthTarget - m_drcCameraPositionStrength;
			m_drcCameraPositionStrength += diff * decay;
		}
	}
	
	// Light Ambient
	getEffectValues(m_lightAmbients       , m_lightAmbient       , m_lightAmbientStrength       );
	getEffectValues(m_cameraFoVAmbients   , m_cameraFoVAmbient   , m_cameraFoVAmbientStrength   );
	getEffectValues(m_drcCameraFoVAmbients, m_drcCameraFoVAmbient, m_drcCameraFoVAmbientStrength);
	
	ColorGradingEffectMgr& colorGradingMgr = AppGlobal::getGame()->getColorGradingEffectMgr();
	
	for (RectWithEffectColorGrading::iterator it = m_colorGradings.begin(); it != m_colorGradings.end();)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			it = m_colorGradings.erase(it);
			continue;
		}
		const real strength = rect->getEffectStrength() * it->baseStrength;
		colorGradingMgr.setEffectStrength(it->colorGradingIndex, strength);
		
		++it;
	}
}


tt::engine::renderer::ColorRGB EffectMgr::getFogColor(const tt::engine::renderer::ColorRGB& p_defaultColor) const
{
	if (m_fogColorStrength >= 0.0f)
	{
		if (m_fogColorStrength >= 1.0f)
		{
			return m_fogColor;
		}
		return tt::engine::renderer::ColorRGB::mix(p_defaultColor, m_fogColor, m_fogColorStrength);
	}
	else
	{
		return p_defaultColor;
	}
}


real EffectMgr::getFogNear(real p_defaultNear) const
{
	if (m_fogNearFarStrength >= 0.0f)
	{
		if (m_fogNearFarStrength >= 1.0f)
		{
			return m_fogNear;
		}
		return p_defaultNear * (1 - m_fogNearFarStrength) + m_fogNear * m_fogNearFarStrength;
	}
	else
	{
		return p_defaultNear;
	}
}


real EffectMgr::getFogFar(real p_defaultFar) const
{
	if (m_fogNearFarStrength >= 0.0f)
	{
		if (m_fogNearFarStrength >= 1.0f)
		{
			return m_fogFar;
		}
		return p_defaultFar * (1 - m_fogNearFarStrength) + m_fogFar * m_fogNearFarStrength;
	}
	else
	{
		return p_defaultFar;
	}
}


tt::math::Vector2 EffectMgr::getCameraPosition(   const tt::math::Vector2 p_position) const
{
	return mixEffect(p_position, m_cameraPosition   , m_cameraPositionStrength   );
}


tt::math::Vector2 EffectMgr::getDrcCameraPosition(const tt::math::Vector2 p_position) const
{
	return mixEffect(p_position, m_drcCameraPosition, m_drcCameraPositionStrength);
}


real EffectMgr::getLightAmbient(real p_defaultAmbient) const
{
	return mixEffect(p_defaultAmbient, m_lightAmbient, m_lightAmbientStrength);
}


real EffectMgr::getCameraFov(real p_defaultFov) const
{
	return mixEffect(p_defaultFov, m_cameraFoVAmbient, m_cameraFoVAmbientStrength);
}


real EffectMgr::getDrcCameraFov(real p_defaultFov) const
{
	return mixEffect(p_defaultFov, m_drcCameraFoVAmbient, m_drcCameraFoVAmbientStrength);
}


void EffectMgr::reset()
{
	m_fogColors.clear();
	m_fogNearFars.clear();
	m_cameraOffsets.clear();
	m_drcCameraOffsets.clear();
	m_lightAmbients.clear();
	m_cameraFoVAmbients.clear();
	m_drcCameraFoVAmbients.clear();
	m_colorGradings.clear();
}


void EffectMgr::addFogColor(const EffectRectHandle&               p_handle,
                            real                                  p_baseStrength,
                            const tt::engine::renderer::ColorRGB& p_color)
{
	m_fogColors.push_back(EffectFogColor(p_handle, p_baseStrength, p_color));
}


void EffectMgr::addFogNearFar(const EffectRectHandle& p_handle, real p_baseStrength,
                              real p_near, real p_far)
{
	m_fogNearFars.push_back(EffectFogNearFar(p_handle, p_baseStrength, p_near, p_far));
}


void EffectMgr::addCameraOffset(const EffectRectHandle& p_handle, real p_baseStrength,
                                const tt::math::Vector2& p_offset)
{
	m_cameraOffsets.push_back(EffectVector2(p_handle, p_baseStrength, p_offset));
}


void EffectMgr::addDrcCameraOffset(const EffectRectHandle& p_handle, real p_baseStrength,
                                   const tt::math::Vector2& p_position)
{
	m_drcCameraOffsets.push_back(EffectVector2(p_handle, p_baseStrength, p_position));
}


void EffectMgr::addCameraPosition(const EffectRectHandle& p_handle, real p_baseStrength,
                                  const tt::math::Vector2& p_position)
{
	m_cameraPositions.push_back(EffectVector2(p_handle, p_baseStrength, p_position));
}


void EffectMgr::addDrcCameraPosition(const EffectRectHandle& p_handle, real p_baseStrength,
                                     const tt::math::Vector2& p_offset)
{
	m_drcCameraPositions.push_back(EffectVector2(p_handle, p_baseStrength, p_offset));
}


void EffectMgr::addLightAmbient(const EffectRectHandle& p_handle, real p_baseStrength, real p_ambient)
{
	m_lightAmbients.push_back(EffectReal(p_handle, p_baseStrength, p_ambient));
}


void EffectMgr::addCameraFov(const EffectRectHandle& p_handle, real p_baseStrength, real p_fov)
{
	m_cameraFoVAmbients.push_back(EffectReal(p_handle, p_baseStrength, p_fov));
}


void EffectMgr::addDrcCameraFov(const EffectRectHandle& p_handle, real p_baseStrength, real p_fov)
{
	m_drcCameraFoVAmbients.push_back(EffectReal(p_handle, p_baseStrength, p_fov));
}


void EffectMgr::addColorGrading(const EntityHandle& p_entityForErrorReporting,
                                const EffectRectHandle& p_handle,
                                real p_baseStrength,
                                const tt::engine::renderer::TexturePtr& p_texture)
{
	EffectRect* newRect = p_handle.getPtr();
	if (newRect == 0)
	{
		TT_PANIC("Can't add color grading when rect is null");
		return;
	}
	
	for (RectWithEffectColorGrading::iterator it = m_colorGradings.begin(); it != m_colorGradings.end(); ++it)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			continue;
		}
		if (newRect->intersects(*rect))
		{
			AppGlobal::getGame()->editorWarning(p_entityForErrorReporting, "The SoftRects used by Color Grading must not overlap!");
			return;
		}
	}
	
	m_colorGradings.push_back(EffectColorGrading(p_handle, p_baseStrength, p_texture));
}


void EffectMgr::serialize  (tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	serializeRectEffects(m_fogColors, p_context);
	bu::put(m_fogColor,         p_context);
	bu::put(m_fogColorStrength, p_context);
	
	serializeRectEffects(m_fogNearFars, p_context);
	bu::put(m_fogNear           , p_context);
	bu::put(m_fogFar            , p_context);
	bu::put(m_fogNearFarStrength, p_context);
	
	serializeRectEffects(m_cameraOffsets, p_context);
	bu::put(m_cameraOffset   , p_context);
	
	serializeRectEffects(m_drcCameraOffsets, p_context);
	bu::put(m_drcCameraOffset, p_context);
	
	serializeRectEffects(m_cameraPositions,    p_context);
	bu::put(m_cameraPosition,                  p_context); // tt::math::Vector2
	bu::put(m_cameraPositionStrength,          p_context); // real
	bu::put(m_cameraPositionStrengthTarget,    p_context); // real
	serializeRectEffects(m_drcCameraPositions, p_context);
	bu::put(m_drcCameraPosition,               p_context); // tt::math::Vector2
	bu::put(m_drcCameraPositionStrength,       p_context); // real
	bu::put(m_drcCameraPositionStrengthTarget, p_context); // real
	
	serializeRectEffects(m_lightAmbients,p_context);
	bu::put(m_lightAmbient,         p_context);
	bu::put(m_lightAmbientStrength, p_context);
	
	serializeRectEffects(m_cameraFoVAmbients,         p_context);
	bu::put(m_cameraFoVAmbient,            p_context);
	bu::put(m_cameraFoVAmbientStrength,    p_context);
	
	serializeRectEffects(m_drcCameraFoVAmbients,      p_context);
	bu::put(m_drcCameraFoVAmbient,         p_context);
	bu::put(m_drcCameraFoVAmbientStrength, p_context);
	
	serializeRectEffects(m_colorGradings,  p_context);
}


void EffectMgr::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	unserializeRectEffects(m_fogColors, p_context);
	m_fogColor         = bu::get<tt::engine::renderer::ColorRGB>(p_context);
	m_fogColorStrength = bu::get<real                          >(p_context);
	
	unserializeRectEffects(m_fogNearFars, p_context);
	m_fogNear            = bu::get<real>(p_context);
	m_fogFar             = bu::get<real>(p_context);
	m_fogNearFarStrength = bu::get<real>(p_context);
	
	unserializeRectEffects(m_cameraOffsets, p_context);
	m_cameraOffset    = bu::get<tt::math::Vector2>(p_context);
	
	unserializeRectEffects(m_drcCameraOffsets, p_context);
	m_drcCameraOffset = bu::get<tt::math::Vector2>(p_context);
	
	unserializeRectEffects(m_cameraPositions,                p_context);
	m_cameraPosition               = bu::get<tt::math::Vector2>(p_context);
	m_cameraPositionStrength       = bu::get<real             >(p_context);
	m_cameraPositionStrengthTarget = bu::get<real             >(p_context);
	unserializeRectEffects(m_drcCameraPositions,             p_context);
	m_drcCameraPosition               = bu::get<tt::math::Vector2>(p_context);
	m_drcCameraPositionStrength       = bu::get<real             >(p_context);
	m_drcCameraPositionStrengthTarget = bu::get<real             >(p_context);
	
	unserializeRectEffects(m_lightAmbients, p_context);
	m_lightAmbient         = bu::get<real>(p_context);
	m_lightAmbientStrength = bu::get<real>(p_context);
	
	unserializeRectEffects(m_cameraFoVAmbients, p_context);
	m_cameraFoVAmbient            = bu::get<real>(p_context);
	m_cameraFoVAmbientStrength    = bu::get<real>(p_context);
	
	unserializeRectEffects(m_drcCameraFoVAmbients, p_context);
	m_drcCameraFoVAmbient         = bu::get<real>(p_context);
	m_drcCameraFoVAmbientStrength = bu::get<real>(p_context);
	
	unserializeRectEffects(m_colorGradings, p_context);
}


//--------------------------------------------------------------------------------------------------
// Private member functions


tt::math::Vector2 EffectMgr::getEffectValue(RectWithEffectVector2& p_collection,
                                            real&                  p_effectStrength_OUT,
                                            const tt::math::Vector2& p_value)
{
	p_effectStrength_OUT    = 0.0f;
	tt::math::Vector2 value = p_value;
	for (RectWithEffectVector2::iterator it = p_collection.begin(); it != p_collection.end();)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			it = p_collection.erase(it);
			continue;
		}
		const real strength = rect->getEffectStrength() * it->baseStrength;
		
		if (strength > 0.0f)
		{
			p_effectStrength_OUT += strength;
			TT_ASSERT(p_effectStrength_OUT > 0.0f);
			const real normalized = (strength / p_effectStrength_OUT);
			value = (value * ( 1.0f - normalized)) + it->value * normalized;
		}
		
		++it;
	}
	
	return value;
}


tt::math::Vector2 EffectMgr::mixEffect(const tt::math::Vector2& p_defaultValue,
                                       const tt::math::Vector2& p_effectValue,
                                       real                     p_effectStrength)
{
	if (p_effectStrength >= 0.0f)
	{
		if (p_effectStrength >= 1.0f)
		{
			return p_effectValue;
		}
		return p_defaultValue * (1 - p_effectStrength) + p_effectValue * p_effectStrength;
	}
	else
	{
		return p_defaultValue;
	}
}


void EffectMgr::getEffectValues(RectWithEffectReal& p_rects, real& p_effect, real& p_effectStrength)
{
	p_effect         = 0.0f;
	p_effectStrength = 0.0f;
	for (RectWithEffectReal::iterator it = p_rects.begin(); it != p_rects.end();)
	{
		EffectRect* rect = it->rectHandle.getPtr();
		if (rect == 0)
		{
			it = p_rects.erase(it);
			continue;
		}
		const real strength = rect->getEffectStrength() * it->baseStrength;
		
		if (strength > 0.0f)
		{
			p_effectStrength += strength;
			TT_ASSERT(p_effectStrength > 0.0f);
			const real normalized = (strength / p_effectStrength);
			const real oneMinus   = 1.0f - normalized;
			
			p_effect = (p_effect * oneMinus) + it->value * normalized;
		}
		
		++it;
	}
}


real EffectMgr::mixEffect(real p_defaultValue, real p_effectValue, real p_effectStrength)
{
	if (p_effectStrength >= 0.0f)
	{
		if (p_effectStrength >= 1.0f)
		{
			return p_effectValue;
		}
		return p_defaultValue * (1 - p_effectStrength) + p_effectValue * p_effectStrength;
	}
	else
	{
		return p_defaultValue;
	}
}


void EffectMgr::EffectBase::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::putHandle(rectHandle, p_context);
	bu::put(baseStrength, p_context);
}


void EffectMgr::EffectBase::unserialize(tt::code::BufferReadContext*  p_context)
{
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	rectHandle   = bu::getHandle<EffectRect>(p_context);
	baseStrength = bu::get<real            >(p_context);
}


void EffectMgr::EffectFogColor::serialize(tt::code::BufferWriteContext* p_context) const
{
	EffectBase::serialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::put(color, p_context);
}


void EffectMgr::EffectFogColor::unserialize(tt::code::BufferReadContext*  p_context)
{
	EffectBase::unserialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	color = bu::get<tt::engine::renderer::ColorRGB>(p_context);
}


void EffectMgr::EffectFogNearFar::serialize(tt::code::BufferWriteContext* p_context) const
{
	EffectBase::serialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::put(fogNear, p_context);
	bu::put(fogFar , p_context);
}


void EffectMgr::EffectFogNearFar::unserialize(tt::code::BufferReadContext*  p_context)
{
	EffectBase::unserialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	fogNear = bu::get<real>(p_context);
	fogFar  = bu::get<real>(p_context);
}


void EffectMgr::EffectVector2::serialize(tt::code::BufferWriteContext* p_context) const
{
	EffectBase::serialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::put(value, p_context);
}


void EffectMgr::EffectVector2::unserialize(tt::code::BufferReadContext*  p_context)
{
	EffectBase::unserialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	value = bu::get<tt::math::Vector2>(p_context);
}


void EffectMgr::EffectReal::serialize(tt::code::BufferWriteContext* p_context) const
{
	EffectBase::serialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::put(value, p_context);
}


void EffectMgr::EffectReal::unserialize(tt::code::BufferReadContext*  p_context)
{
	EffectBase::unserialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	value = bu::get<real>(p_context);
}

EffectMgr::EffectColorGrading::EffectColorGrading(const EffectRectHandle& p_rectHandle, real p_baseStrength, const tt::engine::renderer::TexturePtr& p_texture)
:
EffectBase(p_rectHandle, p_baseStrength),
colorGradingIndex(AppGlobal::getGame()->getColorGradingEffectMgr().createEffect(p_texture, 0.0f))
{
}


void EffectMgr::EffectColorGrading::serialize(tt::code::BufferWriteContext* p_context) const
{
	EffectBase::serialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	bu::put(colorGradingIndex, p_context);
}


void EffectMgr::EffectColorGrading::unserialize(tt::code::BufferReadContext*  p_context)
{
	EffectBase::unserialize(p_context);
	
	TT_NULL_ASSERT(p_context);
	namespace bu = tt::code::bufferutils;
	colorGradingIndex = bu::get<s32>(p_context);
}


// Namespace end
}
}
}
}
