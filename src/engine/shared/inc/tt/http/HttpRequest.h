#if !defined(INC_TT_HTTP_HTTPREQUEST_H)
#define INC_TT_HTTP_HTTPREQUEST_H


#include <string>

#include <tt/http/fwd.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_types.h>


namespace tt {
namespace http {

struct HttpRequest
{
	enum RequestMethod
	{
		RequestMethod_Get,
		RequestMethod_Post
	};
	
	
	inline HttpRequest()
	:
	requestMethod(RequestMethod_Get),
	server(),
	url(),
	headers(),
	getParameters(),
	postParameters(),
	useSSL(false),
	requestID(-1),
	responseHandler(0)
	{
	}
	
	inline static const char* getRequestMethodName(RequestMethod p_method)
	{
		switch (p_method)
		{
		case RequestMethod_Get:  return "GET";
		case RequestMethod_Post: return "POST";
		default: TT_PANIC("Unsupported request method: %d", p_method); return "";
		}
	}
	
	
	RequestMethod        requestMethod;
	std::string          server;          //!< Name or IP of server to connect to (e.g. "www.example.com").
	std::string          url;             //!< Query string to request from server (e.g. "/foo/bar.html").
	Headers              headers;         //!< Optional headers to pass along with request.
	FunctionArguments    getParameters;   //!< GET parameters to add to query.
	FunctionArguments    postParameters;  //!< POST parameters to pass in a POST request.
	bool                 useSSL;          //!< Whether to use an SSL connection (i.e. "https://" instead of "http://").
	s32                  requestID;       //!< ID to differentiate between multiple requests
	// FIXME: Make some kind of smart pointer:
	HttpResponseHandler* responseHandler; //!< Handler to send server response data (or errors) to. Server response will be ignored if this is null.
};

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HTTPREQUEST_H)
