#if !defined(INC_TOKI_SERIALIZATION_UTILS_H)
#define INC_TOKI_SERIALIZATION_UTILS_H

#include <tt/code/BufferReadContext.h>
#include <tt/code/BufferWriteContext.h>
#include <tt/engine/scene2d/fwd.h>
#include <tt/pres/fwd.h>

#include <toki/game/types.h>
#include <toki/game/movement/fwd.h>


namespace toki {
namespace serialization {

struct PersistentData
{
	PersistentData()
	{ }
};


struct PresentationRestoreInfo
{
	PresentationRestoreInfo(const std::string&    p_filename,
	                        const tt::pres::Tags& p_requiredTags,
	                        game::ParticleLayer   p_layer)
	:
	filename    (p_filename),
	requiredTags(p_requiredTags),
	layer       (p_layer)
	{
	}
	
	PresentationRestoreInfo()
	:
	filename(), requiredTags(), layer(game::ParticleLayer_Invalid)
	{
	}
	
	std::string         filename;
	tt::pres::Tags      requiredTags;
	game::ParticleLayer layer;
};


enum PersistentDataLoadResult
{
	PersistentDataLoadResult_Fail,
	PersistentDataLoadResult_Success,
	PersistentDataLoadResult_SuccessWithDemoFile
};

inline bool isPersistentDataLoadResultSuccessful(PersistentDataLoadResult p_result)
{
	return (p_result == PersistentDataLoadResult_Success) || (p_result == PersistentDataLoadResult_SuccessWithDemoFile);
}


bool savePersistentDataAndShutdownState(bool p_forceAndWait);
bool savePersistentData();
PersistentDataLoadResult loadPersistentData(PersistentData* p_persistentData);
void clearRegistries();
bool saveShutdownState();
bool loadShutdownState(u32* p_dataVersion = 0, bool p_forceLoadFromDemo = false);
bool removeShutdownState();

void serializePresentationRestoreInfo(const PresentationRestoreInfo& p_restoreInfo,
                                      tt::code::BufferWriteContext* p_context);
PresentationRestoreInfo unserializePresentationRestoreInfo(tt::code::BufferReadContext* p_context);


void serializePresentationObjectState(const tt::pres::PresentationObjectPtr& p_object,
                                      tt::code::BufferWriteContext* p_context);
void unserializePresentationObjectState(const tt::pres::PresentationObjectPtr& p_object,
                                        tt::code::BufferReadContext* p_context,
                                        real p_presTime);


void           serializePresTags(const tt::pres::Tags& p_tags, tt::code::BufferWriteContext* p_context);
tt::pres::Tags unserializePresTags(tt::code::BufferReadContext* p_context);


void serializeMove(const game::movement::MoveBasePtr& p_move,
                   const game::movement::MovementSetPtr& p_movementSet,
                   tt::code::BufferWriteContext* p_context);
game::movement::MoveBasePtr unserializeMove(const game::movement::MovementSetPtr& p_movementSet,
                                            tt::code::BufferReadContext* p_context);


void serializeBlurLayers(const tt::engine::scene2d::BlurLayers& p_layers,
                         tt::code::BufferWriteContext* p_context);
const tt::engine::scene2d::BlurLayers unserializeBlurLayers(tt::code::BufferReadContext* p_context);


void serializeTexturePtr(const tt::engine::renderer::TexturePtr& p_texture,
                         tt::code::BufferWriteContext* p_context);
tt::engine::renderer::TexturePtr unserializeTexturePtr(tt::code::BufferReadContext* p_context);


// Namespace end
}
}


#endif  // !defined(INC_TOKI_SERIALIZATION_UTILS_H)
