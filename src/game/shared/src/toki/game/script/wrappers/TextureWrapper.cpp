#include <tt/code/bufferutils.h>
#include <tt/engine/renderer/Texture.h>
#include <tt/platform/tt_error.h>

#include <toki/game/script/wrappers/TextureWrapper.h>
#include <toki/serialization/utils.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


void TextureWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	serialization::serializeTexturePtr(m_texture, p_context);
}


void TextureWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	// Start with defaults
	*this = TextureWrapper();
	
	m_texture = serialization::unserializeTexturePtr(p_context);
}


void TextureWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(TextureWrapper, "Texture");
}

// Namespace end
}
}
}
}
