#include <toki/game/script/wrappers/DebugViewWrapper.h>
#include <toki/game/script/EntityBase.h>
#include <toki/AppGlobal.h>
#include <toki/game/Game.h>
#include <toki/game/DebugView.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {

// Global debug color
tt::engine::renderer::ColorRGBA DebugViewWrapper::ms_color = tt::engine::renderer::ColorRGB::red;


void DebugViewWrapper::setColor(u8 p_red, u8 p_green, u8 p_blue, u8 p_alpha)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	ms_color.setColor(p_red, p_green, p_blue, p_alpha);
#else
	(void)p_red;
	(void)p_green;
	(void)p_blue;
	(void)p_alpha;
#endif
}


void DebugViewWrapper::drawCircle(const tt::math::Vector2& p_position, real p_radius, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::CircleInfo circle(p_position, p_radius, false, ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerCircle(circle);
#else
	(void)p_position;
	(void)p_radius;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawFilledCircle(const tt::math::Vector2& p_position, real p_radius, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::CircleInfo circle(p_position, p_radius, true, ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerCircle(circle);
#else
	(void)p_position;
	(void)p_radius;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawCirclePart(const tt::math::Vector2& p_position, real p_radius,
	                                  real p_startAngle, real p_endAngle, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::CirclePartInfo circlePart(p_position,
	                                     p_radius,
	                                     tt::math::degToRad(p_startAngle),
	                                     tt::math::degToRad(p_endAngle),
	                                     ms_color,
	                                     p_lifetime);
	AppGlobal::getGame()->getDebugView().registerCirclePart(circlePart);
#else
	(void)p_position;
	(void)p_radius;
	(void)p_startAngle;
	(void)p_endAngle;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawLine(const tt::math::Vector2& p_start, const tt::math::Vector2& p_end, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::LineInfo line(p_start, p_end, ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerLine(line);
#else
	(void)p_start;
	(void)p_end;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawLineBetween(const EntityBase* p_from, const EntityBase* p_to, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	TT_NULL_ASSERT(p_from);
	TT_NULL_ASSERT(p_to);

	DebugView::EntityLineInfo entityLine(p_from->getHandle(), p_to->getHandle(),
		tt::math::Vector2::zero, tt::math::Vector2::zero, ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerEntityLine(entityLine);
#else
	(void)p_from;
	(void)p_to;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawLineBetweenEx(const EntityBase* p_from, const tt::math::Vector2& p_fromOffset,
		                                 const EntityBase* p_to,   const tt::math::Vector2& p_toOffset, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::EntityLineInfo entityLine(
		p_from == 0 ? entity::EntityHandle() : p_from->getHandle(),
		p_to   == 0 ? entity::EntityHandle() : p_to->getHandle(),
		p_fromOffset,
		p_toOffset, ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerEntityLine(entityLine);
#else
	(void)p_from;
	(void)p_fromOffset;
	(void)p_to;
	(void)p_toOffset;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawRect(const tt::math::Vector2& p_topLeft, const tt::math::Vector2& p_bottomRight, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	using tt::math::Vector2;

	// Top
	DebugView::LineInfo line(p_topLeft, Vector2(p_bottomRight.x, p_topLeft.y), ms_color, p_lifetime);
	AppGlobal::getGame()->getDebugView().registerLine(line);

	// Left
	line.targetPosition = Vector2(p_topLeft.x, p_bottomRight.y);
	AppGlobal::getGame()->getDebugView().registerLine(line);

	// Bottom
	line.sourcePosition = p_bottomRight;
	AppGlobal::getGame()->getDebugView().registerLine(line);

	// Right
	line.targetPosition = Vector2(p_bottomRight.x, p_topLeft.y);
	AppGlobal::getGame()->getDebugView().registerLine(line);
#else
	(void)p_topLeft;
	(void)p_bottomRight;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawTextInWorld(const tt::math::Vector2& p_position, const std::string& p_text, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	DebugView::TextInfo text(p_position, p_text, true, ms_color.rgb(), p_lifetime, true);
	AppGlobal::getGame()->getDebugView().registerText(text);
#else
	(void)p_position;
	(void)p_text;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::drawText(s32 p_x, s32 p_y, const std::string& p_text, real p_lifetime)
{
#if !defined(TT_BUILD_FINAL) && defined(TT_PLATFORM_WIN)
	// Convert to screen coordinates
	tt::math::Vector2 position(static_cast<real>(p_x), static_cast<real>(p_y));

	DebugView::TextInfo text(position, p_text, true, ms_color.rgb(), p_lifetime, false);
	AppGlobal::getGame()->getDebugView().registerText(text);
#else
	(void)p_x;
	(void)p_y;
	(void)p_text;
	(void)p_lifetime;
#endif
}


void DebugViewWrapper::bind(const tt::script::VirtualMachinePtr& p_vm)
{
	TT_SQBIND_SETVM(p_vm);
	
	TT_SQBIND_INIT_NAME(DebugViewWrapper, "DebugView");
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, setColor);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawCircle);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawFilledCircle);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawCirclePart);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawLine);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawLineBetween);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawLineBetweenEx);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawRect);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawTextInWorld);
	TT_SQBIND_STATIC_METHOD(DebugViewWrapper, drawText);
}


// Namespace end
}
}
}
}
