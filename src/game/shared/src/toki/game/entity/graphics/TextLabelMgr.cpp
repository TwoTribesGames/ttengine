#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>
#include <tt/engine/renderer/Renderer.h>

#include <toki/game/entity/graphics/TextLabelMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

//--------------------------------------------------------------------------------------------------
// Public member functions

TextLabelMgr::TextLabelMgr(s32 p_reserveCount)
:
m_labels(p_reserveCount),
m_graphicsNeedUpdate(false)
{
}


TextLabelHandle TextLabelMgr::createTextLabel(const EntityHandle& p_parentHandle,
                                              utils::GlyphSetID   p_glyphSetId)
{
	return m_labels.create(TextLabel::CreationParams(p_parentHandle, p_glyphSetId));
}


void TextLabelMgr::destroyTextLabel(TextLabelHandle& p_handle)
{
	m_labels.destroy(p_handle);
	
	p_handle.invalidate();
}


void TextLabelMgr::setShowTextBorders(bool p_show)
{
	TextLabel* textLabelPtr = m_labels.getFirst();
	for (s32 i = 0; i <  m_labels.getActiveCount(); ++i, ++textLabelPtr)
	{
		textLabelPtr->setShowTextBorders(p_show);
	}
}


void TextLabelMgr::update(real p_elapsedTime)
{
	TextLabel* textLabelPtr = m_labels.getFirst();
	for (s32 i = 0; i <  m_labels.getActiveCount(); ++i, ++textLabelPtr)
	{
		textLabelPtr->update(p_elapsedTime);
	}
	m_graphicsNeedUpdate = true;
}


void TextLabelMgr::updateForRender()
{
	if (m_graphicsNeedUpdate)
	{
		TextLabel* textLabelPtr = m_labels.getFirst();
		for (s32 i = 0; i <  m_labels.getActiveCount(); ++i, ++textLabelPtr)
		{
			textLabelPtr->updateForRender();
		}
	}
	m_graphicsNeedUpdate = false;
}


void TextLabelMgr::render() const
{
	//tt::engine::renderer::Renderer* renderer = tt::engine::renderer::Renderer::getInstance();
	//const bool fogWasEnabled = renderer->isFogEnabled();
	//renderer->setFogEnabled(false);
	// Disable fog for all text labels.
	
	const TextLabel* textLabelPtr = m_labels.getFirst();
	for (s32 i = 0; i <  m_labels.getActiveCount(); ++i, ++textLabelPtr)
	{
		textLabelPtr->render();
	}
	
	// Restore fog setting
	//renderer->setFogEnabled(fogWasEnabled);
}


void TextLabelMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_TextLabelMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TextLabelMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	tt::code::serializeHandleArrayMgr(m_labels, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void TextLabelMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(
			serialization::Section_TextLabelMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the TextLabelMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	tt::code::unserializeHandleArrayMgr(&m_labels, &context);
}

// Namespace end
}
}
}
}
