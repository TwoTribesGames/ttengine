#if !defined(INC_TT_PRES_TRIGGERINFO_H)
#define INC_TT_PRES_TRIGGERINFO_H


#include <string>

#include <tt/pres/fwd.h>
#include <tt/pres/DataTags.h>
#include <tt/pres/PresentationValue.h>
#include <tt/str/str_types.h>

namespace tt {
namespace pres {


struct TriggerInfo
{
public:
	/*! \brief type of trigger */
	std::string type;
	
	/*! \brief extra data for the trigger */
	std::string data;
	
	/*! \brief Set of hashed tags that this trigger should respond to */
	DataTags dataTags;
	
	/*! \brief delay in seconds before triggering */
	PresentationValue delay;
	
	/*! \brief time the trigger should last */
	PresentationValue duration;
	
	/*! \brief id of an animation where the timing of the trigger should be synced to */
	std::string syncId;
	
	/*! \brief Whether the syncId is an end sync or a normal sync*/
	bool endSync;
	
	/*! \brief Whether the trigger should trigger again after the duration*/
	bool looping;
	
	inline TriggerInfo()
	:
	type(),
	data(),
	dataTags(),
	delay(0),
	duration(0),
	syncId(),
	endSync(false),
	looping(false)
	{ }
	
	
	/*! \brief Loads the info needed to create a trigger from an xml Node
	    \param p_node xml node to load from 
	    \param p_applyTags tags that will be applied aditionaly to the loaded tags 
	    \param p_acceptedTags will assert if a loaded tag is not in this set of tags
	    \param p_errStatus error object
	    \return a filled in TriggerInfo struct on succes a default one on failure */ 
	static TriggerInfo loadXml(const xml::XmlNode* p_node, 
	                           const DataTags& p_applyTags, 
	                           const Tags& p_acceptedTags,
	                           code::ErrorStatus* p_errStatus);
	
	bool loadXmlCommon(const xml::XmlNode* p_node, 
	                   const DataTags& p_applyTags, 
	                   const Tags& p_acceptedTags,
	                   code::ErrorStatus* p_errStatus);
	
	static TriggerInfo loadBin(const u8*& p_bufferOUT, size_t& p_sizeOUT,
	                           const DataTags& p_applyTags, const Tags& p_acceptedTags,
	                           code::ErrorStatus* p_errStatus);
	
	
	bool saveBin(u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus) const;
	
	/*! \brief gets the needed buffersize for this struct */ 
	size_t getBufferSize() const;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TRIGGERFACTORYINTERFACE_H)
