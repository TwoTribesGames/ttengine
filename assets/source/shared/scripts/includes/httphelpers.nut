::g_queuedHttpRequests <- {};

function sendGetRequestWithResponse(p_callbackEntity, p_server, p_url, p_params)
{
	local id = ::sendGetRequest(p_server, p_url, p_params);
	if (id >= 0 && p_callbackEntity != null)
	{
		::g_queuedHttpRequests[id] <- p_callbackEntity.weakref();
	}
}

function onHTTPResponseReceived(p_requestID, p_statusID, p_data)
{
	if (p_requestID in ::g_queuedHttpRequests)
	{
		// Forward to entity
		local entity = ::g_queuedHttpRequests[p_requestID];
		if (entity != null)
		{
			entity.customCallback("onHTTPResponseReceived", p_requestID, p_statusID, p_data);
		}
		delete ::g_queuedHttpRequests[p_requestID];
		return;
	}
	::echo("GET [" + p_requestID + "]: " + p_data); 
}
