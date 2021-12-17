#include <tt/audio/player/Xact3Helpers.h>


namespace tt {
namespace audio {
namespace player {

#if !defined(TT_BUILD_FINAL)

const char* getXactErrorName(HRESULT p_errorCode)
{
	switch (p_errorCode)
	{
	case XACTENGINE_E_OUTOFMEMORY:               return "XACTENGINE_E_OUTOFMEMORY";
	case XACTENGINE_E_INVALIDARG:                return "XACTENGINE_E_INVALIDARG";
	case XACTENGINE_E_NOTIMPL:                   return "XACTENGINE_E_NOTIMPL";
	case XACTENGINE_E_FAIL:                      return "XACTENGINE_E_FAIL";
	case XACTENGINE_E_ALREADYINITIALIZED:        return "XACTENGINE_E_ALREADYINITIALIZED";
	case XACTENGINE_E_NOTINITIALIZED:            return "XACTENGINE_E_NOTINITIALIZED";
	case XACTENGINE_E_EXPIRED:                   return "XACTENGINE_E_EXPIRED";
	case XACTENGINE_E_NONOTIFICATIONCALLBACK:    return "XACTENGINE_E_NONOTIFICATIONCALLBACK";
	case XACTENGINE_E_NOTIFICATIONREGISTERED:    return "XACTENGINE_E_NOTIFICATIONREGISTERED";
	case XACTENGINE_E_INVALIDUSAGE:              return "XACTENGINE_E_INVALIDUSAGE";
	case XACTENGINE_E_INVALIDDATA:               return "XACTENGINE_E_INVALIDDATA";
	case XACTENGINE_E_INSTANCELIMITFAILTOPLAY:   return "XACTENGINE_E_INSTANCELIMITFAILTOPLAY";
	case XACTENGINE_E_NOGLOBALSETTINGS:          return "XACTENGINE_E_NOGLOBALSETTINGS";
	case XACTENGINE_E_INVALIDVARIABLEINDEX:      return "XACTENGINE_E_INVALIDVARIABLEINDEX";
	case XACTENGINE_E_INVALIDCATEGORY:           return "XACTENGINE_E_INVALIDCATEGORY";
	case XACTENGINE_E_INVALIDCUEINDEX:           return "XACTENGINE_E_INVALIDCUEINDEX";
	case XACTENGINE_E_INVALIDWAVEINDEX:          return "XACTENGINE_E_INVALIDWAVEINDEX";
	case XACTENGINE_E_INVALIDTRACKINDEX:         return "XACTENGINE_E_INVALIDTRACKINDEX";
	case XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX: return "XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX";
	case XACTENGINE_E_READFILE:                  return "XACTENGINE_E_READFILE";
	case XACTENGINE_E_UNKNOWNEVENT:              return "XACTENGINE_E_UNKNOWNEVENT";
	case XACTENGINE_E_INCALLBACK:                return "XACTENGINE_E_INCALLBACK";
	case XACTENGINE_E_NOWAVEBANK:                return "XACTENGINE_E_NOWAVEBANK";
	case XACTENGINE_E_SELECTVARIATION:           return "XACTENGINE_E_SELECTVARIATION";
	case XACTENGINE_E_AUDITION_WRITEFILE:        return "XACTENGINE_E_AUDITION_WRITEFILE";
	case XACTENGINE_E_AUDITION_NOSOUNDBANK:      return "XACTENGINE_E_AUDITION_NOSOUNDBANK";
	case XACTENGINE_E_AUDITION_INVALIDRPCINDEX:  return "XACTENGINE_E_AUDITION_INVALIDRPCINDEX";
	case XACTENGINE_E_AUDITION_MISSINGDATA:      return "XACTENGINE_E_AUDITION_MISSINGDATA";
	case XACTENGINE_E_AUDITION_UNKNOWNCOMMAND:   return "XACTENGINE_E_AUDITION_UNKNOWNCOMMAND";
	}
	
	return "[unknown]";
}


const char* getXactErrorDesc(HRESULT p_errorCode)
{
	switch (p_errorCode)
	{
	case XACTENGINE_E_OUTOFMEMORY:               return "The system has run out of memory.";
	case XACTENGINE_E_INVALIDARG:                return "One or more arguments to a function or method are invalid.";
	case XACTENGINE_E_NOTIMPL:                   return "The called function or method has not been implemented.";
	case XACTENGINE_E_FAIL:                      return "A failure of an unspecified type has occured.";
	case XACTENGINE_E_ALREADYINITIALIZED:        return "The engine has already been initialized.";
	case XACTENGINE_E_NOTINITIALIZED:            return "The engine has not yet been initialized.";
	case XACTENGINE_E_EXPIRED:                   return "The engine is a pre-release version and has expired.";
	case XACTENGINE_E_NONOTIFICATIONCALLBACK:    return "No notification callback has been registered with the engine.";
	case XACTENGINE_E_NOTIFICATIONREGISTERED:    return "A notification callback has already been registered.";
	case XACTENGINE_E_INVALIDUSAGE:              return "The method or function called cannot be used in the manner requested.";
	case XACTENGINE_E_INVALIDDATA:               return "The data used by the method or function is invalid.";
	case XACTENGINE_E_INSTANCELIMITFAILTOPLAY:   return "A sound or cue has reached an instance limit and cannot be played.";
	case XACTENGINE_E_NOGLOBALSETTINGS:          return "Global settings have not been loaded.";
	case XACTENGINE_E_INVALIDVARIABLEINDEX:      return "The specified variable index is invalid.";
	case XACTENGINE_E_INVALIDCATEGORY:           return "The specified category is invalid.";
	case XACTENGINE_E_INVALIDCUEINDEX:           return "The specified cue index is invalid.";
	case XACTENGINE_E_INVALIDWAVEINDEX:          return "The specified wave index is invalid.";
	case XACTENGINE_E_INVALIDTRACKINDEX:         return "The specified track index is invalid.";
	case XACTENGINE_E_INVALIDSOUNDOFFSETORINDEX: return "The specified sound offset or index is invalid.";
	case XACTENGINE_E_READFILE:                  return "An error has occured while attempting to read a file.";
	case XACTENGINE_E_UNKNOWNEVENT:              return "The event specified could not be found.";
	case XACTENGINE_E_INCALLBACK:                return "An invalid method or function was called during a callback function.";
	case XACTENGINE_E_NOWAVEBANK:                return "No wavebank exists for the requested operation.";
	case XACTENGINE_E_SELECTVARIATION:           return "A variation could not be selected.";
	case XACTENGINE_E_AUDITION_WRITEFILE:        return "An error has occured while writing a file during auditioning.";
	case XACTENGINE_E_AUDITION_NOSOUNDBANK:      return "A required sound bank is missing.";
	case XACTENGINE_E_AUDITION_INVALIDRPCINDEX:  return "A required wave bank is missing.";
	case XACTENGINE_E_AUDITION_MISSINGDATA:      return "A required set of data is missing for the requested audition command.";
	case XACTENGINE_E_AUDITION_UNKNOWNCOMMAND:   return "The audition command requested is not known.";
	}
	
	return "[unknown]";
}

#endif  // !defined(TT_BUILD_FINAL)

// Namespace end
}
}
}
