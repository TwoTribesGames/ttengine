#if !defined(INC_TT_INPUT_CONTROLLERTYPE_H)
#define INC_TT_INPUT_CONTROLLERTYPE_H


namespace tt    /*! */ {
namespace input /*! */ {


/*! \brief ControllerType - enumeration of different controller types */
enum ControllerType
{
	ControllerType_Keyboard,          //!< ControllerType_Keyboard (Keyboard & Mouse)
	ControllerType_GenericController, //!< ControllerType_GenericController
	ControllerType_Xbox360Controller, //!< ControllerType_Xbox360Controller
	
	ControllerType_Count,
	ControllerType_Invalid
};


inline bool isValidControllerType(ControllerType p_type)
{
	return p_type >= 0 && p_type < ControllerType_Count;
}


/*! \brief Returns the controller type of the controller that is currently used by the player */
ControllerType getCurrentControllerType();

/*! \brief Forces the controller type of the controller that is currently used by the player */
void           setCurrentControllerType(ControllerType p_type);

/*! \brief Controller type is determined by input (through onControllerTypeUsed callback) */
void           resetCurrentControllerType();

void           updateCurrentControllerType();
void           onControllerTypeUsed(ControllerType p_type);


// Namespace end
}
}


#endif  // !defined(INC_TT_INPUT_CONTROLLERTYPE_H)
