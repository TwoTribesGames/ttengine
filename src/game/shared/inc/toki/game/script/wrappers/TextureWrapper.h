#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_TEXTUREWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_TEXTUREWRAPPER_H


#include <tt/code/fwd.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/script/helpers.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'Texture' in Squirrel.  */
class TextureWrapper
{
public:
	inline TextureWrapper()
	:
	m_texture()
	{ }
	
	inline explicit TextureWrapper(const tt::engine::renderer::TexturePtr& p_texture)
	:
	m_texture(p_texture)
	{ }
	
	inline const tt::engine::renderer::TexturePtr& getTexture() const { return m_texture; }
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
private:
	tt::engine::renderer::TexturePtr m_texture;
};

// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_TEXTUREWRAPPER_H)
