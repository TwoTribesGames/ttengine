#if !defined(INC_TT_CODE_SRCPOSNESTED_H)
#define INC_TT_CODE_SRCPOSNESTED_H

#include <vector>

#include <tt/code/SrcPos.h>
#include <tt/platform/tt_types.h>

namespace tt {
namespace code {


enum ReportLevel
{
	ReportLevel_ErrorOnly,                         // Only report the error.
	ReportLevel_ErrorWithSourceLocation,           // Report the error + sourcefile with line.
	ReportLevel_ErrorWithSourceLocationAndFunction // Report the error + sourcefile with line and function sigature.
};


/*
class SrcPosNested;
typedef tt_ptr<      SrcPosNested>::shared      SrcPosNestedPtr;
typedef tt_ptr<const SrcPosNested>::shared ConstSrcPosNestedPtr;
*/
#ifndef TT_BUILD_FINAL
class SrcPosNested : public SrcPos
{
public:
	typedef std::vector<SrcPos> SrcPosContainer;
	
	SrcPosNested(const char* p_file = 0, s32 p_line = 0, const char* p_function = 0, const std::string& p_msg = "")
	:
	SrcPos(p_file, p_line, p_function, p_msg),
	m_path()
	{
	}
	virtual ~SrcPosNested() {}
	
	inline void pushParent(const SrcPos& p_parent) { m_path.push_back(p_parent); }
	inline SrcPosContainer getPath() const { return m_path; }
	inline void reset() { SrcPos::reset(); m_path.clear(); }
	
	//! \brief Get the path information as string.
	std::string getPathStr(ReportLevel p_reportLevel) const;
	//! \brief Get the message with path info as string.
	std::string getFullMessage(ReportLevel p_reportLevel) const;
	
private:
	SrcPosContainer m_path;
};
#else
class SrcPosNested
{
public:
	SrcPosNested() {}
};
#endif // #ifndef TT_BUILD_FINAL

// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_SRCPOSNESTED_H)
