#if !defined(INC_TT_HTTP_HTTPCONNECTMGRIMPLWIN_H)
#define INC_TT_HTTP_HTTPCONNECTMGRIMPLWIN_H


#if !defined(_XBOX)  // FIXME: It's relatively easy to implement this class for Xbox 360: use XHttp instead of WinHttp (see XDK documentation for details)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winhttp.h>
#endif
#include <map>
#include <vector>

#include <tt/http/fwd.h>
#include <tt/http/HttpRequest.h>


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
	bool hasInternetConnection() const;
	
	/*! \brief Opens the specified URL externally (outside of the application;
	           in the user's default browser for example). */
	static void openUrlExternally(const std::string& p_url);
	
	static void queueRequest(const HttpRequest& p_request);
	static void startQueuingThread();
	static void stopQueuingThread();
	
private:
	enum DownloadState
	{
		DownloadState_Idle,
		DownloadState_SendingRequest,
		DownloadState_WaitingForResponse,
		DownloadState_QueryDataAvailable,
		DownloadState_ReceivingData
	};
	
	typedef std::vector<u8> Data;
	
#if !defined(_XBOX)
	struct DownloadInfo
	{
		DownloadInfo()
		:
		connection(0),
		state(DownloadState_Idle),
		statusCode(0),
		rawData(0),
		responseHandler(0)
		{ }
		~DownloadInfo() { delete[] rawData; }
		
		
		HINTERNET     connection;   //!< Connection opened using WinHttpConnect.
		DownloadState state;
		s32           statusCode;   //!< HTTP status code received as response.
		Data          data;         //!< All data received through the connection.
		u8*           rawData;      //!< Temporary buffer for WinHttpReadData.
		std::string   optionalData; //!< Buffer with the optional POST/PUT data. (Might need to change it to a void* in the future.)
		
		HttpResponseHandler* responseHandler;
	};
	
	// Maps WinHTTP request handle to download tracking info (connection handle etc)
	typedef std::map<HINTERNET, DownloadInfo> DownloadMapping;
#endif
	
	
	void handleReturnData(s32 p_statusCode, const Data& p_data, HttpResponseHandler* p_responseHandler);
	
#if !defined(TT_BUILD_FINAL)
	static const char* getDownloadStateName(DownloadState p_state);
#else
	static inline const char* getDownloadStateName(DownloadState) { return ""; }
#endif
	
#if !defined(_XBOX)
	static void CALLBACK statusCallback(HINTERNET p_handle, DWORD_PTR p_context, DWORD p_internetStatus,
	                                    LPVOID p_statusInformation, DWORD p_statusInformationLength);
	
	static void reportHttpError(const char* p_internalMessage, DownloadInfo& p_info);
#endif
	
	// No copying
	HttpConnectMgr(const HttpConnectMgr&);
	HttpConnectMgr& operator=(const HttpConnectMgr&);
	
	
#if !defined(_XBOX)
	HINTERNET       m_session;
	DownloadMapping m_downloadTracking;
#endif
};

// Namespace end
}
}


#endif  // !defined(INC_TT_HTTP_HTTPCONNECTMGRIMPLWIN_H)
