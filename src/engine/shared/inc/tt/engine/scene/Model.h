#if !defined(INC_TT_ENGINE_SCENE_MODEL_H)
#define INC_TT_ENGINE_SCENE_MODEL_H

#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/math/Vector3.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/engine/renderer/SubModel.h>
#include <tt/engine/renderer/Sphere.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/engine/animation/fwd.h>


namespace tt {
namespace engine {
namespace scene {


class Model : public SceneObject
{
public:
	Model();
	explicit Model(SceneObject::Type p_type);
	virtual ~Model() {}

	ModelPtr            clone() const;
	renderer::SubModel* getSubModel(s32 p_index);
	
	inline s32   getSubModelCount() const { return static_cast<s32>(m_subModels.size()); }
	inline void  setSphere(const renderer::Sphere& p_sphere) { m_sphere = p_sphere; }
	inline const renderer::Sphere& getSphere() const { return m_sphere; }
	virtual Model* getModel() { return this; }
	
	void setRenderNormals(bool p_enabled) { m_renderNormals = p_enabled; }
	
protected:
	virtual bool load(const fs::FilePtr& p_file);

	virtual void renderObject(renderer::RenderContext& p_renderContext);
	void renderNormals();
	 
	Model(const Model& p_model);

private:
	// No assignment
	Model& operator=(const Model&);
	
	
	renderer::Sphere m_sphere;
	bool             m_renderNormals;
	
	typedef std::vector<renderer::SubModel> SubModelContainer;
	SubModelContainer m_subModels;
};


// Namespace end
}
}
}


#endif // INC_TT_ENGINE_SCENE_MODEL_H
