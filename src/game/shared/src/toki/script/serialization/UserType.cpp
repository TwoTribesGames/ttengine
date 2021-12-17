#include <squirrel/sqbind.h>

#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>

#include <toki/script/serialization/UserType.h>
#include <toki/script/serialization/SQSerializer.h>

// Wrappers
#include <tt/math/Vector2.h>
#include <tt/math/Rect.h>
#include <toki/game/entity/sensor/Shape.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/wrappers/CameraEffectWrapper.h>
#include <toki/game/script/wrappers/DarknessWrapper.h>
#include <toki/game/script/wrappers/EffectRectWrapper.h>
#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/EventWrapper.h>
#include <toki/game/script/wrappers/FluidSettingsWrapper.h>
#include <toki/game/script/wrappers/LightWrapper.h>
#include <toki/game/script/wrappers/MusicTrackWrapper.h>
#include <toki/game/script/wrappers/ParticleEffectWrapper.h>
#include <toki/game/script/wrappers/PhysicsSettingsWrapper.h>
#include <toki/game/script/wrappers/PointerEventWrapper.h>
#include <toki/game/script/wrappers/PowerBeamGraphicWrapper.h>
#include <toki/game/script/wrappers/PresentationObjectWrapper.h>
#include <toki/game/script/wrappers/PresentationStartSettingsWrapper.h>
#include <toki/game/script/wrappers/SensorWrapper.h>
#include <toki/game/script/wrappers/ShapeWrapper.h>
#include <toki/game/script/wrappers/ShoeboxPlaneDataWrapper.h>
#include <toki/game/script/wrappers/SoundCueWrapper.h>
#include <toki/game/script/wrappers/TextureWrapper.h>
#include <toki/game/script/wrappers/TextLabelWrapper.h>
#include <toki/game/script/wrappers/TileSensorWrapper.h>
#include <toki/game/script/EntityBase.h>


namespace toki {
namespace script {
namespace serialization {

//--------------------------------------------------------------------------------------------------
// Public member functions

void UserType::buildCache(Cache& p_cache)
{
	// Cache usertypes
	p_cache[SqBind<tt::engine::renderer::ColorRGB         >::get_typetag()] = Type_ColorRGB;
	p_cache[SqBind<tt::engine::renderer::ColorRGBA        >::get_typetag()] = Type_ColorRGBA;
	p_cache[SqBind<tt::math::Vector2                      >::get_typetag()] = Type_Vector2;
	p_cache[SqBind<tt::math::VectorRect                   >::get_typetag()] = Type_VectorRect;
	p_cache[SqBind<tt::math::interpolation::Easing<real>  >::get_typetag()] = Type_EasingReal;
	
	{
		using namespace toki::game::script;
		
		p_cache[SqBind<EntityBase                         >::get_typetag()] = Type_EntityBase;
		
		//p_cache[SqBind<wrappers::ShapeWrapper             >::get_typetag()] = Type_ShapeWrapper;
		p_cache[SqBind<wrappers::BoxShapeWrapper          >::get_typetag()] = Type_BoxShapeWrapper;
		p_cache[SqBind<wrappers::CircleShapeWrapper       >::get_typetag()] = Type_CircleShapeWrapper;
		p_cache[SqBind<wrappers::ConeShapeWrapper         >::get_typetag()] = Type_ConeShapeWrapper;
		p_cache[SqBind<wrappers::RayShapeWrapper          >::get_typetag()] = Type_RayShapeWrapper;
		
		p_cache[SqBind<wrappers::CameraEffectWrapper      >::get_typetag()] = Type_CameraEffectWrapper;
		p_cache[SqBind<wrappers::DarknessWrapper          >::get_typetag()] = Type_DarknessWrapper;
		p_cache[SqBind<wrappers::EffectRectWrapper        >::get_typetag()] = Type_EffectRectWrapper;
		//p_cache[SqBind<wrappers::EventWrapper             >::get_typetag()] = Type_EventWrapper;
		p_cache[SqBind<wrappers::FluidSettingsWrapper     >::get_typetag()] = Type_FluidSettingsWrapper;
		p_cache[SqBind<wrappers::LightWrapper             >::get_typetag()] = Type_LightWrapper;
		p_cache[SqBind<wrappers::MusicTrackWrapper        >::get_typetag()] = Type_MusicTrackWrapper;
		p_cache[SqBind<wrappers::ParticleEffectWrapper    >::get_typetag()] = Type_ParticleEffectWrapper;
		p_cache[SqBind<wrappers::PhysicsSettingsWrapper   >::get_typetag()] = Type_PhysicsSettingsWrapper;
		p_cache[SqBind<wrappers::PointerEventWrapper      >::get_typetag()] = Type_PointerEventWrapper;
		p_cache[SqBind<wrappers::PowerBeamGraphicWrapper  >::get_typetag()] = Type_PowerBeamGraphicWrapper;
		p_cache[SqBind<wrappers::PresentationObjectWrapper>::get_typetag()] = Type_PresentationObjectWrapper;
		p_cache[SqBind<wrappers::PresentationStartSettingsWrapper>::get_typetag()] = Type_PresentationStartSettingsWrapper;
		p_cache[SqBind<wrappers::SensorWrapper            >::get_typetag()] = Type_SensorWrapper;
		p_cache[SqBind<wrappers::ShoeboxPlaneDataWrapper  >::get_typetag()] = Type_ShoeboxPlaneDataWrapper;
		p_cache[SqBind<wrappers::SoundCueWrapper          >::get_typetag()] = Type_SoundCueWrapper;
		p_cache[SqBind<wrappers::TextureWrapper           >::get_typetag()] = Type_TextureWrapper;
		p_cache[SqBind<wrappers::TextLabelWrapper         >::get_typetag()] = Type_TextLabelWrapper;
		p_cache[SqBind<wrappers::TileSensorWrapper        >::get_typetag()] = Type_TileSensorWrapper;
	}
}


void UserType::serialize(const SQSerializer& p_serializer,
                         Type p_type,
                         SQUserPointer p_userData,
                         tt::code::BufferWriteContext* p_context)
{
	TT_NULL_ASSERT(p_userData);
	
	namespace bu = tt::code::bufferutils;
	using namespace toki::game::script;
	
	switch(p_type)
	{
	case Type_ColorRGB:
		{
			const tt::engine::renderer::ColorRGB* ptr = reinterpret_cast<tt::engine::renderer::ColorRGB*>(p_userData);
			bu::put(*ptr, p_context);
		}
		break;
		
	case Type_ColorRGBA:
		{
			const tt::engine::renderer::ColorRGBA* ptr = reinterpret_cast<tt::engine::renderer::ColorRGBA*>(p_userData);
			bu::put(*ptr, p_context);
		}
		break;
		
	case Type_Vector2:
		{
			const tt::math::Vector2* ptr = reinterpret_cast<tt::math::Vector2*>(p_userData);
			bu::put(*ptr, p_context);
		}
		break;
		
	case Type_VectorRect:
		{
			const tt::math::VectorRect* ptr = reinterpret_cast<tt::math::VectorRect*>(p_userData);
			bu::put(*ptr, p_context);
		}
		break;
		
	case Type_EasingReal:
		{
			const tt::math::interpolation::Easing<real>* ptr = 
				reinterpret_cast<tt::math::interpolation::Easing<real>*>(p_userData);
			bu::putEasing<real>(*ptr, p_context);
		}
		break;
		
	case Type_EntityBase:
		{
			const wrappers::EntityWrapper* ptr = reinterpret_cast<wrappers::EntityWrapper*>(p_userData);
			game::entity::EntityHandle handle(ptr->getHandle());
			game::entity::Entity* entity(handle.getPtr());
			const bool validEntity = (entity != 0 && entity->getEntityScript() != 0);
			bu::put(validEntity, p_context);
			if (validEntity)
			{
				entity->getEntityScript()->serialize(p_serializer, p_context);
			}
		}
		break;
		
	case Type_CircleShapeWrapper:
	case Type_BoxShapeWrapper:
	case Type_ConeShapeWrapper:
	case Type_RayShapeWrapper:
		{
			const wrappers::ShapeWrapper* ptr = reinterpret_cast<wrappers::ShapeWrapper*>(p_userData);
			ptr->getShape()->serialize(p_context);
		}
		break;
		
	case Type_CameraEffectWrapper:
		{
			const wrappers::CameraEffectWrapper* ptr =
					reinterpret_cast<wrappers::CameraEffectWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_DarknessWrapper:
		{
			const wrappers::DarknessWrapper* ptr = reinterpret_cast<wrappers::DarknessWrapper*>(p_userData);
			bu::putHandle(ptr->getHandle(), p_context);
		}
		break;
		
	case Type_EffectRectWrapper:
		{
			const wrappers::EffectRectWrapper* ptr = 
				reinterpret_cast<wrappers::EffectRectWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_FluidSettingsWrapper:
		{
			const wrappers::FluidSettingsWrapper* ptr =
					reinterpret_cast<wrappers::FluidSettingsWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_LightWrapper:
		{
			const wrappers::LightWrapper* ptr = reinterpret_cast<wrappers::LightWrapper*>(p_userData);
			bu::putHandle(ptr->getHandle(), p_context);
		}
		break;

	case Type_MusicTrackWrapper:
		{
			const wrappers::MusicTrackWrapper* ptr =
					reinterpret_cast<wrappers::MusicTrackWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_ParticleEffectWrapper:
		{
			const wrappers::ParticleEffectWrapper* ptr = reinterpret_cast<wrappers::ParticleEffectWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_PhysicsSettingsWrapper:
		{
			const wrappers::PhysicsSettingsWrapper* ptr =
					reinterpret_cast<wrappers::PhysicsSettingsWrapper*>(p_userData);
			ptr->getSettings().serialize(p_context);
		}
		break;
		
	case Type_PointerEventWrapper:
		{
			const wrappers::PointerEventWrapper* ptr =
					reinterpret_cast<wrappers::PointerEventWrapper*>(p_userData);
			ptr->getEvent().serialize(p_context);
		}
		break;
		
	case Type_PowerBeamGraphicWrapper:
		{
			const wrappers::PowerBeamGraphicWrapper* ptr =
					reinterpret_cast<wrappers::PowerBeamGraphicWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_PresentationObjectWrapper:
		{
			const wrappers::PresentationObjectWrapper* ptr =
					reinterpret_cast<wrappers::PresentationObjectWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_PresentationStartSettingsWrapper:
		{
			const wrappers::PresentationStartSettingsWrapper* ptr =
					reinterpret_cast<wrappers::PresentationStartSettingsWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_SensorWrapper:
		{
			const wrappers::SensorWrapper* ptr = reinterpret_cast<wrappers::SensorWrapper*>(p_userData);
			bu::putHandle(ptr->getHandle(), p_context);
		}
		break;
		
	case Type_ShoeboxPlaneDataWrapper:
		{
			//const wrappers::ShoeboxPlaneDataWrapper* ptr = reinterpret_cast<wrappers::ShoeboxPlaneDataWrapper*>(p_userData);
			//ptr->serialize(p_context);
			TT_PANIC("ShoeboxPlaneDataWrapper does not support serialize! Should also not happen! "
			         "(Can't use after spawn.)");
		}
		break;
		
	case Type_SoundCueWrapper:
		{
			const wrappers::SoundCueWrapper* ptr = reinterpret_cast<wrappers::SoundCueWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_TextureWrapper:
		{
			const wrappers::TextureWrapper* ptr = reinterpret_cast<wrappers::TextureWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_TextLabelWrapper:
		{
			const wrappers::TextLabelWrapper* ptr = reinterpret_cast<wrappers::TextLabelWrapper*>(p_userData);
			ptr->serialize(p_context);
		}
		break;
		
	case Type_TileSensorWrapper:
		{
			const wrappers::TileSensorWrapper* ptr = reinterpret_cast<wrappers::TileSensorWrapper*>(p_userData);
			bu::putHandle(ptr->getHandle(), p_context);
		}
		break;
		
	default:
		TT_PANIC("Unhandled usertype '%d'", p_type);
		break;
	}
}


void UserType::unserialize(HSQUIRRELVM p_vm,
                           const SQUnserializer& p_unserializer,
                           Type p_type,
                           tt::code::BufferReadContext* p_context)
{
	namespace bu = tt::code::bufferutils;
	using namespace toki::game::script;
	
#if !defined(TT_BUILD_FINAL)
	// This function should push the new unserialized object to the stack.
	const SQInteger preUnserializeTop = sq_gettop(p_vm);
#endif
	
	switch(p_type)
	{
	case Type_ColorRGB:
		{
			const tt::engine::renderer::ColorRGB data = bu::get<tt::engine::renderer::ColorRGB>(p_context);
			SqBind<tt::engine::renderer::ColorRGB>::push(p_vm, data);
		}
		break;
		
	case Type_ColorRGBA:
		{
			const tt::engine::renderer::ColorRGBA data = bu::get<tt::engine::renderer::ColorRGBA>(p_context);
			SqBind<tt::engine::renderer::ColorRGBA>::push(p_vm, data);
		}
		break;
		
	case Type_Vector2:
		{
			const tt::math::Vector2 data = bu::get<tt::math::Vector2>(p_context);
			SqBind<tt::math::Vector2>::push(p_vm, data);
		}
		break;
		
	case Type_VectorRect:
		{
			const tt::math::VectorRect data = bu::get<tt::math::VectorRect>(p_context);
			SqBind<tt::math::VectorRect>::push(p_vm, data);
		}
		break;
		
	case Type_EasingReal:
		{
			const tt::math::interpolation::Easing<real> data =
				bu::getEasing<real>(p_context);
			SqBind<tt::math::interpolation::Easing<real> >::push(p_vm, data);
		}
		break;
		
	case Type_EntityBase:
		{
			const bool validEntity = bu::get<bool>(p_context);
			if (validEntity == false)
			{
				sq_pushnull(p_vm);
			}
			else
			{
				toki::game::script::EntityBasePtr entityBase = toki::game::script::EntityBase::unserialize(p_unserializer, p_context);
				if (entityBase == 0)
				{
					sq_pushnull(p_vm);
				}
				else
				{
					sq_pushobject(p_vm, entityBase->getSqInstance());
				}
			}
		}
		break;
		
	case Type_CircleShapeWrapper:
	case Type_BoxShapeWrapper:
	case Type_ConeShapeWrapper:
	case Type_RayShapeWrapper:
		{
			using toki::game::entity::sensor::Shape;
			using toki::game::entity::sensor::ShapePtr;
			ShapePtr shape = Shape::unserialize(p_context);
			if (shape == 0)
			{
				sq_pushnull(p_vm);
			}
			else
			{
				shape->pushToVMStack(p_vm);
			}
		}
		break;
		
	case Type_CameraEffectWrapper:
		{
			wrappers::CameraEffectWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::CameraEffectWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_DarknessWrapper:
		{
			wrappers::DarknessWrapper wrapper(bu::getHandle<game::light::Darkness>(p_context));
			SqBind<wrappers::DarknessWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_EffectRectWrapper:
		{
			wrappers::EffectRectWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::EffectRectWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_FluidSettingsWrapper:
		{
			using game::fluid::FluidSettings;
			wrappers::FluidSettingsWrapper settingsWrapper;
			settingsWrapper.unserialize(p_context);
			SqBind<wrappers::FluidSettingsWrapper>::push(p_vm, settingsWrapper);
		}
		break;
		
	case Type_LightWrapper:
		{
			wrappers::LightWrapper wrapper(bu::getHandle<game::light::Light>(p_context));
			SqBind<wrappers::LightWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_MusicTrackWrapper:
		{
			wrappers::MusicTrackWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::MusicTrackWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_ParticleEffectWrapper:
		{
			wrappers::ParticleEffectWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::ParticleEffectWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_PhysicsSettingsWrapper:
		{
			using game::entity::movementcontroller::PhysicsSettings;
			wrappers::PhysicsSettingsWrapper settingsWrap(PhysicsSettings::unserialize(p_context));
			SqBind<wrappers::PhysicsSettingsWrapper>::push(p_vm, settingsWrap);
		}
		break;
		
	case Type_PointerEventWrapper:
		{
			using game::event::input::PointerEvent;
			wrappers::PointerEventWrapper eventWrap(PointerEvent::unserialize(p_context));
			SqBind<wrappers::PointerEventWrapper>::push(p_vm, eventWrap);
		}
		break;
		
	case Type_PowerBeamGraphicWrapper:
		{
			wrappers::PowerBeamGraphicWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::PowerBeamGraphicWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_PresentationObjectWrapper:
		{
			wrappers::PresentationObjectWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::PresentationObjectWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_PresentationStartSettingsWrapper:
		{
			wrappers::PresentationStartSettingsWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::PresentationStartSettingsWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_SensorWrapper:
		{
			wrappers::SensorWrapper wrapper(bu::getHandle<game::entity::sensor::Sensor>(p_context));
			SqBind<wrappers::SensorWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_ShoeboxPlaneDataWrapper:
		{
			//wrappers::ShoeboxPlaneDataWrapper wrapper;
			//wrapper.unserialize(p_context);
			//SqBind<wrappers::ShoeboxPlaneDataWrapper>::push(p_vm, wrapper);
			TT_PANIC("ShoeboxPlaneDataWrapper does not support unserialize! Should also not happen! "
			         "(Can't use after spawn.)");
		}
		break;
		
	case Type_SoundCueWrapper:
		{
			wrappers::SoundCueWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::SoundCueWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_TextureWrapper:
		{
			wrappers::TextureWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::TextureWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_TextLabelWrapper:
		{
			wrappers::TextLabelWrapper wrapper;
			wrapper.unserialize(p_context);
			SqBind<wrappers::TextLabelWrapper>::push(p_vm, wrapper);
		}
		break;
		
	case Type_TileSensorWrapper:
		{
			wrappers::TileSensorWrapper wrapper(bu::getHandle<game::entity::sensor::TileSensor>(p_context));
			SqBind<wrappers::TileSensorWrapper>::push(p_vm, wrapper);
		}
		break;
		
	default:
		TT_PANIC("Unhandled usertype '%d'", p_type);
		break;
	}
	
#if !defined(TT_BUILD_FINAL)
	// This function should push the new unserialized object to the stack.
	const SQInteger postUnserializeTop = sq_gettop(p_vm);
	TT_ASSERTMSG(preUnserializeTop + 1 == postUnserializeTop,
	             "UserType::unseralize should push one new object on the stack. This did not happen!"
	             "The stack top went from %d to %d (should be %d). Type was: %d.",
	              preUnserializeTop, postUnserializeTop, preUnserializeTop + 1, p_type);
#endif
}

// Namespace end
}
}
}
