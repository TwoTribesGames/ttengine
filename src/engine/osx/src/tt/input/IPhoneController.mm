#if defined(TT_PLATFORM_OSX_IPHONE)

#import <UIKit/UIKit.h>

#include <tt/code/helpers.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/input/IPhoneController.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


//--------------------------------------------------------------------------------------------------
// Objective-C AccelerationController interface

/*
@interface ObjectiveCIPhoneController : UIView <UIAccelerometerDelegate>
{
@private
	bool fingerOneOnScreen;
	bool fingerTwoOnScreen;
	
	CGPoint fingerOneLocation; // Coordinates for finger 1
	CGPoint fingerTwoLocation; // Coordinates for finger 2
	
	UIAccelerationValue x; //doubles
	UIAccelerationValue y;
	UIAccelerationValue z;
}

- (id)initWithSuperView:(UIView*)view;
// Touch handling functions.
- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event;
- (void)updateCoordinates:(NSSet*)touches withEvent:(UIEvent*)event nrOfTouches:(int)p_nrOfTouches;

// Acceleration function.
- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration;


@property(readonly) CGPoint fingerOneLocation;
@property(readonly) CGPoint fingerTwoLocation;
@property bool fingerOneOnScreen;
@property bool fingerTwoOnScreen;

@property(readonly) UIAccelerationValue x;
@property(readonly) UIAccelerationValue y;
@property(readonly) UIAccelerationValue z;

@end


//--------------------------------------------------------------------------------------------------
// Objective-C AccelerationController implementation

@implementation ObjectiveCIPhoneController

// Generate the getters and setters of the properties.
@synthesize fingerOneLocation, fingerTwoLocation, fingerOneOnScreen, fingerTwoOnScreen, x, y, z;

- (id)initWithSuperView:(UIView*)view
{
	self = [super initWithFrame:view.bounds];
	if (self)
	{
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		[view addSubview:self];
		fingerOneOnScreen = false;
		fingerTwoOnScreen = false;
	}
	return self;
}


- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
	int actualTouches = 0;
	const int touchCount = [[event allTouches] count];
	for (int i = 0; i < touchCount; ++i)
	{
		UITouchPhase phase = [[[[event allTouches] allObjects] objectAtIndex: i] phase];
		if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
		{
			++actualTouches;
		}
	}
	
	fingerOneOnScreen = actualTouches > 0;
	fingerTwoOnScreen = actualTouches > 1;
	
	[self updateCoordinates:touches withEvent:event nrOfTouches:actualTouches];
}


- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
	[self updateCoordinates:touches withEvent:event nrOfTouches:[[event allTouches] count]];
}


- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event 
{
	(void)touches;
	int touchCount = [[event allTouches] count];
	int stillTouching = 0;
	for (int i = 0; i < touchCount; ++i)
	{
		UITouchPhase phase = [[[[event allTouches] allObjects] objectAtIndex: i] phase];
		if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
		{
			++stillTouching;
		}
	}
	
	if (stillTouching > 1)
	{
		fingerOneOnScreen = true;
		fingerTwoOnScreen = true;
	}
	else if (stillTouching == 1)
	{
		fingerOneOnScreen = true;
		fingerTwoOnScreen = false;
	}
	else
	{
		fingerOneOnScreen = false;
		fingerTwoOnScreen = false;
	}
	
	[self updateCoordinates:touches withEvent:event nrOfTouches:stillTouching];
}


- (void)updateCoordinates:(NSSet*)touches withEvent:(UIEvent*)event nrOfTouches:(int)p_nrOfTouches
{
	// When one finger is on the screen.
	if (p_nrOfTouches == 1)
	{
		// Find the UITouch that is still on screen
		const int touchCount = [[event allTouches] count];
		for (int i = 0; i < touchCount; ++i)
		{
			UITouch* touch = [[[event allTouches] allObjects] objectAtIndex: i];
			UITouchPhase phase = [touch phase];
			if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
			{
				// Use this touch's location
				fingerOneLocation = [touch locationInView:self];
				break;
			}
		}
	}
	// When two fingers are on the screen.
	else if (p_nrOfTouches > 1)
	{
		// Find the UITouches that are still on screen
		const int touchCount = [[event allTouches] count];
		int fingerIndex = 0;
		for (int i = 0; i < touchCount; ++i)
		{
			UITouch*     touch = [[[event allTouches] allObjects] objectAtIndex: i];
			UITouchPhase phase = [touch phase];
			if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
			{
				// Use this touch's location
				if (fingerIndex == 0)
				{
					// Touch one
					fingerOneLocation = [touch locationInView:self];
					++fingerIndex;
				}
				else if (fingerIndex == 1)
				{
					// Touch two
					fingerTwoLocation = [touch locationInView:self];
					break;
				}
			}
		}
	}
}


- (void)accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration
{
	(void)accelerometer;
	x = acceleration.x;
	y = acceleration.y;
	z = acceleration.z;
}

@end
//*/


//--------------------------------------------------------------------------------------------------
// Start C++ implementation

namespace tt {
namespace input {

bool                               IPhoneController::ms_initialized = false;
IPhoneController                   IPhoneController::ms_controller;
IPhoneController                   IPhoneController::ms_temporary;
Touch                              IPhoneController::ms_touchDummy;

//ObjectiveCIPhoneController* g_objectiveCIPhoneController = 0;


bool IPhoneController::isConnected(ControllerIndex p_index)
{
	return p_index == ControllerIndex_One && isInitialized();
}


const IPhoneController& IPhoneController::getState(ControllerIndex p_index)
{
	TT_ASSERTMSG(isInitialized(), "IPhoneController has not been initialized yet.");
	TT_ASSERTMSG(p_index == ControllerIndex_One, "Invalid controller index: %d", p_index);
	return ms_controller;
}


void IPhoneController::update()
{
	if (isInitialized() == false)
	{
		TT_PANIC("IPhoneController has not been initialized yet.");
		return;
	}
	
	ms_controller.touch1Status.update(ms_temporary.touch1Status.down);
	ms_controller.touch2Status.update(ms_temporary.touch2Status.down);
	ms_controller.touch3Status.update(ms_temporary.touch3Status.down);
	
	ms_controller.touch1Location = ms_temporary.touch1Location;
	ms_controller.touch2Location = ms_temporary.touch2Location;
	ms_controller.touch3Location = ms_temporary.touch3Location;
	
	ms_controller.accelerometer  = ms_temporary.accelerometer;
	
	//TT_Printf("IPhoneController::update\n");
	
	ms_controller.removeOldTouches();
	
	// Update or add new touches. (From temporary to controller)
	ms_controller.updateOrAddTouches(ms_temporary);
	
	ms_temporary.removeOldTouches();
}


bool IPhoneController::isInitialized()
{
	return ms_initialized;
	//return g_objectiveCIPhoneController != 0;
}


bool IPhoneController::initialize(/*float p_accelerometerFrequency*/)
{
	if (isInitialized())
	{
		TT_PANIC("IPhoneController is already initialized.");
		return false;
	}
	
	/* FIXME: Restructure IPhoneController so that the Objective C view is in the app framework
	// Create the Objective-C class that will receive the input data.
	g_objectiveCIPhoneController = [[ObjectiveCIPhoneController alloc]
		initWithSuperView:((UIView*)tt::engine::renderer::Renderer::getInstance()->getUIView())];
	
	// Set up accelerometer.
	[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0f / p_accelerometerFrequency)];
	
	// Set the objectiveCIPhoneHandler to handle the accelerometer events.
	[[UIAccelerometer sharedAccelerometer] setDelegate:g_objectiveCIPhoneController];
	//*/
	
	ms_initialized = true;
	
	return true;
}


void IPhoneController::deinitialize()
{
	TT_ASSERTMSG(isInitialized(), "IPhoneController has not been initialized yet.");
	
	/*
	if (g_objectiveCIPhoneController != 0)
	{
		[g_objectiveCIPhoneController dealloc];
		g_objectiveCIPhoneController = 0;
	}
	*/
	
	ms_initialized = false;
}


void IPhoneController::appView_startTouch(Touch::ID p_id, const tt::math::Point2& p_location)
{
	// Make sure this timestamp isn't already used.
	TT_ASSERTMSG(hasTouch(p_id) == false,
	             "Starting touch for UITouchID (%p) which already exists! (current touches count: %u)",
	             p_id, allTouches.size());
	
	// Add new touch
	allTouches.push_back(Touch(p_id, p_location));
}
	
	
Touch& IPhoneController::appView_modifyTouch(Touch::ID p_id)
{
	for (Touches::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
	{
		if (it->id == p_id)
		{
			return (*it);
		}
	}
	
	TT_PANIC("Couldn't find touch for ID: %d", p_id);
	ms_touchDummy.reset(); // Reset incase previous call changed it. (Extra failsafe.)
	return ms_touchDummy;
}


bool IPhoneController::hasTouch(Touch::ID p_id) const
{
	for (Touches::const_iterator it = allTouches.begin(); it != allTouches.end(); ++it)
	{
		if (it->id == p_id)
		{
			return true;
		}
	}
	return false;
}

	
const Touch& IPhoneController::getTouch(Touch::ID p_id) const
{
	for (Touches::const_iterator it = allTouches.begin(); it != allTouches.end(); ++it)
	{
		if (it->id == p_id)
		{
			return (*it);
		}
	}
	
	TT_PANIC("Couldn't find touch for ID: %d", p_id);
	ms_touchDummy.reset(); // Reset incase previous call changed it. (Extra failsafe.)
	return ms_touchDummy;
}


void IPhoneController::updateOrAddTouches(const IPhoneController& p_otherController)
{
	// Updated existing touches
	for (Touches::iterator it = allTouches.begin(); it != allTouches.end(); ++it)
	{
		// FIXME: In theory it's possible that a touch has ended and a new one with same id has started.
		//        So this isn't thread safe, but it should not be an issue with the current OS implementation.
		
		if (p_otherController.hasTouch(it->id))
		{
			const Touch& touch = p_otherController.getTouch(it->id);
			if (touch.status.down)
			{
				(*it).updateLocation(touch);
			}
			else
			{
				(*it).updateRelease();
			}
		}
		else
		{
			// This ID is no longer found. release it.
			(*it).updateRelease();
		}
	}
	
	// 
	for (Touches::const_iterator it = p_otherController.allTouches.begin(); it != p_otherController.allTouches.end(); ++it)
	{
		if (hasTouch(it->id) == false)
		{
			const Touch& touch = (*it);
			// Add new touch
			// 
			// Don't copy construct from p_touch, but create new touch based on p_touch settings
			// (This is to make  sure pressed and released get correct state.)
			Touch newTouch(touch.id, touch);
			newTouch.status.released = touch.status.released;
			allTouches.push_back(newTouch);
		}
	}
}


void IPhoneController::removeOldTouches()
{
	for (Touches::iterator it = allTouches.begin(); it != allTouches.end();)
	{
		if (it->status.down == false)
		{
			it = allTouches.erase(it);
		}
		else
		{
			++it;
		}
	}
}


// Namespace end
}
}


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
