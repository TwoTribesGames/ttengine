#if !defined(INC_TT_APP_TTDEVOBJCAPPVIEW_IPHONE_H)
#define INC_TT_APP_TTDEVOBJCAPPVIEW_IPHONE_H

#if defined(TT_PLATFORM_OSX_IPHONE) // this file is for iPhone builds only


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

#include <string>

#include <tt/app/TTOpenGLContext_iphone.h>


namespace tt {
namespace app {
	class OsxApp;
}
}


// This class is responsible for:
// - OpenGL setup and maintenance
// - Receiving touch events and pushing them to IPhoneController
// - Receiving accelerometer events and pushing them to IPhoneController

@interface TTdevObjCAppView : UIView <UIAccelerometerDelegate>
{
@private
	tt::app::OsxApp* m_app;
	
	GLuint m_viewFramebuffer;
	GLuint m_viewRenderbuffer;
	//GLuint m_depthBuffer;
	
	TTOpenGLContext* openGLContext;
	
	/*
	// Support variables for tt::input::IPhoneController
	bool fingerOneOnScreen;
	bool fingerTwoOnScreen;
	
	CGPoint fingerOneLocation; // Coordinates for finger 1
	CGPoint fingerTwoLocation; // Coordinates for finger 2
	
	UIAccelerationValue x; //doubles
	UIAccelerationValue y;
	UIAccelerationValue z;
	*/
	
	UIViewController* m_viewController;
}

@property (readonly, nonatomic) TTOpenGLContext* openGLContext;
@property (readonly, nonatomic) UIViewController* viewController;


- (id)initWithFrame:(CGRect)p_frameRect app:(tt::app::OsxApp*)p_app ios2xMode:(bool)p_ios2xMode;

// For internal use (by the view controller):
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation;
- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)p_orientation duration:(NSTimeInterval)p_duration;
- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)p_fromOrientation;

@end


#endif  // defined(TT_PLATFORM_OSX_IPHONE)

#endif  // !defined(INC_TT_APP_TTDEVOBJCAPPVIEW_IPHONE_H)
