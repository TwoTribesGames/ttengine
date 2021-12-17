#if !defined(INC_TOKI_SCRIPT_SERIALIZATION_USERTYPE_H)
#define INC_TOKI_SCRIPT_SERIALIZATION_USERTYPE_H

#include <map>

#include <squirrel/squirrel.h>

#include <tt/code/fwd.h>

#include <toki/script/serialization/fwd.h>

namespace toki {
namespace script {
namespace serialization {

class UserType
{
public:
	enum Type
	{
		Type_None,
		
		// TTDEV
		Type_ColorRGB,
		Type_ColorRGBA,
		Type_Vector2,
		Type_VectorRect,
		Type_EasingReal,
		
		// TOKI
		Type_EntityBase,
		
		// TOKI - Wrappers
		//Type_ShapeWrapper, // We want to group the shapes.
		Type_CircleShapeWrapper,
		Type_BoxShapeWrapper,
		Type_ConeShapeWrapper,
		Type_RayShapeWrapper,
		
		// Alphabatic from here.
		Type_CameraEffectWrapper,
		Type_DarknessWrapper,
		Type_EffectRectWrapper,
		//Type_EventWrapper,
		Type_FluidSettingsWrapper,
		Type_LightWrapper,
		Type_MusicTrackWrapper,
		Type_ParticleEffectWrapper,
		Type_PhysicsSettingsWrapper,
		Type_PointerEventWrapper,
		Type_PowerBeamGraphicWrapper,
		Type_PresentationObjectWrapper,
		Type_PresentationStartSettingsWrapper,
		Type_SensorWrapper,
		Type_ShoeboxPlaneDataWrapper,
		Type_SoundCueWrapper,
		Type_TextureWrapper,
		Type_TextLabelWrapper,
		Type_TileSensorWrapper,
		
		
		Type_Count,
		Type_Invalid
	};
	
	typedef std::map<SQUserPointer, Type> Cache;
	
	static void buildCache(Cache& p_cache);
	static void serialize(const SQSerializer& p_serializer, Type p_type, SQUserPointer p_userData, tt::code::BufferWriteContext* p_context);
	static void unserialize(HSQUIRRELVM p_vm, const SQUnserializer& p_unserializer, Type p_type, tt::code::BufferReadContext* p_context);
};


// Namespace end
}
}
}

#endif  // !defined(INC_TOKI_SCRIPT_SERIALIZATION_USERTYPE_H)
