#include <atomic>

#include <tt/http/HttpRequest.h>
#include <tt/http/HttpConnectMgr.h>
#include <tt/thread/thread.h>
#include <tt/thread/CriticalSection.h>
#include <tt/thread/Mutex.h>

namespace tt {
namespace http {

HttpConnectMgr* HttpConnectMgr::ms_instance = 0;

//--------------------------------------------------------------------------------------------------
// Public member functions

void HttpConnectMgr::createInstance()
{
	TT_ASSERT(ms_instance == 0);
	if (ms_instance == 0)
	{
		ms_instance = new HttpConnectMgr();
	}
	TT_NULL_ASSERT(ms_instance);
}


void HttpConnectMgr::destroyInstance()
{
	TT_ASSERT(ms_instance != 0);
	delete ms_instance;
}


s32 HttpConnectMgr::queueRequest(HttpRequest& p_request)
{
	static s32 currentID = 0;
	
	tt::thread::CriticalSection critSec(&m_mutex);
	p_request.requestID = currentID++;
	m_requests.push_back(p_request);
	return p_request.requestID;
}


void HttpConnectMgr::queueResponse(tt::http::HttpResponseHandler* p_handler, const tt::http::HttpResponse& p_response)
{
	if (p_handler != 0)
	{
		tt::thread::CriticalSection critSec(&m_mutex);
		m_responses.push_back(ResponsePair(p_handler, p_response));
	}
}


void HttpConnectMgr::processResponses()
{
	tt::thread::CriticalSection critSec(&m_mutex);
	if (m_responses.empty())
	{
		return;
	}
	
	for (auto& it : m_responses)
	{
		if (it.first != 0)
		{
			it.first->handleHttpResponse(it.second);
		}
	}
	m_responses.clear();
}


//--------------------------------------------------------------------------------------------------
// Private member functions

HttpConnectMgr::HttpConnectMgr()
:
m_requests(),
m_responses(),
m_mutex(),
m_workerThread(),
m_stopThread(false)
{
	m_workerThread = tt::thread::create(&HttpConnectMgr::workerThreadEntry, this, false, 0U,
		tt::thread::priority_below_normal, tt::thread::Affinity_None, "HTTP Request Thread");
}


HttpConnectMgr::~HttpConnectMgr()
{
	m_stopThread = true;
	tt::thread::wait(m_workerThread);
}


int HttpConnectMgr::workerThreadEntry(void* p_arg)
{
	TT_NULL_ASSERT(p_arg);
	HttpConnectMgr* ptr = reinterpret_cast<HttpConnectMgr*>(p_arg);
	return ptr->workerThreadProc();
}


s32 HttpConnectMgr::workerThreadProc()
{
	init();
	
	while (m_stopThread == false)
	{
		tt::http::HttpRequest request;
		bool handleRequest = false;
		{
			tt::thread::CriticalSection critSec(&m_mutex);
			if (m_requests.empty() == false)
			{
				request = *m_requests.begin();
				m_requests.pop_front();
				handleRequest = true;
			}
		}
		
		if (handleRequest)
		{
			processRequest(request);
		}
		tt::thread::sleep(200);
	}
	
	uninit();
	
	return 0;
}



// Namespace end
}
}
