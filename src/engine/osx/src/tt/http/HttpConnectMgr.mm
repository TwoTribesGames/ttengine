#import <Foundation/Foundation.h>
#if defined(TT_PLATFORM_OSX_MAC)
#import <AppKit/AppKit.h>
#elif defined(TT_PLATFORM_OSX_IPHONE)
#import <UIKit/UIKit.h>
#endif

#include <sstream>
#include <utility>

#include <tt/http/helpers.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/http/HttpRequest.h>
#include <tt/http/HttpResponseHandler.h>
#include <tt/mem/util.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/settings/settings.h>
#include <tt/str/str.h>
#include <tt/version/Version.h>


//--------------------------------------------------------------------------------------------------
// Objective C delegate class for network notifications

@interface TTObjCHttpConnectionDelegate : NSObject
{
	tt::http::HttpConnectMgr* httpConnectMgr;
}

- (id)init:(tt::http::HttpConnectMgr*)p_networkInstance;
- (void)connection:(NSURLConnection*)p_connection didReceiveResponse:(NSURLResponse*)p_response;
- (void)connection:(NSURLConnection*)p_connection didReceiveData:(NSData*)p_data;
- (void)connection:(NSURLConnection*)p_connection didFailWithError:(NSError*)p_error;
- (void)connectionDidFinishLoading:(NSURLConnection*)p_connection;

@end


@implementation TTObjCHttpConnectionDelegate

- (id)init:(tt::http::HttpConnectMgr*)p_httpConnectMgr
{
	TT_NULL_ASSERT(p_httpConnectMgr);
	
	self = [super init];
	if (self != nil)
	{
		httpConnectMgr = p_httpConnectMgr;
	}
	return self;
}


- (void)connection:(NSURLConnection*)p_connection didReceiveResponse:(NSURLResponse*)p_response
{
	httpConnectMgr->connectionReceivedResponse(p_connection, static_cast<s32>([(NSHTTPURLResponse*)p_response statusCode]));
}


- (void)connection:(NSURLConnection*)p_connection didReceiveData:(NSData*)p_data
{
	if (p_data != nil)
	{
		const u8*   bytes      = reinterpret_cast<const u8*>([p_data bytes]);
		std::size_t dataLength = static_cast<std::size_t>([p_data length]);
		httpConnectMgr->connectionReceivedData(p_connection, bytes, dataLength);
	}
}


- (void)connection:(NSURLConnection*)p_connection didFailWithError:(NSError*)p_error
{
	NSLog(@"---- Low-level HTTP connection had an error!\n- Description: '%@'\n- Reason: '%@'\n--------------------",
	      [p_error localizedDescription], [p_error localizedFailureReason]);
	httpConnectMgr->connectionFailed(p_connection, tt::str::utf8ToUtf16([[p_error localizedDescription] UTF8String]));
}


- (void)connectionDidFinishLoading:(NSURLConnection*)p_connection
{
	httpConnectMgr->connectionFinishedLoading(p_connection);
}

@end



namespace tt {
namespace http {

//--------------------------------------------------------------------------------------------------
// Public member functions

HttpConnectMgr::HttpConnectMgr()
:
m_userAgent(),
m_haveInternetConnection(false),
m_reachability(0),
m_downloadTracking()
{
	// Compose a user agent string for our HTTP communications based on the application name and revision
	{
		std::ostringstream userAgent;
		userAgent << tt::str::utf16ToUtf8(tt::settings::getApplicationName()) << "_rev_"
		          << tt::version::getClientRevisionNumber()
		          << "." << tt::version::getLibRevisionNumber();
		m_userAgent = userAgent.str();
	}
	
	TT_Printf("HttpConnectMgr::HttpConnectMgr: User-agent string: '%s'\n", m_userAgent.c_str());
	
	// Create an internet connection checker for "twotribes.com"
	static const char* const hostToCheckAgainst = "twotribes.com";
	m_reachability = SCNetworkReachabilityCreateWithName(0, hostToCheckAgainst);
	if (m_reachability != 0)
	{
		// Perform initial (synchronous) check for internet connection
		SCNetworkReachabilityFlags flags = 0;
		if (SCNetworkReachabilityGetFlags(m_reachability, &flags))
		{
			// Use the callback to update the internet connection flag (avoid code duplication)
			reachabilityCallback(m_reachability, flags, this);
		}
		else
		{
			TT_WARN("Retrieving initial network reachability flags (with host '%s') failed.",
			        hostToCheckAgainst);
		}
		
		// Request internet connection status updates (through the callback)
		SCNetworkReachabilityContext context = { 0 };
		context.info = this;
		if (SCNetworkReachabilitySetCallback(m_reachability, reachabilityCallback, &context) == FALSE ||
		    SCNetworkReachabilityScheduleWithRunLoop(m_reachability, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode) == FALSE)
		{
			// Error setting callback or scheduling status updates in run loop; clean up early
			SCNetworkReachabilitySetCallback(m_reachability, 0, 0);
			CFRelease(m_reachability);
			m_reachability = 0;
		}
	}
}


HttpConnectMgr::~HttpConnectMgr()
{
	// Unregister internet connection status callback
	if (m_reachability != 0)
	{
		SCNetworkReachabilityUnscheduleFromRunLoop(m_reachability, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		SCNetworkReachabilitySetCallback(m_reachability, 0, 0);
		CFRelease(m_reachability);
		m_reachability = 0;
	}
}


bool HttpConnectMgr::sendRequest(const HttpRequest& p_request)
{
	//==========================================================================
	// FIXME: Factor this preamble code out to shared code
	std::string finalUrl(p_request.url);
	
	if (p_request.getParameters.empty() == false)
	{
		finalUrl += "?" + buildQuery(p_request.getParameters);
	}
	
	std::string postDataStr;
	if (p_request.postParameters.empty() == false)
	{
		for (FunctionArguments::const_iterator it(p_request.postParameters.begin());
		     it != p_request.postParameters.end() ; ++it)
		{
			postDataStr += it->first + '=' + it->second + '&';
		}
		postDataStr.erase(postDataStr.size() - 1);
	}
	
	/*
	TT_Printf("HTTP request:\n- Headers (%u):\n", p_request.headers.size());
	for (Headers::const_iterator it = p_request.headers.begin();
	     it != p_request.headers.end(); ++it)
	{
		TT_Printf("   - '%s' = '%s'\n", (*it).first.c_str(), (*it).second.c_str());
	}
	TT_Printf("- POST data: %s\n", postDataStr.c_str());
	TT_Printf("- Full URL: http%s://%s%s\n", p_request.useSSL ? "s" : "",
	          p_request.server.c_str(), finalUrl.c_str());
	//*/
	//==========================================================================
	
	
	const std::string fullUrl((p_request.useSSL ? "https://" : "http://") + p_request.server + finalUrl);
	
	/*
	TT_Printf("HttpConnectMgr::sendRequest: Full URL: '%s'\n",  fullUrl.c_str());
	TT_Printf("HttpConnectMgr::sendRequest: HTTP method: %s\n", HttpRequest::getRequestMethodName(p_request.requestMethod));
	TT_Printf("HttpConnectMgr::sendRequest: POST data: '%s'\n", postDataStr.c_str());
	//*/
	
	NSURL* url = [NSURL URLWithString:[NSString stringWithUTF8String:fullUrl.c_str()]];
	
	NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL:url];
	[request setHTTPMethod:[NSString stringWithUTF8String:HttpRequest::getRequestMethodName(p_request.requestMethod)]];
	[request setValue:[NSString stringWithUTF8String:m_userAgent.c_str()] forHTTPHeaderField:@"User-Agent"];
	
	// Add all headers that were specified (also set a default content type)
	[request setValue:@"application/x-www-form-urlencoded;charset=UTF-8" forHTTPHeaderField:@"Content-Type"];
	for (Headers::const_iterator it = p_request.headers.begin();
	     it != p_request.headers.end(); ++it)
	{
		[request setValue:[NSString stringWithUTF8String:(*it).second.c_str()]
		         forHTTPHeaderField:[NSString stringWithUTF8String:(*it).first.c_str()]];
	}
	
	// Set the POST data as NSData object
	NSData* postData = [NSData dataWithBytes:postDataStr.c_str() length:postDataStr.length()];
	[request setHTTPBody:postData];
	
	// Possibly disable the network activity indicator (iOS-only)?
	//[UIApplication sharedApplication].networkActivityIndicatorVisible = NO; 
	
	TTObjCHttpConnectionDelegate* connectionDelegate = [[TTObjCHttpConnectionDelegate alloc] init:this];
	
	// Open a connection and record the download information
	DownloadInfo info;
	info.connectionDelegate = connectionDelegate;
	info.responseHandler    = p_request.responseHandler;
	
	NSURLConnection* connection = [[NSURLConnection alloc] initWithRequest:request delegate:connectionDelegate];
	
	m_downloadTracking.insert(std::make_pair(connection, info));
	
	return true;
}


void HttpConnectMgr::cancelAllRequestsForResponseHandler(HttpResponseHandler* p_handler)
{
	for (DownloadMapping::iterator it = m_downloadTracking.begin();
	     it != m_downloadTracking.end(); )
	{
		if ((*it).second.responseHandler == p_handler)
		{
			// This is a request that needs to be cancelled
			// First get the required information for cancellation
			NSURLConnection* connection  = reinterpret_cast<NSURLConnection*>((*it).first);
			
			/*
			TT_Printf("HttpConnectMgr::cancelAllRequestsForResponseHandler: "
			          "Found connection 0x%08X for callable 0x%08X\n",
			          connection, p_handler);
			//*/
			
			// Remove the download tracking info for this request
			DownloadMapping::iterator eraseIt(it);
			++it;
			m_downloadTracking.erase(eraseIt);
			
			// Cancel the request
			[connection cancel];
		}
		else
		{
			// Should not cancel this request; continue with the next entry
			++it;
		}
	}
}


void HttpConnectMgr::openUrlExternally(const std::string& p_url)
{
#if defined(TT_PLATFORM_OSX_IPHONE)
	// iOS implementation
	[[UIApplication sharedApplication] openURL:
		[NSURL URLWithString: [NSString stringWithUTF8String:p_url.c_str()]]];
#else
	// Mac OS X implementation
	[[NSWorkspace sharedWorkspace] openURL:
		[NSURL URLWithString: [NSString stringWithUTF8String:p_url.c_str()]]];
#endif
}


void HttpConnectMgr::connectionFailed(void* p_connection, const std::wstring& p_message)
{
	// Remove the connection from the registration
	DownloadMapping::iterator it = m_downloadTracking.find(p_connection);
	if (it != m_downloadTracking.end())
	{
		// FIXME: Do more processing than simply removing the entry?
		m_downloadTracking.erase(it);
	}
	
	// Notify the callback about the error
	if ((*it).second.responseHandler != 0)
	{
		(*it).second.responseHandler->handleHttpError(p_message);
	}
}


void HttpConnectMgr::connectionReceivedResponse(void* p_connection, s32 p_statusCode)
{
	// Find the connection in the registration
	DownloadMapping::iterator it = m_downloadTracking.find(p_connection);
	if (it == m_downloadTracking.end())
	{
		TT_PANIC("Received 'connection received response' notification for unknown connection.");
		return;
	}
	
	// Save the HTTP status code for this connection
	(*it).second.statusCode = p_statusCode;
}


void HttpConnectMgr::connectionReceivedData(void* p_connection, const u8* p_data, std::size_t p_dataLength)
{
	// Find the connection in the registration
	DownloadMapping::iterator it = m_downloadTracking.find(p_connection);
	if (it == m_downloadTracking.end())
	{
		TT_PANIC("Received 'connection received data' notification for unknown connection.");
		return;
	}
	
	// FIXME: Only bother storing data if there's a response handler to send it to
	
	// Append the data that was received to any existing data
	(*it).second.data.insert((*it).second.data.end(), p_data, p_data + p_dataLength);
}


void HttpConnectMgr::connectionFinishedLoading(void* p_connection)
{
	// Find the connection in the registration
	DownloadMapping::iterator it = m_downloadTracking.find(p_connection);
	if (it == m_downloadTracking.end())
	{
		TT_PANIC("Received 'connection finished loading' notification for unknown connection.");
		return;
	}
	
	handleReturnData((*it).second.statusCode, (*it).second.data, (*it).second.responseHandler);
	
	// FIXME: Do more processing than simply removing the entry?
	m_downloadTracking.erase(it);
}


//--------------------------------------------------------------------------------------------------
// Private member functions

void HttpConnectMgr::reachabilityCallback(SCNetworkReachabilityRef   /*p_target*/,
                                          SCNetworkReachabilityFlags p_flags,
                                          void*                      p_userData)
{
	if (p_userData == 0) return;
	
	/*
	TT_Printf("HttpConnectMgr::reachabilityCallback: Reachability flags changed to:\n"
	          "HttpConnectMgr::reachabilityCallback: - TransientConnection   %s\n"
	          "HttpConnectMgr::reachabilityCallback: - Reachable             %s\n"
	          "HttpConnectMgr::reachabilityCallback: - ConnectionRequired    %s\n"
	          "HttpConnectMgr::reachabilityCallback: - ConnectionOnTraffic   %s\n"
	          "HttpConnectMgr::reachabilityCallback: - InterventionRequired  %s\n"
	          "HttpConnectMgr::reachabilityCallback: - ConnectionOnDemand    %s\n"
	          "HttpConnectMgr::reachabilityCallback: - IsLocalAddress        %s\n"
	          "HttpConnectMgr::reachabilityCallback: - IsDirect              %s\n"
	          "HttpConnectMgr::reachabilityCallback: - IsWWAN                %s\n",
	          (p_flags & kSCNetworkReachabilityFlagsTransientConnection)  ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsReachable)            ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsConnectionRequired)   ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsConnectionOnTraffic)  ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsInterventionRequired) ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsConnectionOnDemand)   ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsIsLocalAddress)       ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsIsDirect)             ? "true" : "false",
	          (p_flags & kSCNetworkReachabilityFlagsIsWWAN)               ? "true" : "false");
	//*/
	
	HttpConnectMgr* instance = reinterpret_cast<HttpConnectMgr*>(p_userData);
	
	// Have connection if:
	// - The target is reachable (twotribes.com in this case)
	// - No connection needs to be set up first
	// - No user intervention is required in order to establish a connection
	instance->m_haveInternetConnection =
		((p_flags & kSCNetworkReachabilityFlagsReachable) == kSCNetworkReachabilityFlagsReachable) &&
		((p_flags & kSCNetworkReachabilityFlagsConnectionRequired)   == 0) &&
		((p_flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0);
}


void HttpConnectMgr::handleReturnData(s32 p_statusCode, const Data& p_data,
                                      HttpResponseHandler* p_responseHandler)
{
	if (p_responseHandler != 0)
	{
		p_responseHandler->handleHttpResponse(p_statusCode,
		                                      p_data.empty() ? "" : parseDataAsString(&p_data.at(0), p_data.size()));
	}
}

// Namespace end
}
}
