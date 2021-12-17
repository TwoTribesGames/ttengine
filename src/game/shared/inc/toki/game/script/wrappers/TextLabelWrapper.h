#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_TEXTLABELWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_TEXTLABELWRAPPER_H


#include <tt/code/fwd.h>
#include <tt/str/str_types.h>

#include <toki/game/entity/graphics/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

/*! \brief 'TextLabel' in Squirrel. */
class TextLabelWrapper
{
public:
	TextLabelWrapper();
	TextLabelWrapper(const entity::graphics::TextLabelHandle& p_textLabelGraphic);
	
	/*! \brief DEBUG: Render the border of the text. (Can be used to check alignment. Does nothing in final builds.) */
	void setShowTextBorders(bool p_show);
	/*! \brief Set the color of the text label. */
	void setColor(const tt::engine::renderer::ColorRGBA& p_color);
	/*! \brief Set the text (narrow string) for this label. */
	void setText(const std::string& p_text);
	/*! \brief Set the text (in UTF8 format) for this label. */
	void setTextUTF8(const std::string& p_text);
	/*! \brief Set the localized text for this label. */
	void setTextLocalized(const std::string& p_localizationKey);
	/*! \brief Set the localized text including parsed $VAR1$, $VAR2$ etc tokens */
	void setTextLocalizedAndFormatted(const std::string& p_localizationKey, const tt::str::Strings& p_vars);
	/*! \brief Gets the text in UTF8 format */
	std::string getTextUTF8() const;
	/*! \brief Gets the total number of characters in the label */
	s32 getTextLength() const;
	/*! \brief Gets the character at a certain index. Returns -1 in case of an error. */
	s32 getWCharAtIndex(s32 p_index) const;
	/*! \brief Gets the number of visible characters in the label */
	s32 getNumberOfVisibleCharacters() const;
	/*! \brief Sets the number of visible characters in the label */
	void setNumberOfVisibleCharacters(s32 p_numberOfVisibleCharacters);
	/*! \brief Gets or calculates (when unknown) the actual size rect occupied by the current text. */
	tt::math::VectorRect getUsedSize();
	/*! \brief Set the size of this Text Label. */
	void setSize(real p_width, real p_height);
	/*! \brief Set the scale for this text label. */
	void setScale(real p_scale);
	/*! \brief set Vertical Alignment */
	void setVerticalAlignment(  entity::graphics::VerticalAlignment   p_verticalAlignment);
	/*! \brief set Horizontal Alignment */
	void setHorizontalAlignment(entity::graphics::HorizontalAlignment p_horizontalAlignment);
	/*! \brief Adds a dropshadow */
	void addDropShadow(const tt::math::Vector2& p_offset, const tt::engine::renderer::ColorRGBA& p_color);
	/*! \brief Removes dropshadow */
	void removeDropShadow();
	
	/*! \brief Fade the Text Label in.
	    \param p_time duration of the fade. */
	void fadeIn( real p_time);
	/*! \brief Fade the Text Label out.
	    \param p_time duration of the fade. */
	void fadeOut(real p_time);
	
	void serialize  (tt::code::BufferWriteContext* p_context) const;
	void unserialize(tt::code::BufferReadContext*  p_context);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline const entity::graphics::TextLabelHandle& getHandle() const { return m_handle; }
	inline       entity::graphics::TextLabelHandle& getHandle()       { return m_handle; }
	
private:
	entity::graphics::TextLabelHandle m_handle;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_TEXTLABELWRAPPER_H)
