#if !defined(INC_TT_HTTP_HTTPRESPONSEHANDLER_H)
#define INC_TT_HTTP_HTTPRESPONSEHANDLER_H


#include <string>

#include <tt/platform/tt_types.h>


namespace tt {
namespace http {

struct HttpResponse
{
	HttpResponse()
	:
	data(),
	statusCode(0),
	requestID(-1)
	{
	}
	
	std::string data;
	s32 statusCode;
	s32 requestID;
};


class HttpResponseHandler
{
public:
	HttpResponseHandler() { }
	virtual ~HttpResponseHandler() { }
	
	/*! \brief Called when all HTTP response data has been received.
	    \param p_statusCode   The HTTP status code from the response (e.g. 200 for "ok").
	    \param p_responseData The response data that was received, as UTF-8 string. */
	virtual void handleHttpResponse(const HttpResponse& p_responseData) = 0;
	
	/*! \brief Called when an HTTP request encountered an error.
	    \param p_errorMessage The error message reported by the HTTP library. */
	virtual void handleHttpError(const std::wstring& p_errorMessage)
	{ (void)p_errorMessage; }
};

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HTTPRESPONSEHANDLER_H)
