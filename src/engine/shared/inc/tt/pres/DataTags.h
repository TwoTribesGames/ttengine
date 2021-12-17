#if !defined(INC_TT_PRES_DATATAGS_H)
#define INC_TT_PRES_DATATAGS_H


#include <tt/platform/tt_types.h>
#include <tt/pres/fwd.h>
#include <tt/str/str_types.h>
#include <tt/xml/fwd.h>

// Forward Declarations
namespace tt
{
	namespace code
	{
		class ErrorStatus;
	}
}


namespace tt {
namespace pres {


/*! \brief Represents the tags of an animation as specified in data. */
class DataTags
{
public:
	/*! \brief Loads the tags from memory.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Data tags that need to be applied additionaly to the loaded tags.
	    \param p_acceptedTags Loaded tags will be checked against this list if it is not empty.
	                          Will assert if non accepted tags are loaded.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT, const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags, code::ErrorStatus* p_errStatus);

	/*! \brief Whether this DataTags is Tagged with the given tag set.
	    \param p_startTags The tags this animation is started with.
	    \return Whether this DataTags is Tagged with the given tag set..*/
	bool shouldPlay(const Tags& p_startTags, const std::string& p_name);
	
	/*! \brief Adds all the tags from another dataTags object.*/
	void addDataTags(const DataTags& p_applyTags);
	
	Tags getAllUsedTags() const;
	
#if !defined(TT_BUILD_FINAL)
	/*! \brief Loads the tags from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Data tags that need to be applied additionaly to the loaded tags.
	    \param p_acceptedTags Loaded tags will be checked against this list if it is not empty.
	                          Will assert if non accepted tags are loaded.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, const DataTags& p_applyTags, const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus);
	
	/*! \brief Save the tags to memory.
	    \param p_bufferOUT The buffer to save to, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \return Whether saving succeeded or not.*/
	bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;

	/*! \brief Returns the size of the buffer needed to load or save.
	    \return The size of the buffer.*/
	size_t getBufferSize() const;

	std::string getDebugString() const;
	
	str::StringSet getAllUsedNames()const;
	
	inline const std::string& getName() const { return m_name; }
#else
	// FIXME: Make sure these aren't needed in final builds
	inline bool load(const xml::XmlNode*, const DataTags&, const Tags&, code::ErrorStatus*) { return false; }
	inline size_t getBufferSize() const { return 0; }
	inline bool save(u8*&, size_t&, code::ErrorStatus*) const { return false; }
	inline const std::string getName() const { return ""; }
#endif
	
private:
	bool loadTags(const xml::XmlNode* p_node, const Tags& p_acceptedTags, Tags* p_tagsOut,
	              str::Strings* p_strTagsOut, code::ErrorStatus* p_errStatus);
	
	typedef std::vector<Tags> TagGroup;
	typedef std::vector<str::Strings> TagStrGroup;
	
	TagGroup m_mustHaves;
	TagGroup m_mustNotHaves;
	Tag      m_nameTag;
	
#if !defined(TT_BUILD_FINAL)
	std::string    m_name;
	str::StringSet m_allUsedNames;

	// string versions of tags needed and used only for conversion
	TagStrGroup m_mustHavesStr;
	TagStrGroup m_mustNotHavesStr;
#endif
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_DATATAGS_H)
