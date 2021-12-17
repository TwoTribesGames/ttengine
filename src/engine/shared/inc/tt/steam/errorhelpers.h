#if !defined(INC_STEAM_ERRORHELPERS_H)
#define INC_STEAM_ERRORHELPERS_H

#if !defined(TT_PLATFORM_OSX_IPHONE)  // Steam support works on Mac OS X, but not on iOS


#include <steam/steam_api.h>


namespace tt {
namespace steam {

inline const char* const getHumanReadableResult(EResult p_resultCode)
{
	switch (p_resultCode)
	{
	case k_EResultOK:                                       return "success";
	case k_EResultFail:                                     return "generic failure";
	case k_EResultNoConnection:                             return "no/failed network connection";
	case k_EResultInvalidPassword:                          return "password/ticket is invalid";
	case k_EResultLoggedInElsewhere:                        return "same user logged in elsewhere";
	case k_EResultInvalidProtocolVer:                       return "protocol version is incorrect";
	case k_EResultInvalidParam:                             return "a parameter is incorrect";
	case k_EResultFileNotFound:                             return "file was not found";
	case k_EResultBusy:                                     return "called method busy - action not taken";
	case k_EResultInvalidState:                             return "called object was in an invalid state";
	case k_EResultInvalidName:                              return "name is invalid";
	case k_EResultInvalidEmail:                             return "email is invalid";
	case k_EResultDuplicateName:                            return "name is not unique";
	case k_EResultAccessDenied:                             return "access is denied";
	case k_EResultTimeout:                                  return "operation timed out";
	case k_EResultBanned:                                   return "VAC2 banned";
	case k_EResultAccountNotFound:                          return "account not found";
	case k_EResultInvalidSteamID:                           return "Steam ID is invalid";
	case k_EResultServiceUnavailable:                       return "the requested service is currently unavailable";
	case k_EResultNotLoggedOn:                              return "the user is not logged on";
	case k_EResultPending:                                  return "request is pending (may be in process, or waiting on third party)";
	case k_EResultEncryptionFailure:                        return "encryption or decryption failed";
	case k_EResultInsufficientPrivilege:                    return "insufficient privilege";
	case k_EResultLimitExceeded:                            return "limit exceeded";
	case k_EResultRevoked:                                  return "access has been revoked (used for revoked guest passes)";
	case k_EResultExpired:                                  return "license/Guest pass the user is trying to access is expired";
	case k_EResultAlreadyRedeemed:                          return "guest pass has already been redeemed by account, cannot be acked again";
	case k_EResultDuplicateRequest:                         return "the request is a duplicate and the action has already occurred in the past, ignored this time";
	case k_EResultAlreadyOwned:                             return "all the games in this guest pass redemption request are already owned by the user";
	case k_EResultIPNotFound:                               return "IP address not found";
	case k_EResultPersistFailed:                            return "failed to write change to the data store";
	case k_EResultLockingFailed:                            return "failed to acquire access lock for this operation";
	case k_EResultLogonSessionReplaced:                     return "logon session replaced";
	case k_EResultConnectFailed:                            return "connect failed";
	case k_EResultHandshakeFailed:                          return "handshake failed";
	case k_EResultIOFailure:                                return "I/O failure";
	case k_EResultRemoteDisconnect:                         return "remote disconnect";
	case k_EResultShoppingCartNotFound:                     return "failed to find the shopping cart requested";
	case k_EResultBlocked:                                  return "a user didn't allow it";
	case k_EResultIgnored:                                  return "target is ignoring sender";
	case k_EResultNoMatch:                                  return "nothing matching the request found";
	case k_EResultAccountDisabled:                          return "account disabled";
	case k_EResultServiceReadOnly:                          return "this service is not accepting content changes right now";
	case k_EResultAccountNotFeatured:                       return "account doesn't have value, so this feature isn't available";
	case k_EResultAdministratorOK:                          return "allowed to take this action, but only because requester is admin";
	case k_EResultContentVersion:                           return "a version mismatch in content transmitted within the Steam protocol";
	case k_EResultTryAnotherCM:                             return "the current CM can't service the user making a request, user should try another";
	case k_EResultPasswordRequiredToKickSession:            return "you are already logged in elsewhere, this cached credential login has failed";
	case k_EResultAlreadyLoggedInElsewhere:                 return "you are already logged in elsewhere, you must wait";
	case k_EResultSuspended:                                return "long running operation (content download) suspended/paused";
	case k_EResultCancelled:                                return "operation canceled (typically by user: content download)";
	case k_EResultDataCorruption:                           return "operation canceled because data is ill formed or unrecoverable";
	case k_EResultDiskFull:                                 return "operation canceled - not enough disk space";
	case k_EResultRemoteCallFailed:                         return "a remote call or IPC call failed";
	case k_EResultPasswordUnset:                            return "password could not be verified as it's unset server side";
	case k_EResultExternalAccountUnlinked:                  return "external account (PSN, Facebook...) is not linked to a Steam account";
	case k_EResultPSNTicketInvalid:                         return "PSN ticket was invalid";
	case k_EResultExternalAccountAlreadyLinked:             return "external account (PSN, Facebook...) is already linked to some other account, must explicitly request to replace/delete the link first";
	case k_EResultRemoteFileConflict:                       return "the sync cannot resume due to a conflict between the local and remote files";
	case k_EResultIllegalPassword:                          return "the requested new password is not legal";
	case k_EResultSameAsPreviousValue:                      return "new value is the same as the old one ( secret question and answer )";
	case k_EResultAccountLogonDenied:                       return "account login denied due to 2nd factor authentication failure";
	case k_EResultCannotUseOldPassword:                     return "the requested new password is not legal";
	case k_EResultInvalidLoginAuthCode:                     return "account login denied due to auth code invalid";
	case k_EResultAccountLogonDeniedNoMail:                 return "account login denied due to 2nd factor auth failure - and no mail has been sent";
	case k_EResultHardwareNotCapableOfIPT:                  return "hardware not capable of IPT";
	case k_EResultIPTInitError:                             return "IPT init error";
	case k_EResultParentalControlRestricted:                return "operation failed due to parental control restrictions for current user";
	case k_EResultFacebookQueryError:                       return "Facebook query returned an error";
	case k_EResultExpiredLoginAuthCode:                     return "account login denied due to auth code expired";
	case k_EResultIPLoginRestrictionFailed:                 return "IP login restriction failed";
	case k_EResultAccountLockedDown:                        return "account locked down";
	case k_EResultAccountLogonDeniedVerifiedEmailRequired:  return "account logon denied: verified email required";
	case k_EResultNoMatchingURL:                            return "no matching URL";
	case k_EResultBadResponse:                              return "parse failure, missing field, etc.";
	case k_EResultRequirePasswordReEntry:                   return "the user cannot complete the action until they re-enter their password";
	case k_EResultValueOutOfRange:                          return "the value entered is outside the acceptable range";
	default:                                                return "unknown";
	}
}

// Namespace end
}
}


#endif  // !defined(TT_PLATFORM_OSX_IPHONE)

#endif // !defined(INC_STEAM_ERRORHELPERS_H)
