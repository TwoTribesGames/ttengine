#if !defined(INC_TT_PRES_TRIGGERINTERFACE_H)
#define INC_TT_PRES_TRIGGERINTERFACE_H


#include <tt/platform/tt_types.h>

#include <tt/pres/fwd.h>


namespace tt {
namespace pres {


// Forward Declaration
class DataTags;


class TriggerInterface
{
public:
	TriggerInterface(){}
	virtual ~TriggerInterface(){}
	
	/*! \brief Update logic of this trigger.
	    \param p_delta Time passed in seconds.*/
	virtual void update(real p_delta) = 0;
	
	/*! \brief Returns whether the trigger is active.*/
	virtual bool isActive() const = 0;
	
	/*! \brief Returns whether the trigger is started with the tag filtering.*/
	virtual bool hasNameOrTagMatch() const = 0;
	
	/*! \brief Returns the tags of this Trigger. */
	virtual const DataTags& getTags() const = 0;
	
	/*! \brief Gets whether the trigger is continuous or not.*/
	virtual bool isLooping() const = 0;
	
	/*! \brief Starts the animations with the specified tags.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_presObj The owning presentation object.
	    \param p_name Name of the animation to start.
	    \param p_restart Whether this trigger is restarted by a sync event. */
	virtual void start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name, bool p_restart = false) = 0;
	
	/*! \brief Stops the animation.*/
	virtual void stop() = 0;
	
	/*! \brief Pauses the animation.*/
	virtual void pause() = 0;
	
	/*! \brief Resumes the animation.*/
	virtual void resume() = 0;
	
	/*! \brief Unpauses and stops the animation, sets time to 0.*/
	virtual void reset() = 0;
	
	/*! \brief Gets called when the presentation is inactive.*/
	virtual void presentationEnded() = 0;
	
	virtual TriggerInterface* clone() const = 0;
	
	/*! \brief Sets the presentationObject after cloning for triggers that need information of it.*/
	virtual void setPresentationObject(const PresentationObjectPtr& p_object ) = 0;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TRIGGERINTERFACE_H)
