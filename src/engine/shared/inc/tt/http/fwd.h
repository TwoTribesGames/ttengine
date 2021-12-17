#if !defined(INC_TT_HTTP_FWD_H)
#define INC_TT_HTTP_FWD_H


#include <map>
#include <string>
#include <utility>
#include <vector>

#include <tt/platform/tt_types.h>


namespace tt {
namespace http {

typedef std::map<std::string, std::string> FunctionArguments;

// Header: pair.first = header name, pair.second = header value
// NOTE: Vector of pairs because header names can be specified multiple times
typedef std::pair<std::string, std::string> Header;
typedef std::vector<Header> Headers;


struct HttpRequest;
struct HttpResponse;

class HttpResponseHandler;
typedef tt_ptr<HttpResponseHandler>::shared HttpResponseHandlerPtr;
typedef tt_ptr<HttpResponseHandler>::weak   HttpResponseHandlerWeakPtr;

class HttpConnectMgr;

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_FWD_H)
