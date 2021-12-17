#if !defined(INC_TT_CODE_SRCPOS_H)
#define INC_TT_CODE_SRCPOS_H

#include <string>

#include <tt/platform/tt_types.h>

namespace tt {
namespace code {

/*
class SrcPos;
typedef tt_ptr<      SrcPos>::shared      SrcPosPtr;
typedef tt_ptr<const SrcPos>::shared ConstSrcPosPtr;
*/

class SrcPos
{
public:
	SrcPos(const char* p_file = 0, s32 p_line = 0, const char* p_function = 0, 
	       const std::string& p_message = "")
	:
	m_file(p_file), m_line(p_line), m_function(p_function), m_message(p_message)
	{
	}
	/*
	SrcPos(const SrcPos& p_copy)
	:
	m_file(p_copy.m_file), m_line(p_copy.m_line), m_function(p_copy.m_function), m_message(p_copy.m_message)
	{
	}
	*/
	virtual ~SrcPos() {}
	
	inline std::string getFile()      const     { return m_file     == 0 ? "" : m_file;     }
	inline s32         getLine()      const     { return m_line;                            }
	inline std::string getFunction()  const     { return m_function == 0 ? "" : m_function; }
	inline const std::string getMsg() const     { return m_message;                         }
	inline void setMsg(const std::string& p_msg) { m_message = p_msg;                        }
	inline void set(const char* p_file, s32 p_line, const char* p_function,
	                const std::string& p_message = "")
	{
		m_file = p_file; m_line = p_line; m_function = p_function; m_message = p_message;
	}
	inline void reset() { set(0,0,0,""); }
	
private:
	const char* m_file;
	s32         m_line;
	const char* m_function;
	std::string m_message;
};


// Namespace end
}
}


#endif  // !defined(INC_TT_CODE_SRCPOS_H)
