#include <atomic>
#include <sstream>
#include <utility>

#import <AppKit/AppKit.h>

#include <tt/http/helpers.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/http/HttpRequest.h>
#include <tt/http/HttpResponseHandler.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>
#include <tt/settings/settings.h>
#include <tt/str/str.h>
#include <tt/version/Version.h>


#include <curl/curl.h>

namespace tt {
namespace http {

static std::string g_userAgent;

class CurlHandle
{
public:
	CurlHandle() { m_pCurl = curl_easy_init(); }
	~CurlHandle() { curl_easy_cleanup(m_pCurl); }

	operator CURL*() const { return m_pCurl; };

private:
	CurlHandle(const CurlHandle&);
	CurlHandle &operator=(const CurlHandle&);

	CURL* m_pCurl;
};

template<typename T>
class CurlHandleCleanup
{
public:
	CurlHandleCleanup(T* ptr) : m_pHandle(ptr) {}
	~CurlHandleCleanup();

private:
	CurlHandleCleanup(const CurlHandleCleanup &);
	CurlHandleCleanup &operator=(const CurlHandleCleanup &);

	T *m_pHandle;
};

template<>
CurlHandleCleanup<struct curl_slist>::~CurlHandleCleanup() {
	if(m_pHandle) curl_slist_free_all(m_pHandle);
}

template<>
CurlHandleCleanup<struct curl_httppost>::~CurlHandleCleanup() {
	if(m_pHandle) curl_formfree(m_pHandle);
}

void HttpConnectMgr::openUrlExternally(const std::string& p_url)
{
	@autoreleasepool {
		// Mac OS X implementation
		[[NSWorkspace sharedWorkspace] openURL:
			[NSURL URLWithString: [NSString stringWithUTF8String:p_url.c_str()]]];
	}
}

void HttpConnectMgr::init()
{
	// Compose a user agent string for our HTTP communications based on the application name and revision
	std::stringstream userAgent;
	userAgent << tt::str::narrow(tt::settings::getApplicationName()) << "_rev_"
			<< tt::version::getClientRevisionNumber()
			<< "." << tt::version::getLibRevisionNumber();

	g_userAgent = userAgent.str();

	TT_Printf("HttpConnectMgr::HttpConnectMgr: User-agent string: '%s'\n",
			g_userAgent.c_str());

	CURLcode code = curl_global_init(CURL_GLOBAL_ALL);

	TT_ASSERTMSG(code == CURLE_OK, "Initializng curl library failed.");
}

void HttpConnectMgr::uninit()
{
	curl_global_cleanup();
}

bool HttpConnectMgr::processRequest(const HttpRequest& p_request)
{
	std::stringstream sstr;
	CURLcode success;
	CurlHandle ch;
	struct curl_slist *slist = 0;
	struct curl_httppost *post = 0, *last = 0;

	//==========================================================================
	// FIXME: Factor this preamble code out to shared code
	std::string finalPath(p_request.url);

	if (p_request.getParameters.empty() == false)
	{
		finalPath += "?" + buildQuery(p_request.getParameters);
	}
	
	TT_ASSERTMSG(p_request.useSSL == false, "SSL is not supported right now.");

	std::string finalUrl = "http://" + p_request.server + finalPath;

	if (p_request.postParameters.empty() == false)
	{
		TT_ASSERTMSG(p_request.requestMethod == HttpRequest::RequestMethod_Post,
					"POST parameters were specified in HttpRequest, "
					"but request method is not POST (request method is '%s'). "
					"These POST parameters will not reach the server.",
					p_request.getRequestMethodName(p_request.requestMethod));


		for (FunctionArguments::const_iterator it(p_request.postParameters.begin());
		     it != p_request.postParameters.end() ; ++it)
		{
			curl_formadd(&post, &last,
						CURLFORM_COPYNAME, it->first.c_str(),
						CURLFORM_COPYCONTENTS, it->second.c_str(),
						CURLFORM_END);
		}

		success = curl_easy_setopt(ch, CURLOPT_HTTPPOST, post);
		TT_ASSERT(success == CURLE_OK);
	}

	if (p_request.headers.empty() == false)
	{
		for (Headers::const_iterator it(p_request.headers.begin());
		     it != p_request.headers.end() ; ++it)
		{
			slist = curl_slist_append( slist, (it->first + ": " + it->second).c_str() );
		}
	}
	slist = curl_slist_append(slist, ("User-Agent: " + g_userAgent).c_str());
	success = curl_easy_setopt(ch, CURLOPT_HTTPHEADER, slist);
	TT_ASSERT(success == CURLE_OK);

	success = curl_easy_setopt(ch, CURLOPT_URL, finalUrl.c_str());
	TT_ASSERT(success == CURLE_OK);

	CurlHandleCleanup<struct curl_slist> cleanSlist(slist);
	CurlHandleCleanup<struct curl_httppost> cleanForm(post);

	success = curl_easy_setopt(ch, CURLOPT_NOSIGNAL, 1L);
	TT_ASSERT(success == CURLE_OK);

	success = curl_easy_perform(ch);
	TT_ASSERT(success == CURLE_OK);

	HttpResponse response;
	long statusCode;

	success = curl_easy_getinfo(ch, CURLINFO_RESPONSE_CODE, &statusCode);
	TT_ASSERT(success == CURLE_OK);

	response.requestID = p_request.requestID;
	response.statusCode = statusCode;

	queueResponse(p_request.responseHandler, response);

	if (success == CURLE_OK) {
		// handle response
	}

	return true;
}

// Namespace end
}
}
