#include <tt/engine/scene/ShadowModel.h>


namespace tt {
namespace engine {
namespace scene {

ShadowModel::ShadowModel() 
: 
Model(SceneObject::Type_ShadowModel)
{
	// Set the alpha
	setAlpha(128);
}

ShadowModel::~ShadowModel()
{
}

////////////////////////////////
// Private

ShadowModel::ShadowModel(const ShadowModel& p_model)
:
Model(p_model)
{
	// Set the alpha
	setAlpha(128);
}


// Namespace end
}
}
}

