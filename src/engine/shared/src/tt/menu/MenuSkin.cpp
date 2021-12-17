#include <tt/platform/tt_error.h>
//#include <tt/memory/HeapMgr.h>
#include <tt/menu/MenuSkin.h>



namespace tt {
namespace menu {

//------------------------------------------------------------------------------
// Inner class implementations

MenuSkin::SkinTexture::SkinTexture(const std::string& p_filename,
                                   real               p_u,
                                   real               p_v,
                                   real               p_width,
                                   real               p_height)
:
m_filename(p_filename),
m_u(p_u),
m_v(p_v),
m_width(p_width),
m_height(p_height)
{
}


const MenuSkin::SkinTexture& MenuSkin::ElementSkin::getTexture(
		int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getTextureCount(),
	             "Skin texture index %d out of range [0 - %d).",
	             p_index, getTextureCount());
	return m_textures.at(static_cast<Textures::size_type>(p_index));
}


engine::renderer::ColorRGBA MenuSkin::ElementSkin::getVertexColor(
		int p_index) const
{
	TT_ASSERTMSG(p_index >= 0 && p_index < getVertexColorCount(),
	             "Vertex color index %d out of range [0 - %d).",
	             p_index, getVertexColorCount());
	return m_vertexColors.at(static_cast<VertexColors::size_type>(p_index));
}


//------------------------------------------------------------------------------
// Public member functions

MenuSkin::MenuSkin()
{
}


MenuSkin::~MenuSkin()
{
}


void MenuSkin::addElementSkin(int                          p_elementID,
                              const MenuSkin::ElementSkin& p_elementSkin)
{
	m_elementSkins.insert(ElementSkins::value_type(p_elementID, p_elementSkin));
}


const MenuSkin::ElementSkin& MenuSkin::getElementSkin(int p_elementID) const
{
	ElementSkins::const_iterator it = m_elementSkins.find(p_elementID);
	TT_ASSERTMSG(it != m_elementSkins.end(),
	             "Element ID %d does not have skin information.",
	             p_elementID);
	return (*it).second;
}


bool MenuSkin::hasElementSkin(int p_elementID) const
{
	return (m_elementSkins.find(p_elementID) != m_elementSkins.end());
}


/*
void* MenuSkin::operator new(size_t p_blockSize)
{
	using memory::HeapMgr;
#ifndef TT_BUILD_FINAL
	u32 foo = 0;
	asm	{    mov     foo, lr}
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize, 4, (void*)foo);
#else
	return HeapMgr::allocFromHeap(HeapMgr::SINGLETON, p_blockSize);
#endif
}


void MenuSkin::operator delete(void* p_block)
{
	memory::HeapMgr::freeToHeap(p_block);
}
//*/

// Namespace end
}
}
