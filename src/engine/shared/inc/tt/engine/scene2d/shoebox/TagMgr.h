#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGMGR_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGMGR_H

#include <string>
#include <map>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {

class Taggable;

class TagMgr
{
public:
	/*! \brief Registers a Taggable object.
	    \param p_tag The tag by which to register the object.
	    \param p_tagable The object to register.
	    \return Whether registering succeeded or not.*/
	static bool registerTaggable(const std::string& p_tag, Taggable* p_tagable);
	
	/*! \brief Unregisters a Taggable object.
	    \param p_tag The tag by which to unregister the object.
	    \param p_tagable The object to unregister.
	    \return Whether unregistering succeeded or not.*/
	static bool unregisterTaggable(const std::string& p_tag, Taggable* p_tagable);
	
	/*! \brief Sends an event to all Tagable objects with the specified tag.
	    \param p_tag The tag of the objects which should handle the event.
	    \param p_event The event to be handled.
	    \param p_event Parameter for the event.
	    \return False when one of the Tagable objects encountered an error.*/
	static bool sendEvent(const std::string& p_tag, const std::string& p_event,
	                      const std::string& p_param);
	
private:
	// uncopyable
	TagMgr();
	TagMgr(const TagMgr&);
	~TagMgr();
	const TagMgr& operator=(const TagMgr&);
	
	typedef std::multimap<std::string, Taggable*> Taggables;
	
	static Taggables ms_taggables; //!< All registered Taggable objects.
};


// Namespace end
}
}
}
}


#endif // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGMGR_H)
