#if !defined(INC_TOKI_GAME_LIGHT_GLOW_H)
#define INC_TOKI_GAME_LIGHT_GLOW_H


#include <tt/code/fwd.h>
#include <tt/engine/renderer/fwd.h>

#include <toki/game/light/fwd.h>
#include <toki/game/light/Light.h>


namespace toki {
namespace game {
namespace light {

class Glow
{
public:
	struct CreationParams
	{
		inline CreationParams(const std::string& p_imageName, real p_scale,
		                      real p_minRadius, real p_maxRadius, real p_fadeRadius)
		:
		imageName(p_imageName),
		scale(p_scale),
		minRadius(p_minRadius),
		maxRadius(p_maxRadius),
		fadeRadius(p_fadeRadius)
		{ }
		
		std::string imageName;
		real        scale;
		real        minRadius;
		real        maxRadius;
		real        fadeRadius;
	};
	typedef const CreationParams& ConstructorParamType;
	
	static GlowPtr create(const CreationParams& p_creationParams);
	
	void update(const Light& p_light);
	void render() const;
	
	void           serialize  (tt::code::BufferWriteContext* p_context) const;
	static GlowPtr unserialize(tt::code::BufferReadContext*  p_context);
	
private:
	Glow(const CreationParams& p_creationParams,
	     const tt::engine::renderer::QuadSpritePtr p_glowQuad);
	
	CreationParams                      m_creationParams;
	tt::engine::renderer::QuadSpritePtr m_glowQuad;
	
	// Unfortunately there isn't a getOpacity() in QuadSprite
	// so we need to store this for optimization purposes. No need to (un)serialize it though.
	u8                                  m_opacity; 
};


// Namespace end
}
}
}


#endif  // !defined(INC_TOKI_GAME_LIGHT_GLOW_H)
