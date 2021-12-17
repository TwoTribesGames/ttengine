#if !defined(INC_TT_ENGINE_SCENE2D_WORLDSCENE_H)
#define INC_TT_ENGINE_SCENE2D_WORLDSCENE_H


#include <list>
#include <vector>

#include <tt/engine/scene2d/SceneInterface.h>
#include <tt/engine/scene2d/fwd.h>


namespace tt {
namespace engine {
namespace scene2d {

class Scene2D;

/*! \brief Scene for organizing scene nodes */
class WorldScene : public SceneInterface
{
public:
	WorldScene();
	virtual ~WorldScene();
	
	/*! \brief Update logic of all scene nodes */
	virtual void update(real p_delta_time);
	
	/*! \brief Render the scene to the screen */
	virtual void render();

	/*! \brief Render the scene to the screen with dynamically blurred planes */
	void renderWithBlurFromBack(const BlurLayers& p_layers, BlurQuality p_quality);

	/*! \brief Render the scene to the screen with dynamically blurred planes */
	void renderWithBlurToFront(const BlurLayers& p_layers, BlurQuality p_quality);
	
	/*! \brief Insert a render node in the scene */
	virtual void insert(Scene2D* p_node);
	
	/*! \brief Remove a render node from the scene */
	virtual void remove(Scene2D* p_node);
	
	/*! \brief Rearrange a render node within the scene */
	virtual void rearrange(Scene2D* p_node);
	
	/*! \brief Remove all render nodes from the scene */
	virtual void removeAll();
	
	/*! \brief Delete and remove all render nodes from the scene. */
	virtual void deleteAll();
	
	/* \brief Indicates whether it is a virtual scene or not */
	virtual bool isVirtual() const { return false; }
	
	/* Create quad batches for non-animated planes */
	void createBatchesFromPlanes();
	
	void invalidateBatch();
	
	static void setCullPlanesInUpdate(bool p_enable) { ms_cullPlanesInUpdate = p_enable; }
	
private:
	typedef std::vector<Scene2D*> SceneVector;
	
	SceneVector m_sceneNodes;
	SceneVector m_scheduledForResort;
	SceneVector m_visibleNodes;
	SceneVector m_nonBatchableNodes;
	bool        m_isUpdating; // for sanity checking: whether currently in update()
	
	struct TextureState
	{
		renderer::TexturePtr  texture;
		renderer::AddressMode addressModeU;
		renderer::AddressMode addressModeV;
		renderer::BlendMode   blendMode;
		bool                  fogEnabled;
	};
	
	struct BatchLayer
	{
		SceneVector             animatedPlanes;
		renderer::QuadBufferPtr staticPlanes;
		TextureState            staticState;
	};
	typedef std::vector<BatchLayer> QuadBatches;
	
	typedef std::vector<PlaneScene*> Planes;
	struct ParsedLayer
	{
		SceneVector  animatedPlanes;
		Planes       staticPlanes;
		TextureState state;
	};
	typedef std::vector<ParsedLayer> PlaneBatch;
	
	
	QuadBatches m_quadBatches;
	bool        m_batchCreated;
	
	static bool ms_cullPlanesInUpdate;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_WORLDSCENE_H)
