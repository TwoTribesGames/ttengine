#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iPhone builds only

#include <tt/app/objc_helpers/UIApplication.h>
#include <tt/app/OsxApp_iphone.h>
#include <tt/app/TTdevObjCAppView_iphone.h>
#include <tt/engine/renderer/Renderer.h>
#include <tt/engine/renderer/ScreenSettings.h>
#include <tt/input/IPhoneController.h>
#include <tt/math/Point2.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>


@interface TTdevObjCAppViewController : UIViewController
- (id)initForView:(UIView*)p_view;
@end


@implementation TTdevObjCAppViewController
- (id)initForView:(UIView*)p_view
{
	self = [super init];
	if (self != nil)
	{
		self.view = p_view;
	}
	return self;
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation
{
	TTdevObjCAppView* appView = (TTdevObjCAppView*)self.view;
	return [appView shouldAutorotateToInterfaceOrientation:p_orientation];
}


- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation duration:(NSTimeInterval)p_duration
{
	[super willRotateToInterfaceOrientation:p_orientation duration:p_duration];
	TTdevObjCAppView* appView = (TTdevObjCAppView*)self.view;
	[appView willRotateToInterfaceOrientation:p_orientation duration:p_duration];
}


- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)p_fromOrientation
{
	[super didRotateFromInterfaceOrientation:p_fromOrientation];
	TTdevObjCAppView* appView = (TTdevObjCAppView*)self.view;
	[appView didRotateFromInterfaceOrientation:p_fromOrientation];
}

@end


@implementation TTdevObjCAppView

@synthesize openGLContext;
@synthesize viewController = m_viewController;


// Make this view class use CAEAGLLayer as its layer class
+ (Class)layerClass
{
    return [CAEAGLLayer class];
}


- (id)initWithFrame:(CGRect)p_frameRect app:(tt::app::OsxApp*)p_app ios2xMode:(bool)p_ios2xMode
{
	self = [super initWithFrame:p_frameRect];
	if (self != nil)
	{
		// Initialize member variables (Objective C needs constructors...)
		m_app = p_app;
		m_viewFramebuffer  = 0;
		m_viewRenderbuffer = 0;
		//m_depthBuffer = 0;
		openGLContext = 0;
		m_viewController = nil;
		
		// Set up the OpenGL view layer
		CAEAGLLayer* eaglLayer = (CAEAGLLayer*)[self layer];
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
														  [NSNumber numberWithBool:FALSE], 
														  kEAGLDrawablePropertyRetainedBacking, 
														  kEAGLColorFormatRGB565, 
														  kEAGLDrawablePropertyColorFormat, 
														  nil];
		
		
		// For iOS 2x Mode ("Retina" support), set the contentScaleFactor to 2.0
		if (p_ios2xMode && [self respondsToSelector:@selector(contentScaleFactor)])
		{
			self.contentScaleFactor = 2.0f;
		}
		
		// Create an OpenGL ES 1.x context
		openGLContext = [[TTOpenGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		if (openGLContext == nil)
		{
			[self release];
			return nil;
		}
		
		[openGLContext makeCurrentContext];
		
		// Perform initial OpenGL setup
		// FIXME: Create OpenGL context etc
		//NSOpenGLContext* context = [self openGLContext];
		
		//[context makeCurrentContext];
		// ***** Initialization of the OpenGL system needed for the iPhone. *****
		
		// Create the frame and render buffers
		glGenFramebuffersOES(1, &m_viewFramebuffer);
		glGenRenderbuffersOES(1, &m_viewRenderbuffer);
		
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, m_viewFramebuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_viewRenderbuffer);
		[openGLContext renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(id<EAGLDrawable>)eaglLayer];
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, m_viewRenderbuffer);
		
		// Depth Buffer
		// TODO: Maybe add if here so we don't always create a depth buffer. (Might not be used.)
		/*
		{
			glGenRenderbuffersOES(1, &m_depthBuffer);
			glBindRenderbufferOES(GL_RENDERBUFFER_OES, m_depthBuffer);
			glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, m_backingWidth, m_backingHeight);
			glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, m_depthBuffer);
		}
		//*/
		
		if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) 
		{
			[self release];
			return nil;
		}
		
		// Pass the frame and render buffers to our OpenGL context class
		[openGLContext setFrameBufferID:m_viewFramebuffer];
		[openGLContext setRenderBufferID:m_viewRenderbuffer];
		
		
		// Set up support for IPhoneController (input)
		[self setMultipleTouchEnabled:YES];
		[self setExclusiveTouch:YES];
		//fingerOneOnScreen = false;
		//fingerTwoOnScreen = false;
		
		// FIXME: Make accelerometer frequency configurable?
		//[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0f / p_accelerometerFrequency)];
		[[UIAccelerometer sharedAccelerometer] setUpdateInterval:(1.0f / 30.0f)];
		[[UIAccelerometer sharedAccelerometer] setDelegate:self];
		
		// Set up the view controller
		m_viewController = [[TTdevObjCAppViewController alloc] initForView:self];
		
		// Disable auto-resizing
		self.autoresizingMask = UIViewAutoresizingNone;
	}
	
	return self;
}


- (void)dealloc
{
	if (m_viewFramebuffer != 0)
	{
		glDeleteFramebuffersOES(1, &m_viewFramebuffer);
		m_viewFramebuffer = 0;
	}
	
	if (m_viewRenderbuffer != 0)
	{
		glDeleteRenderbuffersOES(1, &m_viewRenderbuffer);
		m_viewRenderbuffer = 0;
	}
	
	/*
	if (m_depthBuffer != 0)
	{
		glDeleteFramebuffersOES(1, &m_depthBuffer);
		m_depthBuffer = 0;
	}
	//*/
	
	if (openGLContext != 0)
	{
		[openGLContext release];
		openGLContext = 0;
	}
	
	if (m_viewController != nil)
	{
		[m_viewController release];
		m_viewController = nil;
	}
	
	[super dealloc];
}


//--------------------------------------------------------------------------------------------------
// App orientation event handling (forwarded from custom UIViewController)

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation
{
	if (m_app == 0)
	{
		// No app available! No autorotation.
		return NO;
	}
	
	return m_app->onShouldAutoRotateToOrientation(tt::app::getAppOrientation(p_orientation)) ? YES : NO;
}


- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation
								duration:(NSTimeInterval)p_duration
{
	if (m_app != 0)
	{
		m_app->onWillRotateToOrientation(tt::app::getAppOrientation(p_orientation),
		                                 static_cast<real>(p_duration));
	}
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)p_fromOrientation

{
	if (m_app != 0)
	{
		m_app->onDidRotateFromOrientation(tt::app::getAppOrientation(p_fromOrientation));
	}
}



//--------------------------------------------------------------------------------------------------
// Input event handling (for IPhoneController)

- (BOOL)acceptsFirstResponder
{
	return YES;
}


- (void)updateCoordinates:(NSSet*)p_touches withEvent:(UIEvent*)p_event
{
	(void)p_touches;
	
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	
	// Find the UITouches that are still on screen
	const NSUInteger touchCount = [[p_event allTouches] count];
	int fingerIndex = 0;
	for (NSUInteger i = 0; i < touchCount; ++i)
	{
		UITouch*     touch = [[[p_event allTouches] allObjects] objectAtIndex: i];
		UITouchPhase phase = [touch phase];
		if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
		{
			// Use this touch's location
			if (fingerIndex == 0)
			{
				// Touch one
				CGPoint loc = [touch locationInView:self];
				tempState.touch1Location.valid = true;
				tempState.touch1Location.x = static_cast<int>(loc.x);
				tempState.touch1Location.y = static_cast<int>(loc.y);
				++fingerIndex;
			}
			else if (fingerIndex == 1)
			{
				// Touch two
				CGPoint loc = [touch locationInView:self];
				tempState.touch2Location.valid = true;
				tempState.touch2Location.x = static_cast<int>(loc.x);
				tempState.touch2Location.y = static_cast<int>(loc.y);
				++fingerIndex;
			}
			else if (fingerIndex == 2)
			{
				// Touch three
				CGPoint loc = [touch locationInView:self];
				tempState.touch3Location.valid = true;
				tempState.touch3Location.x = static_cast<int>(loc.x);
				tempState.touch3Location.y = static_cast<int>(loc.y);
				break;
			}
		}
	}
}


- (void)touchesBegan:(NSSet*)p_touches withEvent:(UIEvent*)p_event
{
	// ----- Old touch input code.
	NSUInteger actualTouches = 0;
	const NSUInteger touchCount = [[p_event allTouches] count];
	for (NSUInteger i = 0; i < touchCount; ++i)
	{
		UITouchPhase phase = [[[[p_event allTouches] allObjects] objectAtIndex: i] phase];
		if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
		{
			++actualTouches;
		}
	}
	
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	
	tempState.touch1Status.update(actualTouches > 0);
	tempState.touch2Status.update(actualTouches > 1);
	tempState.touch3Status.update(actualTouches > 2);
	
	[self updateCoordinates:p_touches withEvent:p_event];
	
	// ----- End of old touch input code.
	NSEnumerator* enumerator = [p_touches objectEnumerator];
	
	for (id value = [enumerator nextObject]; value != nil; value = [enumerator nextObject])
	{
		UITouch* uiTouch = static_cast<UITouch*>(value);
		
		TT_ASSERT([uiTouch phase] != UITouchPhaseEnded && [uiTouch phase] != UITouchPhaseCancelled);
		TT_ASSERT([uiTouch phase] == UITouchPhaseBegan);
		
		//TT_Printf("touchesBegan:withEvent - id: %p\n", uiTouch);
		
		/*
		if ([uiTouch tapCount] == 1)
		{
			CGPoint loc = [uiTouch locationInView:self];
			tempState.appView_startTouch(uiTouch,
			                             tt::math::Point2(static_cast<s32>(loc.x), static_cast<s32>(loc.y)));
		}
		else */
		if (tempState.hasTouch(uiTouch) == false)
		{
			CGPoint loc = [uiTouch locationInView:self];
			tempState.appView_startTouch(uiTouch,
			                             tt::math::Point2(static_cast<s32>(loc.x), static_cast<s32>(loc.y)));			
		}
		else
		{
			CGPoint loc = [uiTouch locationInView:self];
			tempState.appView_modifyTouch(uiTouch).updateLocation(
					tt::math::Point2(static_cast<s32>(loc.x), static_cast<s32>(loc.y)));
		}
	}
}


- (void)touchesMoved:(NSSet*)p_touches withEvent:(UIEvent*)p_event
{
	[self updateCoordinates:p_touches withEvent:p_event];
	
	// ----- End of old touch input code.
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	NSEnumerator* enumerator = [p_touches objectEnumerator];
	
	for (id value = [enumerator nextObject]; value != nil; value = [enumerator nextObject])
	{
		UITouch* uiTouch = static_cast<UITouch*>(value);
		
		//TT_Printf("touchesMoved:withEvent - id: %p\n", uiTouch);
		
		
		TT_ASSERT([uiTouch phase] != UITouchPhaseEnded && [uiTouch phase] != UITouchPhaseCancelled);
		TT_ASSERT([uiTouch phase] == UITouchPhaseMoved);
		
		CGPoint loc = [uiTouch locationInView:self];
		tempState.appView_modifyTouch(uiTouch).updateLocation(tt::math::Point2(static_cast<s32>(loc.x), static_cast<s32>(loc.y)));
	}
}


- (void)touchesEnded:(NSSet*)p_touches withEvent:(UIEvent*)p_event 
{
	NSUInteger touchCount = [[p_event allTouches] count];
	int stillTouching = 0;
	for (NSUInteger i = 0; i < touchCount; ++i)
	{
		UITouchPhase phase = [[[[p_event allTouches] allObjects] objectAtIndex: i] phase];
		if (phase != UITouchPhaseEnded && phase != UITouchPhaseCancelled)
		{
			++stillTouching;
		}
	}
	
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	
	tempState.touch1Status.update(stillTouching >= 1);
	tempState.touch2Status.update(stillTouching >= 2);
	tempState.touch3Status.update(stillTouching >= 3);
	
	[self updateCoordinates:p_touches withEvent:p_event];
	
	// ----- End of old touch input code.
	
	NSEnumerator* enumerator = [p_touches objectEnumerator];
	
	for (id value = [enumerator nextObject]; value != nil; value = [enumerator nextObject])
	{
		UITouch* uiTouch = static_cast<UITouch*>(value);
		
		//TT_Printf("touchesEnded:withEvent - id: %p\n", uiTouch);
		
		TT_ASSERT([uiTouch phase] == UITouchPhaseEnded);
		
		tempState.appView_modifyTouch(uiTouch).updateRelease();
		TT_ASSERT(tempState.getTouch(uiTouch).status.down == false);
	}
}


- (void)touchesCancelled:(NSSet*)p_touches withEvent:(UIEvent*)p_event 
{
	(void)p_event;
	
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	NSEnumerator* enumerator = [p_touches objectEnumerator];
	
	for (id value = [enumerator nextObject]; value != nil; value = [enumerator nextObject])
	{
		UITouch* uiTouch = static_cast<UITouch*>(value);
		
		//TT_Printf("touchesCancelled:withEvent - id: %p\n", uiTouch);
		
		TT_ASSERT([uiTouch phase] == UITouchPhaseCancelled);
		
		if (tempState.hasTouch(uiTouch))
		{
			// FIXME: Cancel might need to be handled differently from end.
			tempState.appView_modifyTouch(uiTouch).updateRelease();
		}
	}
}


- (void)accelerometer:(UIAccelerometer*)p_accelerometer didAccelerate:(UIAcceleration*)p_acceleration
{
	(void)p_accelerometer;
	
	tt::input::IPhoneController& tempState(tt::input::IPhoneController::appView_getTemporaryState());
	
	tempState.accelerometer.x = static_cast<real>(p_acceleration.x);
	tempState.accelerometer.y = static_cast<real>(p_acceleration.y);
	tempState.accelerometer.z = static_cast<real>(p_acceleration.z);
}

@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)
