#if !defined(INC_TT_SCRIPT_BINDINGS_H)
#define INC_TT_SCRIPT_BINDINGS_H

#include <tt/app/Platform.h>
#include <tt/engine/renderer/ColorRGB.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/input/ControllerType.h>
#include <tt/input/KeyList.h>

#include <tt/script/helpers.h>
#include <tt/script/VirtualMachine.h>

#include <tt/math/interpolation.h>
#include <tt/math/Rect.h>
#include <tt/math/Vector2.h>

#include <tt/score/Score.h>
#include <tt/stats/stats.h>


// Scripting-specific documentation for the types bound here

/*! \namespace tt
    \brief Root of the Two Tribes libraries. */

///////// ColorRGB //////////

/*! \namespace tt::engine
    \brief Two Tribes engine */

/*! \namespace tt::engine::renderer
    \brief Two Tribes Renderer */

/*! \class tt::engine::renderer::ColorRGB
    \brief Color with 3 channels. (with int values 0 to 255 per channel). 
    <h2>Public Member Functions</h2>
    <p><strong>ColorRGB(u8 p_r, u8 p_g, u8 p_b)</strong><br/>
    Constructs a new ColorRGB.</p> */

/*! \var u8 tt::engine::renderer::ColorRGB::r
    \brief r Red*/
/*! \var u8 tt::engine::renderer::ColorRGB::g
    \brief g Green*/
/*! \var u8 tt::engine::renderer::ColorRGB::b
    \brief b Blue*/

///////// ColorRGBA //////////

/*! \class tt::engine::renderer::ColorRGBA
    \brief Color with 4 channels. (with int values 0 to 255 per channel). 
    <h2>Public Member Functions</h2>
    <p><strong>ColorRGBA(u8 p_r, u8 p_g, u8 p_b, u8 p_a = 255)</strong><br/>
    Constructs a new ColorRGBA from separate color channels (with optional alpha).</p>
    <p><strong>ColorRGBA(const tt::engine::renderer::ColorRGB& p_rgb, u8 p_a = 255)</strong><br/>
    Constructs a new ColorRGBA from a ColorRGB and optional alpha.</p> */

/*! \var u8 tt::engine::renderer::ColorRGBA::r
    \brief r Red*/
/*! \var u8 tt::engine::renderer::ColorRGBA::g
    \brief g Green*/
/*! \var u8 tt::engine::renderer::ColorRGBA::b
    \brief b Blue*/
/*! \var u8 tt::engine::renderer::ColorRGBA::a
    \brief b Alpha / opacity*/

///////// Vector2 //////////

/*! \namespace tt::math
    \brief Two Tribes math library. */

/*! \class tt::math::Vector2
    \brief Two-component vector.
	
	<h2>Public Member Functions</h2>
	<p><strong>Vector2(real p_x, real p_y)</strong><br/>
	Constructs a new Vector2.</p>
	
	<p><strong>tt::math::Vector2 normalize()</strong><br/>
	Normalizes this Vector2 and returns itself.</p>
	
	<p><strong>tt::math::Vector2 getNormalized() const</strong><br/>
	Returns a Vector2 with the value of this Vector2 normalized.</p>
	
	<p><strong>real length() const</strong><br/>
	Returns the length of this Vector2.</p>
	
	<p><strong>real lengthSquared() const</strong><br/>
	Returns the length^2 of this Vector2.</p>
	
	//rotate
	//getRotated
	//getAngle
*/


/*! \var real tt::math::Vector2::x
    \brief x */
/*! \var real tt::math::Vector2::y
    \brief y */


/*! \fn real tt::math::distance(const tt::math::Vector2& p_a, const tt::math::Vector2& p_b)
    \brief Calculates the distance between two vectors. */

/*! \fn real tt::math::dotProduct(const tt::math::Vector2& p_a, const tt::math::Vector2& p_b)
    \brief Calculates the dot product of two vectors. */

///////// VectorRect //////////

/*! \class tt::math::VectorRect
    \brief A rectangle with floating point position and size. The position is the center point of the rectangle.

<h2>Public Member Functions</h2>

<p><strong>VectorRect(Vector2 p_centerPos, real p_width, real p_height)</strong><br/>
Constructs a new rectangle with its center point at p_centerPos and of size p_width x p_height.</p>

<p><strong>Vector2 getPosition()</strong><br/>
Returns the center position of the rectangle.</p>

<p><strong>void setPosition(Vector2 p_centerPos)</strong><br/>
Sets the center position for the rectangle. Does not change the width or height.</p>

<p><strong>real getWidth()</strong><br/>
Returns the width of the rectangle.</p>

<p><strong>void setWidth(real p_width)</strong><br/>
Sets the width of the rectangle. Does not change the position.</p>

<p><strong>real getHeight()</strong><br/>
Returns the height of the rectangle.</p>

<p><strong>void setHeight(real p_height)</strong><br/>
Sets the height of the rectangle. Does not change the position.</p>

<p><strong>void translate(Vector2 p_moveBy)</strong><br/>
Moves the rectangle position by <strong>p_moveBy</strong>.</p>

<p><strong>Vector2 getMin()</strong><br/>
Gets the minimum position of the rectangle (with the lowest X and Y values): the bottom-left position in Toki Tori 2.</p>

<p><strong>Vector2 getMaxInside()</strong><br/>
Gets the maximum position inside the rectangle (the highest X and Y values that are still inside the rectangle): the top-right position in Toki Tori 2.</p>

<p><strong>Vector2 getMaxEdge()</strong><br/>
Gets the maximum position at the edge of the rectangle (the highest X and Y values that are still inside the rectangle): the top-right position in Toki Tori 2.</p>

<p><strong>bool containsCircle(const Vector2& p_point, real p_radius)</strong><br/>
Returns true if this rect fully contains the circle specified by p_point and p_radius.</p>

<p><strong>bool intersectsCircle(const Vector2& p_point, real p_radius)</strong><br/>
Returns true if this rect (partially) intersects the circle specified by p_point and p_radius.</p>

<p><strong>bool containsRect(const VectorRect& p_rect)</strong><br/>
Returns true if this rect fully contains the rect specified by p_rect.</p>

<p><strong>bool intersectsRect(const VectorRect& p_rect, VectorRect* p_intersectionRect)</strong><br/>
Returns true if this rect (partially) intersects the specified p_rect.<br/>
p_intersectionRect Optional pointer to a Rect that will receive the rectangle of intersection.
May be null and will not be used in that case.
</p>
*/

///////// ControllerType //////////

/*! \namespace tt::input
    \brief  Two Tribes input library */

/*! \fn tt::input::ControllerType tt::input::getCurrentControllerType()
	\brief Returns the controller type that is currently used to play with
	\return The ControllerType (keyboard, generic controller or xbox360 gamepad) */

///////// Platform //////////

/*! \namespace tt::app
    \brief  Two Tribes application library */

/*! \fn tt::app::Platform tt::app::getPlatform()
	\brief Returns the current platform the game is played on
	\return The Platform enum value (eg. Platform_WIN, Platform_MAC)*/


#ifdef SQBIND_NAMESPACE
namespace SQBIND_NAMESPACE {
#endif

// Bind enum types to Squirrel
SQBIND_INTEGER(tt::math::interpolation::EasingType);
SQBIND_INTEGER(tt::input::ControllerType);
SQBIND_INTEGER(tt::app::Platform);
SQBIND_INTEGER(tt::score::DownloadRequestSource);
SQBIND_INTEGER(tt::score::DownloadRequestRangeType);
SQBIND_INTEGER(tt::input::Key);
SQBIND_INTEGER(tt::stats::StatType);


#ifdef SQBIND_NAMESPACE
}; // end of sqbind namespace
#endif


namespace tt       /*! */ {
namespace script   /*! */ {
namespace bindings /*! */ {

tt::engine::renderer::ColorRGB* colorRGB_constructor(HSQUIRRELVM v);
void bindColorRGB(const tt::script::VirtualMachinePtr& p_vm);

tt::engine::renderer::ColorRGBA* colorRGBA_constructor(HSQUIRRELVM v);
void bindColorRGBA(const tt::script::VirtualMachinePtr& p_vm);

tt::math::Vector2* vector2_constructor(HSQUIRRELVM v);
void bindVector2(const tt::script::VirtualMachinePtr& p_vm);

tt::math::VectorRect* vectorRect_constructor(HSQUIRRELVM v);
void bindVectorRect(const tt::script::VirtualMachinePtr& p_vm);

void bindInterpolation(const tt::script::VirtualMachinePtr& p_vm);

void bindRandom(const tt::script::VirtualMachinePtr& p_vm);

void bindLeaderboardsTypes(const tt::script::VirtualMachinePtr& p_vm);

void bindStatsTypes(const tt::script::VirtualMachinePtr& p_vm);

void bindKeyTypes(const tt::script::VirtualMachinePtr& p_vm);

/*! \brief Returns a floating-point random number in range [0 - 1]. */
real frnd();

/*! \brief Returns a floating-point random number in range [p_lower - p_upper]
    \param p_lower The minimum number to return.
    \param p_upper The upper limit of the number to return (the result of this function will never be equal to or higher than p_upper). */
real frnd_minmax(real p_lower, real p_upper);

/*! \brief Returns an integer random number in range [0 - MAX_INT]. */
u32 rnd();

/*! \brief Returns an integer random number in range [p_lower - p_upper]
    \param p_lower The minimum number to return.
    \param p_upper The upper limit of the number to return (the result of this function can be equal to p_upper). */
u32 rnd_minmax(u32 p_lower, u32 p_upper);

void bindTime(const tt::script::VirtualMachinePtr& p_vm);

/*! \brief 'Time' in Squirrel. */
class Time
{
public:
	/*! \brief Returns number of milliseconds since startup of application of platform (depends on system).
	           NOTE: This number is casted down from a 64 bit number, so it overflows every 50 days. */
	static u32 getMilliSeconds();
	
	/*! \brief Returns number of seconds since startup of application of platform (depends on system). */
	static u32 getSeconds();
	
	/*! \brief Returns time in human readable form */
	static std::string getNowAsString();
};

void bindInput(const tt::script::VirtualMachinePtr& p_vm);

void bindPlatform(const tt::script::VirtualMachinePtr& p_vm);

void bindSharedWrappers(const tt::script::VirtualMachinePtr& p_vm);


// Namespace end
}
}
}


#endif  // !defined(INC_TT_SCRIPT_BINDINGS_H)
