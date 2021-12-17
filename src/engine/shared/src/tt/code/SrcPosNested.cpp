#ifndef TT_BUILD_FINAL

#include <sstream>

#include <tt/code/SrcPosNested.h>


namespace tt {
namespace code {


//--------------------------------------------------------------------------------------------------
// Public member functions


std::string SrcPosNested::getPathStr(ReportLevel p_reportLevel) const
{
	std::ostringstream oss;
	for (SrcPosContainer::const_iterator it = m_path.begin(); it != m_path.end(); ++it)
	{
		bool lastPos = it + 1 == m_path.end();
		
		std::string msg((*it).getMsg());
		// When reporting errors only empty message should be ignored.
		if (p_reportLevel != ReportLevel_ErrorOnly || msg.empty() == false)
		{
			oss << "In: " << msg;
			if (p_reportLevel == ReportLevel_ErrorWithSourceLocation ||
			    p_reportLevel == ReportLevel_ErrorWithSourceLocationAndFunction)
			{
				oss << " (" << (*it).getFile() << ":" << (*it).getLine();
				if (p_reportLevel == ReportLevel_ErrorWithSourceLocationAndFunction)
				{
					oss << " " << (*it).getFunction();
				}
				oss << ")";
			}
			
			if (lastPos == false)
			{
				oss << "\n";
			}
		}
	}
	return oss.str();
}


std::string SrcPosNested::getFullMessage(ReportLevel p_reportLevel) const
{
	std::ostringstream oss;
	oss << getMsg() << "\n";
	oss << getPathStr(p_reportLevel);
	return oss.str();
}

//--------------------------------------------------------------------------------------------------
// Private member functions


// Namespace end
}
}

#endif //#ifndef TT_BUILD_FINAL
