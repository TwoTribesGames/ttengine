#if !defined(INC_TT_PRES_TIMER_H)
#define INC_TT_PRES_TIMER_H

#include <tt/code/ErrorStatus.h>
#include <tt/pres/fwd.h>
#include <tt/pres/DataTags.h>
#include <tt/pres/PresentationValue.h>
#include <tt/xml/XmlNode.h>
#include <tt/str/str_types.h>


namespace tt {
namespace pres {


class Timer
{
public:
	Timer();

	/*! \brief updates the logic of this timer.*/
	void update(real p_delta);
	
	/*! \brief Starts the timer.
	    \param p_tags A container of Tags the animations will be started with.
	    \param p_customValues Any custom values found, will be replaced by values in this container. */
	void start(const Tags& p_tags, PresentationObject* p_presObj, const std::string& p_name);
	
	/*! \brief Stops the timer.*/
	void stop();
	
	/*! \brief Pauses the timer.*/
	void pause();
	
	/*! \brief Resumes the timer.*/
	void resume();
	
	/*! \brief Unpauses and stops the timer, sets time to 0.*/
	void reset();
	
	/*! \brief Returns whether the timer is active.
	    \return Whether the timer is active.*/
	inline bool isActive() const { return m_active; }

	/*! \brief Returns whether the timer is started with tag filtering.
	    \return Whether the timer is tagged.*/
	inline bool hasNameOrTagMatch() const { return m_tagged; }
	
	/*! \brief Returns the tags of this Timer. */
	inline const DataTags& getTags() const { return m_tags; }
	
	/*! \brief Sets the duration of the timer.
	    \param p_duration The duration type of the timer.*/
	void setDuration(real p_duration);
	
	/*! \brief Gets the duration of the timer.
	    \return The duration of the timer.*/
	inline const PresentationValue& getDuration() const { return m_duration; }
	
	/*! \brief Loads the timer from an xml node.
	    \param p_node The xml node to load from.
	    \param p_applyTags Additional tags that should be applied to this timer.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const xml::XmlNode* p_node, 
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus);
	
	
	/*! \brief Needed for StackBase. always returns false a timer cannot loop*/
	inline bool isLooping() { return false; }
	
	/*! \brief Saves the timer to a buffer.*/
	bool save( u8*& p_bufferOUT, size_t& p_sizeOUT, code::ErrorStatus* p_errStatus );
	
	/*! \brief Loads the timer from a buffer.
	    \param p_bufferOUT The buffer to load from, will be updated.
	    \param p_sizeOUT The size of the buffer, will be updated
	    \param p_applyTags Additional tags that should be applied to this timer.
	    \param p_acceptedTags List of accepted tags where loaded tags should be checked against.
	    \return Whether loading succeeded or not.*/
	bool load(const u8*& p_bufferOUT, size_t& p_sizeOUT,
	          const DataTags& p_applyTags, 
	          const Tags& p_acceptedTags,
	          code::ErrorStatus* p_errStatus);
	
	/*! \brief Returns the size of the buffer needed to load or save this stack.
	    \return The size of the buffer.*/
	size_t getBufferSize() const;
	
	/*! \brief Returns a clone of the timer object.*/
	inline Timer* clone() const { return new Timer(*this); }
	
private:
	void setRanges(PresentationObject* p_presObj);
	
	bool m_active;
	bool m_paused;
	
	PresentationValue m_duration;
	real              m_time;
	
	DataTags m_tags;
	bool m_tagged;
	
	//Enable copy / disable assignment
	Timer& operator=(const Timer&) = delete;
};


//namespace end
}
}

#endif // !defined(INC_TT_PRES_TIMER_H)
