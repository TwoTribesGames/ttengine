#if !defined(INC_TOKI_PRES_PRESENTATIONOBJECTMGR_H)
#define INC_TOKI_PRES_PRESENTATIONOBJECTMGR_H

#include <tt/code/HandleArrayMgr.h>
#include <tt/pres/fwd.h>

#include <toki/game/fwd.h>
#include <toki/game/entity/fwd.h>
#include <toki/pres/PresentationObject.h>
#include <toki/pres/fwd.h>
#include <toki/serialization/fwd.h>

namespace toki {
namespace pres {

class PresentationObjectMgr
{
public:
	explicit PresentationObjectMgr(s32 p_reserveCount);
	
	PresentationObjectHandle createPresentationObject(const game::entity::EntityHandle& p_source,
	                                                  const std::string& p_filename,
	                                                  const tt::pres::Tags& p_requiredTags,
	                                                  game::ParticleLayer p_layer);
	
	void destroyPresentationObject(PresentationObjectHandle& p_handle);
	
	inline PresentationObject* getPresentationObject(const PresentationObjectHandle& p_handle)
	{ return m_objects.get(p_handle); }
	
	void checkAnimationEnded();
	void update(real p_deltaTime);
	
	inline void reset() { m_objects.reset(); }
	
	inline s32 getActiveCount() const { return m_objects.getActiveCount(); }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
	// State is serialized separatly from resources.
	void serializeState  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserializeState(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<PresentationObject> PresentationObjects;
	
	PresentationObjects m_objects;
};

// Namespace end
}
}

#endif  // !defined(INC_TOKI_PRES_PRESENTATIONOBJECTMGR_H)
