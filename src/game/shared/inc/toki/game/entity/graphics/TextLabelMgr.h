#if !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABELMGR_H)
#define INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABELMGR_H


#include <tt/code/HandleArrayMgr.h>

#include <toki/game/entity/graphics/fwd.h>
#include <toki/game/entity/graphics/types.h>
#include <toki/game/entity/graphics/TextLabel.h>
#include <toki/game/entity/fwd.h>
#include <toki/serialization/fwd.h>


namespace toki {
namespace game {
namespace entity {
namespace graphics {

class TextLabelMgr
{
public:
	explicit TextLabelMgr(s32 p_reserveCount);
	
	TextLabelHandle createTextLabel(const EntityHandle& p_parentHandle,
	                                utils::GlyphSetID   p_glyphSetId);
	
	void destroyTextLabel(TextLabelHandle& p_handle);
	inline TextLabel* getTextLabel(const TextLabelHandle& p_handle)
	{
		return m_labels.get(p_handle);
	}
	void setShowTextBorders(bool p_show);
	
	void update(real p_elapsedTime);
	void updateForRender();
	void render() const;
	
	inline void reset() { m_labels.reset(); }
	
	// FIXME: (Un)serialization should probably indicate whether this was successful
	void serialize  (      toki::serialization::SerializationMgr& p_serializationMgr) const;
	void unserialize(const toki::serialization::SerializationMgr& p_serializationMgr);
	
private:
	typedef tt::code::HandleArrayMgr<TextLabel> TextLabels;
	
	TextLabels m_labels;
	bool       m_graphicsNeedUpdate;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_ENTITY_GRAPHICS_TEXTLABELMGR_H)
