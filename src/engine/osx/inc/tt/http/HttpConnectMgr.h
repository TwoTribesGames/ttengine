#if !defined(INC_TT_HTTP_HTTPCONNECTMGR_H)
#define INC_TT_HTTP_HTTPCONNECTMGR_H


#include <SystemConfiguration/SystemConfiguration.h>

#include <map>
#include <string>
#include <vector>

#include <tt/http/fwd.h>


namespace tt {
namespace http {

class HttpConnectMgr
{
public:
	HttpConnectMgr();
	~HttpConnectMgr();
	
	/*! \brief Sends an HTTP request. If a valid response handler is specified in the HttpRequest,
	           the server's response will be sent there. If not, the response is ignored. */
	bool sendRequest(const HttpRequest& p_request);
	
	/*! \brief Cancels any outstanding HTTP requests for the specified response handler. */
	void cancelAllRequestsForResponseHandler(HttpResponseHandler* p_handler);
	
	/*! \brief Check whether an internet connection is currently available.
	    \note This can potentially be a slow operation. */
	inline bool hasInternetConnection() const { return m_haveInternetConnection; }
	
	/*! \brief Opens the specified URL externally (outside of the application;
	           in the user's default browser for example). */
	static void openUrlExternally(const std::string& p_url);
	
	
	// Internal functions for connection delegate (Objective C, so cannot be private)
	void connectionFailed(void* p_connection, const std::wstring& p_message);
	void connectionReceivedResponse(void* p_connection, s32 p_statusCode);
	void connectionReceivedData(void* p_connection, const u8* p_data, std::size_t p_dataLength);
	void connectionFinishedLoading(void* p_connection);
	
private:
	typedef std::vector<u8> Data;
	
	struct DownloadInfo
	{
		inline DownloadInfo()
		:
		connectionDelegate(0),
		statusCode(0),
		data(),
		responseHandler(0)
		{ }
		inline ~DownloadInfo() { }
		
		
		void* connectionDelegate; //!< Custom Objective C class TTObjCHttpConnectionDelegate.
		s32   statusCode;         //!< HTTP status code received as response.
		Data  data;               //!< All data received through the connection.
		
		HttpResponseHandler* responseHandler;
	};
	
	// Mapping NSURLConnection to DownloadInfo
	typedef std::map<void*, DownloadInfo> DownloadMapping;
	
	
	static void reachabilityCallback(SCNetworkReachabilityRef   p_target,
	                                 SCNetworkReachabilityFlags p_flags,
	                                 void*                      p_userData);
	
	void handleReturnData(s32 p_statusCode, const Data& p_data, HttpResponseHandler* p_callable);
	
	
	std::string              m_userAgent;
	bool                     m_haveInternetConnection; //!< Whether internet connection is currently available. Will be updated in callback.
	SCNetworkReachabilityRef m_reachability;
	DownloadMapping          m_downloadTracking;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HTTPCONNECTMGR_H)
