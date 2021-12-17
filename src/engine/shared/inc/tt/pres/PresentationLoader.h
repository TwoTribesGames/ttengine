#if !defined(INC_TT_PRES_PRESENTATIONLOADER_H)
#define INC_TT_PRES_PRESENTATIONLOADER_H

#include <tt/code/ErrorStatus.h>
#include <tt/pres/fwd.h>
#include <tt/pres/TriggerInfo.h>
#include <tt/str/str_types.h>
#include <tt/xml/fwd.h>


namespace tt {
namespace pres {


class PresentationLoader
{
public:
	/*! \brief Creates a presentationLoader for conversion*/
	PresentationLoader();

	/*! \brief Creates a presentationLoader for game
	    \param p_acceptedTags accepted tags. Will fail loading if a tag is found that is not in here.
	    \param p_particleCategory Category spawned particles should have. */ 
	PresentationLoader(const Tags& p_acceptedTags, u32 p_particleCategory);
	
	
	/*! \brief Loads a presentationObject from file. Merging all stacks and includes.
	    \param p_object the to-be loaded object
	    \param p_file path + name (no extension) to presentation file. Will load binary if available 
	                  and fall back to xml if there is no binary file
	    \param p_requiredTags Optional required tags. will panic if a tag is not present.
	    \return PresentationObject ready to use */ 
	bool load(const PresentationObjectPtr& p_object,
	          const std::string& p_file,
	          const TriggerFactoryInterfacePtr& p_triggerFactory,
	          const Tags& p_requiredTags = Tags());
	
	/*! \brief Loads a presentationObject from XML for conversion. Holds loaded data internally */
	bool loadXml(xml::XmlNode* p_rootNode, code::ErrorStatus* p_errStatus);
	
	/*! \brief Saves loaded data to buffer */
	bool saveBinary(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	ConstPresentationObjectPtr getLoadedPresentationObject() const{ return m_object; }
	
	/*! \brief gets the needed buffersize for this struct */ 
	size_t getBufferSize() const;
	
	/*! \brief Creates a default presentation Object */ 
	PresentationObjectPtr createDefault(PresentationMgr* p_mgr) const;
	
private:
	/*! \brief determines whether it is an xml or binary file and calls the correct function*/
	bool load(const std::string& p_file,
	          const TriggerFactoryInterfacePtr& p_factory,
	          DataTags& p_applyTags,
	          const Tags& p_acceptedTags,
	          bool p_loadForUsage,
	          code::ErrorStatus* p_errStatus);
	
	bool loadXml(xml::XmlNode* p_rootNode,
	             const TriggerFactoryInterfacePtr& p_factory,
	             DataTags& p_applyTags,
	             const Tags& p_acceptedTags,
	             bool p_loadForUsage,
	             code::ErrorStatus* p_errStatus);
	
	bool parseIncludesXml( const xml::XmlNode* node, const TriggerFactoryInterfacePtr& p_factory, 
	                       const DataTags& p_applyTags, const Tags& p_acceptedTags,
	                       bool p_loadForUsage, code::ErrorStatus* p_errStatus);
	
	
	/*! \brief Saves loaded data to buffer */
	bool save(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus);
	
	bool loadBin(const u8*& p_bufferOUT, size_t& p_sizeOUT, 
	             const TriggerFactoryInterfacePtr& p_factory,
	             const DataTags& p_applyTags,
	             const Tags& p_acceptedTags,
	             code::ErrorStatus* p_errStatus );
	
	bool isTagAccepted(const Tag& p_tag, const Tags& p_acceptedTags)const;
	
	/*! \brief Creates a trigger of a build in type. Or using the factory if the type is unknown */
	TriggerInterfacePtr createTrigger(const TriggerInfo&                p_creationInfo,
	                                  const TriggerFactoryInterfacePtr& p_factory,
	                                  const PresentationObjectPtr&      p_object);
	
	
	// hold data for includes
	struct IncludeInfo
	{
		std::string includeFile;
		DataTags    tags;
		
		
		inline IncludeInfo()
		:
		includeFile(),
		tags()
		{ }
	};
	
	// holds the data needed to save a presentationObject back to file
	typedef std::vector<IncludeInfo> IncludeInfos;
	typedef std::vector<TriggerInfo> TriggerInfos;
	PresentationObjectPtr m_object;
	IncludeInfos m_includes;
	TriggerInfos m_triggerinfos;
	
	
	Tags m_acceptedTags;
	u32  m_particleCategory;
	
	
	// Disable Copy/assignment
	PresentationLoader(const PresentationLoader&);
	PresentationLoader& operator=(const PresentationLoader&);	
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_PRESENTATIONLOADER_H)
