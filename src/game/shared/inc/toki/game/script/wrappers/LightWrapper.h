#if !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_LIGHTWRAPPER_H)
#define INC_TOKI_GAME_SCRIPT_WRAPPERS_LIGHTWRAPPER_H


#include <tt/script/VirtualMachine.h>

#include <toki/game/entity/fwd.h>
#include <toki/game/light/fwd.h>
#include <toki/game/script/wrappers/fwd.h>


namespace toki {
namespace game {
namespace script {
namespace wrappers {


/*! \brief 'Light' in Squirrel. */
class LightWrapper
{
public:
	inline LightWrapper() { }
	explicit LightWrapper(const light::LightHandle& p_lightHandle);
	
	// bindings
	
	/*! \brief LevelLightMode */
	enum LevelLightMode
	{
		LevelLightMode_Light,                  //!< Light level
		LevelLightMode_Dark,                   //!< Dark level
		LevelLightMode_LightWithNoStopOnSplit, //!< Light level with no stop on split
		LevelLightMode_DarkWithStopOnSplit,    //!< Dark level with stop on split
		
		LevelLightMode_Count,
		LevelLightMode_Invalid
	};
	
	/*! \brief Returnes whether or not this light is enabled/disabled. */
	bool isEnabled() const;
	
	/*! \brief Enables/disables this light. */
	void setEnabled(bool p_enabled);
	
	/*! \brief Set the direction of the light. (Angle in degrees can be negative.) */
	void setDirection(real p_angle);
	
	/*! \brief Returns whether or not this light affects entities. */
	bool affectsEntities() const;
	
	/*! \brief Enables/disables whether this light affects entities. */
	void setAffectsEntities(bool p_enabled);
	
	/*! \brief Set the rotation speed in degrees. */
	void setTextureRotationSpeed(real p_speedInDegrees);
	
	/*! \brief Gets the (target) offset of this light. (EndValue of linearInterpolation.) */
	tt::math::Vector2 getOffset() const;
	
	/*! \brief Gets the offset of this light. (Current value of linearInterpolation.) */
	tt::math::Vector2 getCurrentOffset() const;
	
	/*! \brief Sets the offset of this light. 
	    \param p_offset the new offset for this light.
	    \param p_duration How fast (in seconds) the light should get this radius. (0 is instant). */
	void setOffset(const tt::math::Vector2& p_offset, real p_duration);
	
	/*! \brief Gets the (target) radius of this light. (EndValue of linearInterpolation.) */
	real getRadius() const;
	
	/*! \brief Gets the radius of this light. (Current value of linearInterpolation.) */
	real getCurrentRadius() const;
	
	/*! \brief Sets the radius of this light.
	    \param p_radius The new radius for this light.
	    \param p_duration How fast (in seconds) the light should get this radius. (0 is instant). */
	void setRadius(real p_radius, real p_duration);
	
	/*! \brief Gets the (target) spread of this light. (range 0..360) (EndValue of linearInterpolation.) */
	real getSpread() const;
	
	/*! \brief Gets the spread of this light. (range 0..360) (Current value of linearInterpolation.) */
	real getCurrentSpread() const;
	
	/*! \brief Set the spread of the light making it a spotlight
	    \param p_spread The new spread for this light (in degrees. Valid range: 0..360)
	    \param p_duration How fast (in seconds) the light should get this spread. (0 is instant). */
	void setSpread(real p_spread, real p_duration);
	
	/*! \brief Gets the (target) strength of this light. (range 0.0 - 1.0) (EndValue of linearInterpolation.) */
	real getStrength() const;
	
	/*! \brief Gets the strength of this light. (range 0.0 - 1.0) (Current value of linearInterpolation.) */
	real getCurrentStrength() const;
	
	/*! \brief Sets the strength of this light.
	    \param p_strength The new strength for this light. (range 0.0 - 1.0)
	    \param p_duration How fast (in seconds) the light should get this strength. (0 is instant). */
	void setStrength(real p_Strength, real p_duration);
	
	/*! \brief Sets the color for this light.
	    \param p_color the color to be used. */
	void setColor(const tt::engine::renderer::ColorRGBA& p_color);
	
	/*! \brief Override the (default) texture.
	    \note Will look in namespace: 'textures.lights'. */
	void setTexture(const std::string p_textureName);
	
	/*! \brief Creates a glow quad around this light
	    \param p_imageName The name of the image. Should be in the textures.lights namespace
	    \param p_scale scale to apply on the radius of its parent light
	    \param p_minRadius minimum radius of this glow
	    \param p_maxRadius maximum radius of this glow
	    \param p_fadeRadius if radius is smaller than this, it will fade linearly to 0 */
	void createGlow(const std::string& p_imageName, real p_scale,
	                real p_minRadius, real p_maxRadius, real p_fadeRadius);
	
	/*! \brief Sets a jitter effect for this light
	    \param p_scaleFrequency The frequency of the scale jitter
	    \param p_scaleAmplitude The amplitude of the scale jitter
	    \param p_positionXFrequency The frequency of the position jitter in X direction
	    \param p_positionXAmplitude The amplitude of the position jitter in X direction
	    \param p_positionYFrequency The frequency of the position jitter in Y direction
	    \param p_positionYAmplitude The amplitude of the position jitter in Y direction */
	void setJitterEffect(real p_scaleFrequency,     real p_scaleAmplitude,
	                     real p_positionXFrequency, real p_positionXAmplitude,
	                     real p_positionYFrequency, real p_positionYAmplitude);
	
	/*! \brief Tests whether this Light is equal to another.
	    \param p_other the other LightWrapper. */
	inline bool equals(const LightWrapper* p_other) const
	{
		return p_other != 0 && m_lightHandle == p_other->m_lightHandle;
	}
	
	/*! \brief Returns the handle value of this light. */
	inline s32 getHandleValue() const { return static_cast<s32>(m_lightHandle.getValue()); }
	
	/*! \brief Static - is level dark? */
	static bool isLevelDark();
	/*! \brief Static - Sets the level light mode */
	static void setLevelLightMode(LevelLightMode p_mode);
	/*! \brief Static - get the ambient light for this level. (Range 0 to 255) */
	static s32 getLevelLightAmbient();
	/*! \brief Static - set the ambient light for this level. (Range 0 to 255. Will get clamped.) 
	    \note This is only used when there is no lightmask shoebox! */
	static void setLevelLightAmbient(s32 p_ambient);
	
	/*! \brief Static - get the string representation from a LevelLightMode. */
	static std::string    getLevelLightModeName(LevelLightMode p_mode);
	/*! \brief Static - get the LevelLightMode from a string representation. */
	static LevelLightMode getLevelLightModeFromName(const std::string& p_name);
	
	static void bind(const tt::script::VirtualMachinePtr& p_vm);
	
	inline       light::LightHandle& getHandle()       { return m_lightHandle; }
	inline const light::LightHandle& getHandle() const { return m_lightHandle; }
	
private:
	light::LightHandle m_lightHandle;
};

// Namespace end
}
}
}
}


#endif  // !defined(INC_TOKI_GAME_SCRIPT_WRAPPERS_LIGHTWRAPPER_H)
