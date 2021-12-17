#if !defined(INC_TT_ENGINE_SCENE_SHADOWMODEL_H)
#define INC_TT_ENGINE_SCENE_SHADOWMODEL_H


#include <tt/platform/tt_types.h>
#include <tt/engine/scene/Model.h>


namespace tt {
namespace engine {
namespace scene {

class ShadowModel : public Model
{
public:
	ShadowModel();
	virtual	~ShadowModel();

private:
	ShadowModel(const ShadowModel& p_model);
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_SHADOWMODEL_H
