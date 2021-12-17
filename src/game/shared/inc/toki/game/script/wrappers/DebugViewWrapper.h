#if !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_DEBUGVIEWWRAPPER_H)
#define INC_TOKITORI_GAME_SCRIPT_WRAPPERS_DEBUGVIEWWRAPPER_H

#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/math/Vector2.h>
#include <tt/script/helpers.h>
#include <toki/game/script/EntityBase.h>

namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'DebugView' in Squirrel. Used to draw debug graphics */
class DebugViewWrapper
{
public:
	DebugViewWrapper() {}
	~DebugViewWrapper() {}

	/*! \brief Set the global debug color. All DebugView draw calls afterwards will use this color.
	    \param p_red Red color channel value (0-255)
	    \param p_green Green color channel value (0-255)
	    \param p_blue Blue color channel value (0-255)
	    \param p_alpha Alpha channel value (0-255) */
	static void setColor(u8 p_red, u8 p_green, u8 p_blue, u8 p_alpha);

	/*! \brief Draw a circle
	    \param p_position Center of the circle
	    \param p_radius Radius of the circle
	    \param p_lifetime How long to display this circle (in seconds) */
	static void drawCircle(const tt::math::Vector2& p_position, real p_radius, real p_lifetime);

	/*! \brief Draw a filled circle
	    \param p_position Center of the circle
	    \param p_radius Radius of the circle
	    \param p_lifetime How long to display this circle (in seconds) */
	static void drawFilledCircle(const tt::math::Vector2& p_position, real p_radius, real p_lifetime);

	/*! \brief Draw a part of circle
	    \param p_position Center of the circle
	    \param p_radius Radius of the circle
	    \param p_startAngle Angle of the circle to start the part (in degrees) 0 is the top of the circle, degrees increase clockwise
	    \param p_endAngle Angle of the circle to end the part (in degrees)
	    \param p_lifetime How long to display this part (in seconds) */
	static void drawCirclePart(const tt::math::Vector2& p_position, real p_radius,
	                           real p_startAngle, real p_endAngle, real p_lifetime);

	/*! \brief Draw a line
	    \param p_start Start point of the line
	    \param p_end End point of the line
	    \param p_lifetime How long to display this line (in seconds) */
	static void drawLine(const tt::math::Vector2& p_start, const tt::math::Vector2& p_end, real p_lifetime);

	/*! \brief Draw a line between two entities
	    \param p_from Entity to start the line from
	    \param p_to Entity to draw the line to
	    \param p_lifetime How long to display this line (in seconds) */
	static void drawLineBetween(const EntityBase* p_from, const EntityBase* p_to, real p_lifetime);

	/*! \brief Draw a line between two entities
	    \param p_from Entity to start the line with
		\param p_fromOffset Offset from start entity or start position if from entity is 0
	    \param p_to Entity to draw the line to
		\param p_toOffset Offset from end entity or end position if to entity is 0
	    \param p_lifetime How long to display this line (in seconds) */
	static void drawLineBetweenEx(const EntityBase* p_from, const tt::math::Vector2& p_fromOffset,
		                          const EntityBase* p_to,   const tt::math::Vector2& p_toOffset, real p_lifetime);

	/*! \brief Draw a rectangle
	    \param p_topLeft Top left corner of the rectangle
	    \param p_bottomRight Bottom right corner of the rectangle
	    \param p_lifetime How long to display this rectangle (in seconds) */
	static void drawRect(const tt::math::Vector2& p_topLeft, const tt::math::Vector2& p_bottomRight, real p_lifetime);

	/*! \brief Draw some text in world space
	    \param p_position Position of the the text in world space
	    \param p_text The text to display
	    \param p_lifetime How long to display this text (in seconds) */
	static void drawTextInWorld(const tt::math::Vector2& p_position, const std::string& p_text, real p_lifetime);

	/*! \brief Draw some text in screen space
	    \param p_x x position of the the text in screen space
	    \param p_y y position of the the text in screen space
	    \param p_text The text to display
	    \param p_lifetime How long to display this text (in seconds) */
	static void drawText(s32 p_x, s32 p_y, const std::string& p_text, real p_lifetime);


	static void bind(const tt::script::VirtualMachinePtr& p_vm);

private:
	static tt::engine::renderer::ColorRGBA ms_color;
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TOKITORI_GAME_SCRIPT_WRAPPERS_DEBUGVIEWWRAPPER_H)
