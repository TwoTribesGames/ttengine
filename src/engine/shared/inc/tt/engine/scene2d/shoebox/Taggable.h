#if !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGGABLE_H)
#define INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGGABLE_H

#include <string>
#include <vector>


namespace tt {
namespace engine {
namespace scene2d {
namespace shoebox {


class Taggable
{
public:
	inline Taggable() : m_tags() { }
	Taggable(const Taggable& p_rhs);
	
	/*! \brief Destroys a Taggable object and unregisters it with the tag mgr.*/
	virtual ~Taggable();
	
	const Taggable& operator=(const Taggable& p_rhs);
	
	/*! \brief Adds a tag to the Taggable object and registers itself with the tag mgr.*/
	void addTag(const std::string& p_tag);
	
	/*! \brief Handles a tag event.
	    \param p_event The event to handle.
	    \param p_param Parameter for the event.
	    \return Whether any errors occurred or not.*/
	virtual bool handleEvent(const std::string& p_event, const std::string& p_param) = 0;
	
	inline bool hasTags() const { return m_tags.empty() == false; }
private:
	typedef std::vector<std::string> Tags;
	
	Tags m_tags; //!< The Taggable object's tag ids.
};


// Namespace end
}
}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_SHOEBOX_TAGGABLE_H)
