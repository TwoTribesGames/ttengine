#if !defined(INC_TT_HTTP_HTTPCONNECTMGR_H)
#define INC_TT_HTTP_HTTPCONNECTMGR_H

#include <list>
#include <atomic>

#include <tt/http/fwd.h>
#include <tt/http/HttpRequest.h>
#include <tt/http/HttpResponseHandler.h>
#include <tt/platform/tt_error.h>
#include <tt/thread/thread.h>
#include <tt/thread/Mutex.h>

namespace tt {
namespace http {

class HttpConnectMgr
{
public:
	
	static void createInstance();
	static void destroyInstance();
	
	inline static HttpConnectMgr* getInstance() { TT_ASSERT(ms_instance != 0); return ms_instance; }
	inline static bool hasInstance() { return ms_instance != 0; }
	
	/*! \brief Queues an HTTP request. If a valid response handler is specified in the HttpRequest,
	           the server's response will be sent there. If not, the response is ignored. */
	s32 queueRequest(HttpRequest& p_request);
	
	void queueResponse(tt::http::HttpResponseHandler* p_handler, const tt::http::HttpResponse& p_response);
	
	// Call this from the main thread
	void processResponses();
	
	/*! \brief Opens the specified URL externally (outside of the application;
	           in the user's default browser for example). */
	void openUrlExternally(const std::string& p_url);
	
private:
	typedef std::list<tt::http::HttpRequest>  PendingRequests;
	typedef std::pair<tt::http::HttpResponseHandler*, tt::http::HttpResponse> ResponsePair;
	typedef std::list<ResponsePair> PendingResponses;
	
	HttpConnectMgr();
	~HttpConnectMgr();
	
	static int workerThreadEntry(void* p_arg);
	s32 workerThreadProc();
	
	void init();
	void uninit();
	
	bool processRequest(const HttpRequest& p_request);
	
	PendingRequests    m_requests;
	PendingResponses   m_responses;
	tt::thread::Mutex  m_mutex;
	tt::thread::handle m_workerThread;
	std::atomic<bool>  m_stopThread;
	
	static HttpConnectMgr* ms_instance;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HTTPCONNECTMGR_H)
