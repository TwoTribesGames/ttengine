/*
*/

function getEasingTypes()
{
	return [
	"Linear",
	"QuadraticIn",
	"QuadraticOut",
	"QuadraticInOut",
	"CubicIn",
	"CubicOut",
	"CubicInOut",
	"QuarticIn",
	"QuarticOut",
	"QuarticInOut",
	"QuinticIn",
	"QuinticOut",
	"QuinticInOut",
	"SinusoidalIn",
	"SinusoidalOut",
	"SinusoidalInOut",
	"ExponentialIn",
	"ExponentialOut",
	"ExponentialInOut",
	"CircularIn",
	"CircularOut",
	"CircularInOut",
	"BackIn",
	"BackOut",
	"BackInOut"];
}

// set camera follow stacked (so multiple calls should restore it nicely (unless you create under- or overflow)
::g_cameraFollowStack <- 0; // assuming the camera is following when we start the level
::g_primaryFollowEntity <- null;
::g_fovShakerCount <- 0;

function Camera::resetFollowStack()
{
	::g_cameraFollowStack = 0;
	::Camera.setFollowEntity(null);
	::Camera.setScrollingEnabled(false);
	::DrcCamera.setScrollingEnabled(false);
}


// The game can only focus on 1 primary entity. This will be our base entity for the stack. Call this first
function Camera::setPrimaryFollowEntity(p_entity)
{
	resetFollowStack();
	::g_primaryFollowEntity = p_entity.weakref();
	setFollowEntityStacked(true);
}

function Camera::getPrimaryFollowEntity()
{
	return ::g_primaryFollowEntity;
}

function Camera::setFollowEntityStacked(p_follow)
{
	::g_cameraFollowStack += (p_follow ? 1 : -1);
	//echo(::g_cameraFollowStack, "stack");
	//don't use this assert because the stack can dip below zero if something stops the camera from following before toki sets it to follow him
	//::assert(::g_cameraFollowStack >= 0, "::Camera follow stack underflow!");
	
	if (::g_cameraFollowStack == 1)
	{
		setFollowEntity(::g_primaryFollowEntity);
		
		//::Camera.setScrollingEnabled(true);
		//DrcCamera.setScrollingEnabled(true);
	}
	else if (::g_cameraFollowStack == 0)
	{
		resetFollowStack();
	}
}

///////////////////////////////////////////////////////////////////////////////
// Shake helpers

function Camera::shakePosition(p_position, p_duration, p_amplitude, p_frequency, p_shakeHUD, p_positionCulling)
{
	return ::spawnEntity("CameraPositionShaker",
		p_position,
		{
			_amplitude       = p_amplitude,
			_duration        = p_duration,
			_frequency       = p_frequency,
			_shakeHud        = p_shakeHUD,
			_positionCulling = p_positionCulling
		}
	);
}


function Camera::shakeFOV(p_position, p_duration, p_amplitude, p_positionCulling, p_easingType = EasingType_QuadraticInOut)
{
	return ::spawnEntity("CameraFOVShaker",
		p_position,
		{
			_amplitude       = p_amplitude,
			_duration        = p_duration,
			_positionCulling = p_positionCulling,
			_easingType      = p_easingType
		}
	);
}


function Camera::shakeRotation(p_position, p_duration, p_amplitude, p_positionCulling, p_easingType = EasingType_QuadraticInOut)
{
	return ::spawnEntity("CameraRotationShaker",
		p_position,
		{
			_amplitude       = p_amplitude,
			_duration        = p_duration,
			_positionCulling = p_positionCulling,
			_easingType      = p_easingType
		}
	);
}


enum CameraShakeFlag
{
	Position        = 0x1,
	Rumble          = 0x2,
	Rotation        = 0x4,
	FOV             = 0x8,
	HUD             = 0x10,
	RumbleNoPanning = 0x20,
	
	// Presets
	DefaultPlayer  = 0x1F, // Position | Rumble | Rotation | FOV | HUD,
	Default        = 0xF,  // Position | Rumble | Rotation | FOV,
	NoRotation     = 0xB,  // Position | Rumble | FOV,
	NoRumble       = 0xD,  // Position | Rotation | FOV,
	PositionAndFOV = 0x9,  // Position | FOV,
	AnyRumble      = 0x22, // Rumble | RumbleNoPanning
	All            = 0xFF,
};


function Camera::reset()
{
	resetFollowStack();
	
	// FOV Shaker limiter hack
	::g_fovShakerCount = 0;
}


function Camera::shakeScreen(p_power, p_position, p_flags = CameraShakeFlag.Default, p_positionCulling = true)
{
	// Scale power according to current FOV, base is 60 since that's average
	p_power *= (::Camera.getTargetFOV() / 60.0);
	
	// Handle rumble first, otherwise it will be skipped when ScreenShake is off
	if (p_flags & CameraShakeFlag.AnyRumble)
	{
		local strength = RumbleStrength_High;
		if (p_power <= 0.33)
		{
			strength = RumbleStrength_Low;
		}
		else if (p_power <= 0.66)
		{
			strength = RumbleStrength_Medium;
		}
		local panning = (p_flags & CameraShakeFlag.RumbleNoPanning) ? 0.0 : ::getRumblePanning(strength, p_position);
		::setRumble(strength, panning);
	}
	
	if (::OptionsData.get(::OptionsKeys.ScreenShake) == false)
	{
		return;
	}
	
	if (p_flags & CameraShakeFlag.Position)
	{
		local shakeHUD = (p_flags & CameraShakeFlag.HUD) != 0;
		::Camera.shakePosition(p_position, 0.2, ::Vector2(p_power * 0.02, p_power * 0.25), 40, shakeHUD, p_positionCulling);
		::Camera.shakePosition(p_position, 0.35, ::Vector2(p_power * 0.02, p_power * 0.125), 20, shakeHUD, p_positionCulling);
	}
	
	if (p_flags & CameraShakeFlag.Rotation)
	{
		local rotation = ::frnd_minmax(p_power * 4, p_power * 9) * (brnd() ? -1 : 1);
		::Camera.shakeRotation(p_position, 0.2, rotation, p_positionCulling);
	}
	
	if (p_flags & CameraShakeFlag.FOV)
	{
		if (p_power <= 0.33)
		{
			::Camera.shakeFOV(p_position, 0.075, p_power * 7.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.150, p_power * 5.0, p_positionCulling);
		}
		else if (p_power <= 0.66)
		{
			::Camera.shakeFOV(p_position, 0.075, p_power * 9.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.150, p_power * 7.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.225, p_power * 5.0, p_positionCulling);
		}
		else
		{
			::Camera.shakeFOV(p_position, 0.075, p_power * 11.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.150, p_power *  9.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.225, p_power *  7.0, p_positionCulling);
			::Camera.shakeFOV(p_position, 0.300, p_power *  5.0, p_positionCulling);
		}
	}
}
