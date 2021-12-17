#if !defined(INC_TT_ENGINE_ANIMATION_CONTROLLER_H)
#define INC_TT_ENGINE_ANIMATION_CONTROLLER_H

// Include necessary files
#include <tt/platform/tt_types.h>
#include <tt/fs/types.h>
#include <tt/engine/animation/fwd.h>


namespace tt {
namespace engine {
namespace animation {

class Controller
{
public:
	enum Type
	{
		Type_HermiteFloat,
		Type_StepFloat,
		Type_ConstantFloat,
		Type_Transform,
		Type_TextureMatrix,
	};

	Controller() {}
	virtual	~Controller() {}
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_ANIMATION_CONTROLLER_H
