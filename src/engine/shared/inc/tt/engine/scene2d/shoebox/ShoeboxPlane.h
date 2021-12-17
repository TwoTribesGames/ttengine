#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOXPLANE_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOXPLANE_H


#include <tt/engine/scene2d/shoebox/Taggable.h>
#include <tt/engine/scene2d/PlaneScene.h>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

class ShoeboxPlane : public PlaneScene, public Taggable
{
public:
	ShoeboxPlane(real                        p_width,
	             real                        p_height,
	             const renderer::TexturePtr& p_texture,
	             const math::Vector3&        p_position         = math::Vector3::zero,
	             real                        p_rotation         = 0.0f,
	             real                        p_texAnimU         = 0.0f,
	             real                        p_texAnimV         = 0.0f,
	             real                        p_texOffsetScale   = 0.0f,
	             s32                         p_priority         = 0,
	             bool                        p_withVertexColors = false)
	:
	PlaneScene(p_width,
	           p_height,
	           p_texture,
	           p_position,
	           p_rotation,
	           p_texAnimU,
	           p_texAnimV,
	           p_texOffsetScale,
	           p_priority,
	           p_withVertexColors),
	Taggable(),
	m_visible(true)
	{ }
	
	virtual ~ShoeboxPlane();
	
	inline virtual void render()
	{
		if (m_visible)
		{
			PlaneScene::render();
		}
	}
	
	virtual bool handleEvent(const std::string& p_event, const std::string& p_param);
	
	inline void setVisible(bool p_visible) { m_visible = p_visible; }

	virtual bool isSuitableForBatching() const { return m_visible && PlaneScene::isSuitableForBatching(); }
	
private:
	ShoeboxPlane(const ShoeboxPlane&);
	const ShoeboxPlane& operator=(const ShoeboxPlane&);
	
	bool m_visible;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_SHOEBOXPLANE_H)
