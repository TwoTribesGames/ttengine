#if !defined(INC_TOKI_GAME_EDITOR_TOOLS_TYPES_H)
#define INC_TOKI_GAME_EDITOR_TOOLS_TYPES_H


#include <string>

#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace toki {
namespace game {
namespace editor {
namespace tools {

enum ToolID
{
	ToolID_Draw,
	ToolID_FloodFill,
	ToolID_Hand, // For scrolling around the world
	ToolID_BoxSelect,
	ToolID_Resize,
	ToolID_EntityPaint,  // Adds entities to the world (or removes them).
	ToolID_EntityMove,   // Moves existing entities.
	ToolID_Notes,
	
	ToolID_Count,
	ToolID_Invalid
};


inline bool isValidToolID(ToolID p_tool)
{
	return p_tool >= 0 && p_tool < ToolID_Count;
}


inline const char* getToolIDName(ToolID p_tool)
{
	switch (p_tool)
	{
	case ToolID_Draw:        return "draw";
	case ToolID_FloodFill:   return "floodfill";
	case ToolID_Hand:        return "hand";
	case ToolID_BoxSelect:   return "boxselect";
	case ToolID_Resize:      return "resize";
	case ToolID_EntityPaint: return "entitypaint";
	case ToolID_EntityMove:  return "entitymove";
	case ToolID_Notes:       return "notes";
		
	default:
		TT_PANIC("Invalid paint tool: %d", p_tool);
		return "";
	}
}


inline ToolID getToolIDFromName(const std::string& p_name)
{
	for (s32 i = 0; i < ToolID_Count; ++i)
	{
		const ToolID tool = static_cast<ToolID>(i);
		if (p_name == getToolIDName(tool))
		{
			return tool;
		}
	}
	
	return ToolID_Invalid;
}

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_EDITOR_TOOLS_TYPES_H)
