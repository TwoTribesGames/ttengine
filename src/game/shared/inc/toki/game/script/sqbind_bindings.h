#if !defined(INC_TOKI_GAME_SCRIPT_SQBIND_BINDINGS_H)
#define INC_TOKI_GAME_SCRIPT_SQBIND_BINDINGS_H

#include <squirrel/sqbind.h>

#include <tt/script/bindings/bindings.h>
#include <tt/stats/stats.h>

#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/types.h>
#include <toki/game/entity/Entity.h>
#include <toki/game/event/fwd.h>
#include <toki/game/fluid/types.h>
#include <toki/game/movement/fwd.h>
#include <toki/game/script/EntityBase.h>
#include <toki/game/script/wrappers/EntityWrapper.h>
#include <toki/game/script/wrappers/LightWrapper.h>
#include <toki/game/types.h>
#include <toki/input/types.h>
#include <toki/level/types.h>

#include <toki/utils/types.h>
#include <toki/constants.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {
	 // Forward declaration
	class EntityWrapper;
	class LightWrapper;
	class PresentationObjectWrapper;
}
}
}
}


#ifdef SQBIND_NAMESPACE
namespace SQBIND_NAMESPACE {
#endif

// Bind enum types to Squirrel
SQBIND_INTEGER(toki::game::event::EventType);
SQBIND_INTEGER(toki::game::movement::Direction);
SQBIND_INTEGER(toki::game::movement::SurveyResult);
SQBIND_INTEGER(toki::game::entity::LocalDir);
SQBIND_INTEGER(toki::game::fluid::FluidType);
SQBIND_INTEGER(toki::game::ParticleLayer);
SQBIND_INTEGER(toki::level::CollisionType);
SQBIND_INTEGER(toki::level::ThemeType);
SQBIND_INTEGER(toki::level::Placeable);
SQBIND_INTEGER(toki::game::entity::graphics::PowerBeamType);
SQBIND_INTEGER(toki::game::entity::graphics::HorizontalAlignment);
SQBIND_INTEGER(toki::game::entity::graphics::VerticalAlignment);
SQBIND_INTEGER(toki::game::entity::effect::EffectRectTarget);
SQBIND_INTEGER(toki::input::GamepadControlScheme);
SQBIND_INTEGER(toki::input::RumbleStrength);
SQBIND_INTEGER(toki::game::script::wrappers::LightWrapper::LevelLightMode);
SQBIND_INTEGER(toki::utils::GlyphSetID);
SQBIND_INTEGER(toki::ProgressType);


TT_SQBIND_SET_ASSIGNONLY(toki::game::script::wrappers::EntityWrapper);


template<>
class SqBind<toki::game::script::EntityBase>
{
public:
	struct GetterPtr {
	
		SQBIND_INLINE toki::game::script::EntityBase* get(HSQUIRRELVM v, int p_idx)
		{
			if (sq_gettype(v, p_idx) == OT_NULL)
			{
				return 0;
			}
			
			SQUserPointer usrPtr;
			if (SQ_FAILED(sq_getinstanceup(v, p_idx, &usrPtr, get_typetag())))
			{
				return 0;
			}
			toki::game::entity::Entity* entity = 
				reinterpret_cast<toki::game::script::wrappers::EntityWrapper*>(usrPtr)->getHandle().getPtr();
			if (entity == 0)
			{
				return 0;
			}
			return entity->getEntityScript().get();
		}
	};
	
	static void push(HSQUIRRELVM v, const toki::game::script::EntityBase& p_value) 
	{
		sq_pushobject(v, p_value.getSqInstance());
	}
	
	static void push(HSQUIRRELVM v, const toki::game::script::EntityBase* p_value) 
	{
		if (p_value == 0)
		{
			sq_pushnull(v);
			return;
		}
		sq_pushobject(v, p_value->getSqInstance());
	}
	
	static SQUserPointer get_typetag()
	{
		// Address is used as squirrel type tag. (Like class_id is used in sqBind).
		return static_cast<SQUserPointer>(&typeTagDummy);
	}
	
private:
	static s32 typeTagDummy;
};



#ifdef SQBIND_NAMESPACE
}; // end of sqbind namespace
#endif

#endif  // !defined(INC_TOKI_GAME_SCRIPT_SQBIND_BINDINGS_H)
