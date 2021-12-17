#if !defined(INC_TT_CODE_ERRORSTATUS_H)
#define INC_TT_CODE_ERRORSTATUS_H


#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <tt/platform/tt_types.h>
#include <tt/platform/tt_compiler.h>
#include <tt/code/Uncopyable.h>
#include <tt/code/SrcPosNested.h>

#if !defined(TT_BUILD_FINAL)
#	include <tt/thread/CriticalSection.h>
#	include <tt/thread/Mutex.h>
#endif


namespace tt {
namespace code {

#if defined(_MSC_VER)
#pragma warning(disable: 4127)
#endif


// TT_ERR macros with explicit object naming.
// Has _NAME postfix.
// It's recommended to use the implicit macros. That one can be found a bit lower.

#if !defined(TT_BUILD_FINAL)

//==================================================================================================
// Real (non-final) implementations of error macros

#define TT_ERROR_NAME(ErrorStatusObject, MessageStream) \
	do { tt::thread::CriticalSection criticalSection(&tt::code::ErrorStatus::ms_mutex); \
	ErrorStatusObject.USE_MACRO_error(__FILE__, __LINE__, TT_FUNC_SIG) << MessageStream; } while(0)

#define TT_ERR_SET_LOC_NAME(ErrorStatusObject, LocationStream) \
	do { tt::thread::CriticalSection criticalSection(&tt::code::ErrorStatus::ms_mutex); \
	ErrorStatusObject.USE_MACRO_setLocation(__FILE__, __LINE__, TT_FUNC_SIG) << LocationStream; } while(0)

#define TT_ERR_ADD_LOC_NAME(ErrorStatusObject, LocationStream) \
	do { tt::thread::CriticalSection criticalSection(&tt::code::ErrorStatus::ms_mutex); \
	ErrorStatusObject.USE_MACRO_addLocation(__FILE__, __LINE__, TT_FUNC_SIG) << LocationStream; } while(0)

#else


//==================================================================================================
// Dummy no-op (final) implementations of assert macros

#if defined(_MSC_VER) && _MSC_VER >= 1600

// Special-cased dummy implementations for Visual Studio 2010,
// where sizeof(x) does not actually use x (still triggers warnings)

#define TT_ERROR_NAME(ErrorStatusObject, MessageStream) \
	do \
	{ \
		ErrorStatusObject.USE_MACRO_error(); \
		((void)(true ? 0 : ((void)(std::ostringstream() << "" << MessageStream), void(), 0))); \
	} while(0)

#define TT_ERR_SET_LOC_NAME(ErrorStatusObject, LocationStream) \
	do \
	{ \
		(void)ErrorStatusObject; \
		((void)(true ? 0 : ((void)(std::ostringstream() << "" << LocationStream), void(), 0))); \
	} while(0)

#define TT_ERR_ADD_LOC_NAME(ErrorStatusObject, LocationStream) \
	do \
	{ \
		(void)ErrorStatusObject; \
		((void)(true ? 0 : ((void)(std::ostringstream() << "" << LocationStream), void(), 0))); \
	} while(0)

#else

// Dummy implementations for all other compilers

#define TT_ERROR_NAME(ErrorStatusObject, MessageStream) \
	do \
	{ \
		ErrorStatusObject.USE_MACRO_error(); \
		(void)sizeof(std::ostringstream() << "" << MessageStream); \
	} while(0)

#define TT_ERR_SET_LOC_NAME(ErrorStatusObject, LocationStream) \
	do \
	{ \
		(void)ErrorStatusObject; \
		(void)sizeof(std::ostringstream() << "" << LocationStream); \
	} while(0)

#define TT_ERR_ADD_LOC_NAME(ErrorStatusObject, LocationStream) \
	do \
	{ \
		(void)ErrorStatusObject; \
		(void)sizeof(std::ostringstream() << "" << LocationStream); \
	} while(0)

#endif  // defined(_MSC_VER) && _MSC_VER >= 1600

#endif  // defined(TT_BUILD_FINAL)


#define TT_ERR_CREATE_NAME(ErrorStatusObject, LocationStream) \
tt::code::ErrorStatus ErrorStatusObject(""); \
TT_ERR_SET_LOC_NAME(ErrorStatusObject, LocationStream)

#define TT_ERR_CHAIN_NAME(ErrorStatusParent, ErrorStatusName, ReturnType, ReturnValue, LocationStream) \
tt::code::ErrorStatusChain<ReturnType> ErrorStatusName(ErrorStatusParent, ReturnValue); \
if (ErrorStatusName.hasError()) { return ErrorStatusName.getReturnType(); } \
TT_ERR_SET_LOC_NAME(ErrorStatusName, LocationStream)

#define TT_ERR_CHAIN_VOID_NAME(ErrorStatusParent, ErrorStatusName, LocationStream) \
tt::code::ErrorStatusChainVoid ErrorStatusName(ErrorStatusParent); \
if (ErrorStatusName.hasError()) { return ErrorStatusName.getReturnType(); } \
TT_ERR_SET_LOC_NAME(ErrorStatusName, LocationStream)

#define TT_ERR_ASSERTMSG_NAME(ErrorStatusObject, Check, MessageStream) \
do { if ((Check) == false) { TT_ERROR_NAME(ErrorStatusObject, MessageStream); \
     return ErrorStatusObject.getReturnType(); } } while(0)

#define TT_ERR_NULL_ASSERT_NAME(ErrorStatusObject, Pointer) \
TT_ERR_ASSERTMSG_NAME(ErrorStatusObject, Pointer != 0, "The pointer '" #Pointer "' is null.")

#define TT_ERR_ASSERT_NAME(ErrorStatusObject, Check) \
TT_ERR_ASSERTMSG_NAME(ErrorStatusObject, Check, "The check '" #Check "' failed.")

#define TT_ERR_RETURN_ON_ERROR_NAME(ErrorStatusObject) \
do { if (ErrorStatusObject.hasError()) { return ErrorStatusObject.getReturnType(); } } while(0)

#define TT_ERR_ASSERT_ON_ERROR_NAME(ErrorStatusObject) \
TT_ASSERTMSG(ErrorStatusObject.hasError() == false, "ErrorStatus has an error:\n%s", \
             ErrorStatusObject.getErrorMessage().c_str());

#define TT_ERR_AND_RETURN_NAME(ErrorStatusObject, ErrorMessageStream) \
do { TT_ERROR_NAME(ErrorStatusObject, ErrorMessageStream); return ErrorStatusObject.getReturnType(); } while(0)

//
// Implicit object naming.
// It's recommended that you use these and not the explicit once above.
//
// These map p_errStatus and errStatus to the explicit (*_NAME) macros above.

#define TT_ERROR(MessageStream) TT_ERROR_NAME(errStatus, MessageStream)
#define TT_ERR_SET_LOC(LocationStream) TT_ERR_SET_LOC_NAME(errStatus, LocationStream)
#define TT_ERR_ADD_LOC(LocationStream) TT_ERR_ADD_LOC_NAME(errStatus, LocationStream)
#define TT_ERR_CREATE(LocationStream) TT_ERR_CREATE_NAME(errStatus, LocationStream)
#define TT_ERR_CHAIN(ReturnType, ReturnValue, LocationStream) \
	TT_ERR_CHAIN_NAME(p_errStatus, errStatus, ReturnType, ReturnValue, LocationStream)
#define TT_ERR_CHAIN_VOID(LocationStream) TT_ERR_CHAIN_VOID_NAME(p_errStatus, errStatus, LocationStream)
#define TT_ERR_ASSERTMSG(Check, MessageStream) TT_ERR_ASSERTMSG_NAME(errStatus, Check, MessageStream)
#define TT_ERR_NULL_ASSERT(Pointer) TT_ERR_NULL_ASSERT_NAME(errStatus, Pointer)
#define TT_ERR_ASSERT(Check) TT_ERR_ASSERT_NAME(errStatus, Check)
#define TT_ERR_RETURN_ON_ERROR() TT_ERR_RETURN_ON_ERROR_NAME(errStatus)
#define TT_ERR_ASSERT_ON_ERROR() TT_ERR_ASSERT_ON_ERROR_NAME(errStatus)
#define TT_ERR_AND_RETURN(ErrorMessageStream) TT_ERR_AND_RETURN_NAME(errStatus, ErrorMessageStream)



class ErrorStatus : private Uncopyable
{
public:
	/*! \brief Constructor for a parent (root) ErrorStatus object.
	    \param p_locationStr A description of the location of this Error Object.
	                         This is used to indicate to the user where an error occured (in which context). */
	ErrorStatus(const std::string& p_locationStr);
	
	/*! \brief Constructor for a child ErrorStatus object.
	    \param p_parent Pointer to parent object. Errors are pushed to the parent object when triggered.
	                    This pointer can be null which indicates that no error checking should be done.
	    \param p_locaitonStr A description of the location of this Error Object.
	                         This is used to indicate to the user where an error occured (in which context).
	                         An empty string means that this child doesn't add a location to the stack.*/
	ErrorStatus(ErrorStatus* p_parent, const std::string& p_locationStr = "");
	
	virtual ~ErrorStatus();
	
	/*! \return True if there was an error somewhere in the ErrorStatus chain. */
	inline bool hasError() const { return m_hasError; }
	
	/*! \return A formatted error message which includes the location stack.
	    \param p_withSourceInfo indicates if the location source position should be printed.
	                            (file, line, function). */
	std::string getErrorMessage(ReportLevel p_reportLevel = ReportLevel_ErrorOnly) const;
	
	/*! \brief Resets the error status.
	           hasError will return false until the next error is triggered.
	           All the error information such as message and file, line and function are also reset.
	           The location stays unaltered.
	    \note  The reset will NOT travel through the chain. (parent or child).
	           It will only reset the error status on THIS object. */
	void resetError();
	
	/* Warning functions. (Warnings are non-fatal errors.) */
	
	/*! \brief If this object is in an error status the object is reset. (resetError() is called.)
	           All the error information is copied to a child error object which is stored as warning
	           This warning object is passed to parent ErrorStatus just like normal error information.
	           It's possible to store mutiple warnings.
	           Non-like errors, warnings are not passed to children.*/
	void demoteToWarning();
	
	/*! \return Returns true if this object as warning objects. */
	inline bool hasWarnings() const 
	{
#ifdef TT_BUILD_FINAL
		return m_warnings > 0;
#else
		return m_warnings.empty() == false; 
#endif
	}
	
	/*! \return The number of warnings objects stored*/
	inline s32 getWarningCount() const 
	{
#ifdef TT_BUILD_FINAL
		return m_warnings;
#else
		return static_cast<s32>(m_warnings.size());
#endif
	}
	
	/*! \brief Accessor for warnings.
	    \param p_index The index which indicates which warnings object should be returned.
	    \return pointer to an ErrorStatus object (in warning mode) if one exists. 
	            (returns null if p_index is out of bound.) */
	const SrcPosNested getWarning(s32 p_index) const;
	
// Private

#ifdef TT_BUILD_FINAL
	/*! \brief Trigger an error. (DON'T CALL THIS FUNCTION DIRECTLY! USE TT_ERROR! ) */
	inline void USE_MACRO_error() { if (m_ignoreErrors) return; m_hasError = true; }
#else
	/*! \brief Trigger an error. (DON'T CALL THIS FUNCTION DIRECTLY! USE TT_ERROR! ) */
	std::ostream& USE_MACRO_error(const char* p_file = 0, int p_line = 0, const char* p_function = 0);
	
	/*! \brief Overwrite the current locationStr. (DON'T CALL THIS FUNCTION DIRECTLY! USE TT_ERR_SET_LOC! ) */
	std::ostream& USE_MACRO_setLocation(const char* p_file = 0, int p_line = 0, const char* p_function = 0);
	
	/*! \brief Overwrite the current locationStr. (DON'T CALL THIS FUNCTION DIRECTLY! USE TT_ERR_ADD_LOC! ) */
	std::ostream& USE_MACRO_addLocation(const char* p_file = 0, int p_line = 0, const char* p_function = 0);
#endif
	
#if !defined(TT_BUILD_FINAL)
	static thread::Mutex ms_mutex;
#endif
	
private:
	typedef std::vector<SrcPosNested> SrcPosNestedContainer;
	/*! \brief Children should call this function of their parent to notify it of an error. */
	void childHasError(const SrcPosNested& p_errorInfo);
#ifndef TT_BUILD_FINAL
	void addFullLocTo(SrcPosNested* p_srcPos);
	void childHasWarnings(const SrcPosNestedContainer& p_warnings);
#else
	void childHasWarnings(s32 p_warnings);
#endif
	
	ErrorStatus* m_parent; /*!< \brief Pointer to parent status. 
	                            \note Can be null for root parent or a child which should ignore errors. */
	bool m_hasError;       /*!< \brief Flag indicating the error status of this ErrorStatus 
	                                   (or one of its children.) */
	bool m_ignoreErrors;   /*!< \brief If a child is created with a null parent this means that 
	                                   client code passed a null pointer in a function.
	                                   That means that errors should be ignored. */
	
#ifndef TT_BUILD_FINAL
	// Location Info
	SrcPos             m_locInfo;     //!< \brief The location information.
	std::ostringstream m_locationStr; /*!< \brief String with this ErrorStatus location.
	                                       \note  When an error is triggered it will include the locations 
	                                              of its children. */
	
	// Error Info
	SrcPosNested       m_errorInfo; //!< \brief The location information.
	std::ostringstream m_errorMsg;  /*!< The message describing what the error is. 
	                                     (Empty when there is no error.) */
	
	SrcPosNestedContainer m_warnings;
	
	/*! \brief std::ostream that can be returned when input should be ignored. 
	    \note TODO: Make this stream actually a null stream. */
	static std::ostringstream ms_nullStream;
#else
	s32 m_warnings;
#endif // #ifndef TT_BUILD_FINAL
};


template< typename Type>
class ErrorStatusChain : public ErrorStatus
{
public:
	typedef Type ReturnValueType;
	ErrorStatusChain(ErrorStatus* p_parent, ReturnValueType p_returnValue)
	:
	ErrorStatus(p_parent),
	m_returnValue(p_returnValue)
	{ }
	inline ReturnValueType getReturnType() const { return m_returnValue; }
private:
	const ReturnValueType m_returnValue;
};


class ErrorStatusChainVoid : public ErrorStatus
{
public:
	ErrorStatusChainVoid(ErrorStatus* p_parent) : ErrorStatus(p_parent) {}
	inline void getReturnType() const {}
};


// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_ERRORSTATUS_H)
