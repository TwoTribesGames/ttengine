#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>

#include <toki/game/light/Darkness.h>
#include <toki/game/light/DarknessMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace game {
namespace light {


//--------------------------------------------------------------------------------------------------
// Public member functions

DarknessMgr::DarknessMgr(s32 p_reserveCount)
:
m_darknesses(p_reserveCount)
{
}


DarknessMgr::~DarknessMgr()
{
}


DarknessHandle DarknessMgr::createDarkness(const entity::EntityHandle& p_source,
                                           real                        p_width,
                                           real                        p_height)
{
	return m_darknesses.create(Darkness::CreationParams(p_source, p_width, p_height));
}


void DarknessMgr::destroyDarkness(DarknessHandle& p_handle)
{
	m_darknesses.destroy(p_handle);
	
	p_handle.invalidate();
}


void DarknessMgr::updateRects(Rects* p_rects, u8 p_ambient)
{
	TT_NULL_ASSERT(p_rects);
	p_rects->clear();
	
	Darkness* dark = m_darknesses.getFirst();
	for (s32 i = 0; i < m_darknesses.getActiveCount(); ++i, ++dark)
	{
		if (dark->isEnabled())
		{
			dark->setAmbient(p_ambient);
			p_rects->push_back(dark->getRect());
		}
	}
}


void DarknessMgr::renderDarkness() const
{
	const Darkness* dark = m_darknesses.getFirst();
	for (s32 i = 0; i < m_darknesses.getActiveCount(); ++i, ++dark)
	{
		if (dark->isEnabled())
		{
			dark->render();
		}
	}
}


void DarknessMgr::handleLevelResized()
{
}


void DarknessMgr::resetLevel()
{
	m_darknesses.reset();
}


void DarknessMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_DarknessMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the DarknessMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	tt::code::serializeHandleArrayMgr(m_darknesses, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void DarknessMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_DarknessMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the DarknessMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	tt::code::unserializeHandleArrayMgr(&m_darknesses, &context);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

// Namespace end
}
}
}

