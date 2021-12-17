#if !defined(INC_TOKI_LEVEL_ENTITY_EDITOR_FWD_H)
#define INC_TOKI_LEVEL_ENTITY_EDITOR_FWD_H


#include <string>

#include <tt/code/BitMask.h>
#include <tt/platform/tt_error.h>


namespace toki {
namespace level {
namespace entity {
namespace editor {

class EntityInstanceEditorRepresentation;


enum RenderFlag
{
	RenderFlag_Image,
	RenderFlag_SizeShapes,
	RenderFlag_AllEntityIDs,
	RenderFlag_AllEntityReferences,
	RenderFlag_EntityReferencesLabels,
	
	RenderFlag_Count,
	RenderFlag_Invalid
};

typedef tt::code::BitMask<RenderFlag, RenderFlag_Count> RenderFlags;


inline bool isValidRenderFlag(RenderFlag p_flag)
{
	return p_flag >= 0 && p_flag < RenderFlag_Count;
}


inline const char* const getRenderFlagName(RenderFlag p_flag)
{
	switch (p_flag)
	{
	case RenderFlag_Image:                  return "image";
	case RenderFlag_SizeShapes:             return "size_shapes";
	case RenderFlag_AllEntityIDs:           return "all_entity_ids";
	case RenderFlag_AllEntityReferences:    return "all_entity_references";
	case RenderFlag_EntityReferencesLabels: return "entity_references_labels";
		
	default:
		TT_PANIC("Invalid render flag: %d", p_flag);
		return "";
	}
}


inline RenderFlag getRenderFlagFromName(const std::string& p_flagName)
{
	for (s32 i = 0; i < RenderFlag_Count; ++i)
	{
		const RenderFlag flag = static_cast<RenderFlag>(i);
		if (getRenderFlagName(flag) == p_flagName)
		{
			return flag;
		}
	}
	
	return RenderFlag_Invalid;
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_LEVEL_ENTITY_EDITOR_FWD_H)
