#include <tt/code/bufferutils.h>
#include <tt/platform/tt_error.h>
#include <tt/script/ScriptEngine.h>
#include <tt/str/common.h>

#include <toki/game/entity/graphics/TextLabel.h>
#include <toki/game/script/wrappers/TextLabelWrapper.h>
#include <toki/game/script/sqbind_bindings.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

//--------------------------------------------------------------------------------------------------
// Public member functions

TextLabelWrapper::TextLabelWrapper()
:
m_handle()
{
}


TextLabelWrapper::TextLabelWrapper(
		const entity::graphics::TextLabelHandle& p_handle)
:
m_handle(p_handle)
{
}


void TextLabelWrapper::setShowTextBorders(bool p_show)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setShowTextBorders(p_show);
	}
}


void TextLabelWrapper::setColor(const tt::engine::renderer::ColorRGBA& p_color)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setColor(p_color);
	}
}


void TextLabelWrapper::setText(const std::string& p_text)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setText(tt::str::widen(p_text));
	}
}


void TextLabelWrapper::setTextUTF8(const std::string& p_text)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setText(tt::str::utf8ToUtf16(p_text));
	}
}


void TextLabelWrapper::setTextLocalized(const std::string& p_localizationKey)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setTextLocalized(p_localizationKey);
	}
}


void TextLabelWrapper::setTextLocalizedAndFormatted(const std::string& p_localizationKey,
                                                    const tt::str::Strings& p_vars)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setTextLocalizedAndFormatted(p_localizationKey, p_vars);
	}
}


std::string TextLabelWrapper::getTextUTF8() const
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		return tt::str::utf16ToUtf8(tl->getText());
	}
	return std::string();
}


s32 TextLabelWrapper::getTextLength() const
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		return tl->getTextLength();
	}
	return -1;
}


s32 TextLabelWrapper::getWCharAtIndex(s32 p_index) const
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0 && p_index >= 0 && p_index < tl->getTextLength())
	{
		return tl->getText().at(p_index);
	}
	return -1;
}


s32 TextLabelWrapper::getNumberOfVisibleCharacters() const
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		return tl->getNumberOfVisibleCharacters();
	}
	return -1;
}


void TextLabelWrapper::setNumberOfVisibleCharacters(s32 p_numberOfVisibleCharacters)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setNumberOfVisibleCharacters(p_numberOfVisibleCharacters);
	}
}


tt::math::VectorRect TextLabelWrapper::getUsedSize()
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		return tl->getOrCalculateUsedSize();
	}
	return tt::math::VectorRect();
}


void TextLabelWrapper::setSize(real p_width, real p_height)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setSize(p_width, p_height);
	}
}


void TextLabelWrapper::setScale(real p_scale)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setScale(p_scale);
	}
}



void TextLabelWrapper::setVerticalAlignment(  entity::graphics::VerticalAlignment   p_verticalAlignment)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setVerticalAlignment(p_verticalAlignment);
	}
}


void TextLabelWrapper::setHorizontalAlignment(entity::graphics::HorizontalAlignment p_horizontalAlignment)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->setHorizontalAlignment(p_horizontalAlignment);
	}
}


void TextLabelWrapper::addDropShadow(const tt::math::Vector2& p_offset, const tt::engine::renderer::ColorRGBA& p_color)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->addDropShadow(p_offset, p_color);
	}
}


void TextLabelWrapper::removeDropShadow()
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->removeDropShadow();
	}
}


void TextLabelWrapper::fadeIn( real p_time)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->fadeIn(p_time);
	}
}


void TextLabelWrapper::fadeOut(real p_time)
{
	entity::graphics::TextLabel* tl = m_handle.getPtr();
	if (tl != 0)
	{
		tl->fadeOut(p_time);
	}
}


void TextLabelWrapper::serialize(tt::code::BufferWriteContext* p_context) const
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	bu::putHandle(m_handle, p_context);
}


void TextLabelWrapper::unserialize(tt::code::BufferReadContext* p_context)
{
	TT_NULL_ASSERT(p_context);
	
	namespace bu = tt::code::bufferutils;
	
	m_handle = bu::getHandle<entity::graphics::TextLabel>(p_context);
}


void TextLabelWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(TextLabelWrapper, "TextLabel");
	TT_SQBIND_METHOD(TextLabelWrapper, setShowTextBorders);
	TT_SQBIND_METHOD(TextLabelWrapper, setColor);
	TT_SQBIND_METHOD(TextLabelWrapper, setText);
	TT_SQBIND_METHOD(TextLabelWrapper, setTextUTF8);
	TT_SQBIND_METHOD(TextLabelWrapper, setTextLocalized);
	TT_SQBIND_METHOD(TextLabelWrapper, setTextLocalizedAndFormatted);
	TT_SQBIND_METHOD(TextLabelWrapper, getTextUTF8);
	TT_SQBIND_METHOD(TextLabelWrapper, getTextLength);
	TT_SQBIND_METHOD(TextLabelWrapper, getWCharAtIndex);
	TT_SQBIND_METHOD(TextLabelWrapper, getNumberOfVisibleCharacters);
	TT_SQBIND_METHOD(TextLabelWrapper, setNumberOfVisibleCharacters);
	TT_SQBIND_METHOD(TextLabelWrapper, getUsedSize);
	TT_SQBIND_METHOD(TextLabelWrapper, setSize);
	TT_SQBIND_METHOD(TextLabelWrapper, setScale);
	TT_SQBIND_METHOD(TextLabelWrapper, setVerticalAlignment);
	TT_SQBIND_METHOD(TextLabelWrapper, setHorizontalAlignment);
	TT_SQBIND_METHOD(TextLabelWrapper, addDropShadow);
	TT_SQBIND_METHOD(TextLabelWrapper, removeDropShadow);
	TT_SQBIND_METHOD(TextLabelWrapper, fadeIn);
	TT_SQBIND_METHOD(TextLabelWrapper, fadeOut);
}

// Namespace end
}
}
}
}
