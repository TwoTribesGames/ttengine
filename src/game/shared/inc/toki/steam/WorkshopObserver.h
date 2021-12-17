#if !defined(INC_TOKI_STEAM_WORKSHOPOBSERVER_H)
#define INC_TOKI_STEAM_WORKSHOPOBSERVER_H


#if defined(TT_STEAM_BUILD)  // Steam Workshop functionality is only available in Steam builds


#include <steam/isteamremotestorage.h>

#include <toki/steam/fwd.h>


namespace toki {
namespace steam {

class WorkshopObserver
{
public:
	enum FileAction
	{
		FileAction_Subscribed,
		FileAction_Unsubscribed,
		FileAction_DetailsAvailable,
		FileAction_GlobalVotesAvailable,
		FileAction_UserVotesAvailable,
		FileAction_FileDownloaded,
		FileAction_PreviewDownloaded,
		FileAction_FileDownloadedToCustomLocation  // file was downloaded to a client-specified target path (NOT IMPLEMENTED YET)
	};
	
	
	WorkshopObserver() { }
	virtual ~WorkshopObserver() { }
	
	/*! \brief Called when something about a Workshop file changed.
	    \param p_id     The file that changed.
	    \param p_action What changed about the file. */
	virtual void onWorkshopFileChange(PublishedFileId_t p_id, FileAction p_action)
	{ (void)p_id; (void)p_action; }
};

// Namespace end
}
}


#endif  // defined(TT_STEAM_BUILD)

#endif  // !defined(INC_TOKI_STEAM_WORKSHOPOBSERVER_H)
