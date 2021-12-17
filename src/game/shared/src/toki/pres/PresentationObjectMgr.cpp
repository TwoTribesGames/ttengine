#include <tt/code/bufferutils.h>
#include <tt/code/HandleArrayMgr_utils.h>
#include <tt/pres/PresentationMgr.h>

#include <toki/pres/PresentationObjectMgr.h>
#include <toki/serialization/SerializationMgr.h>


namespace toki {
namespace pres {

//--------------------------------------------------------------------------------------------------
// Public member functions

PresentationObjectMgr::PresentationObjectMgr(s32 p_reserveCount)
:
m_objects(p_reserveCount)
{
}


PresentationObjectHandle PresentationObjectMgr::createPresentationObject(const game::entity::EntityHandle& p_source,
                                                                         const std::string& p_filename,
                                                                         const tt::pres::Tags& p_requiredTags,
                                                                         game::ParticleLayer p_layer)
{
	serialization::PresentationRestoreInfo restoreInfo = 
		serialization::PresentationRestoreInfo(p_filename, p_requiredTags, p_layer);
	
	return m_objects.create(PresentationObject::CreationParams(p_source, restoreInfo));
}


void PresentationObjectMgr::destroyPresentationObject(PresentationObjectHandle& p_handle)
{
	m_objects.destroy(p_handle);
	
	p_handle.invalidate();
}


void PresentationObjectMgr::checkAnimationEnded()
{
	PresentationObject* object = m_objects.getFirst();
	for (s32 i = 0; i < m_objects.getActiveCount(); ++i, ++object)
	{
		object->checkAnimationEnded();
	}
}


void PresentationObjectMgr::update(real p_deltaTime)
{
	PresentationObject* object = m_objects.getFirst();
	for (s32 i = 0; i < m_objects.getActiveCount(); ++i, ++object)
	{
		object->update(p_deltaTime);
	}
}


void PresentationObjectMgr::serialize(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_PresentationObjectMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PresentationObjectMgr data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	tt::code::serializeHandleArrayMgr(m_objects, &context);
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void PresentationObjectMgr::unserialize(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section =
		p_serializationMgr.getSection(serialization::Section_PresentationObjectMgr);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PresentationObjectMgr data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	tt::code::unserializeHandleArrayMgr(&m_objects, &context);
}


void PresentationObjectMgr::serializeState(toki::serialization::SerializationMgr& p_serializationMgr) const
{
	const serialization::SerializerPtr& section = p_serializationMgr.getSection(serialization::Section_PresentationObjectMgrState);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PresentationObjectMgrState data.");
		return;
	}
	
	tt::code::BufferWriteContext context(section->getAppendContext());
	
	namespace bu = tt::code::bufferutils;
	
	const u32 objectCount = static_cast<u32>(m_objects.getActiveCount());
	bu::put(objectCount, &context);
	
	const PresentationObject* object = m_objects.getFirst();
	for (u32 i = 0; i < objectCount; ++i, ++object)
	{
		object->serializeState(&context);
	}
	
	// Done writing data: ensure the underlying buffer gets all the data that was written
	context.flush();
}


void PresentationObjectMgr::unserializeState(const toki::serialization::SerializationMgr& p_serializationMgr)
{
	const serialization::SerializerPtr& section =
		p_serializationMgr.getSection(serialization::Section_PresentationObjectMgrState);
	if (section == 0)
	{
		TT_PANIC("Serialization manager does not contain a section for the PresentationObjectMgrState data.");
		return;
	}
	
	tt::code::BufferReadContext context(section->getReadContext());
	
	namespace bu = tt::code::bufferutils;
	
	const u32 objectCount = bu::get<u32>(&context);
	
	PresentationObject* object = m_objects.getFirst();
	for (u32 i = 0; i < objectCount; ++i, ++object)
	{
		object->unserializeState(&context);
	}
}


// Namespace end
}
}
