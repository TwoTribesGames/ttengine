#if !defined(INC_TT_STREAMS_BIOSTREAMBASE_H)
#define INC_TT_STREAMS_BIOSTREAMBASE_H


#include <cstddef>

#include <tt/platform/tt_types.h>


namespace tt {
namespace streams {

typedef s32 streamsize;

/*! \brief Binary Input Output Stream Base class. */
class BIOStreamBase
{
public:
	enum IOState
	{
		IOState_GoodBit = 0,
		IOState_BadBit  = 1 << 0,
		IOState_EOFBit  = 1 << 1,
		IOState_FailBit = 1 << 2
	};
	
	
	BIOStreamBase();
	virtual ~BIOStreamBase();
	
	inline bool isGood()    const { return m_state == IOState_GoodBit;                          }
	inline bool isEOF()     const { return (m_state & IOState_EOFBit) == IOState_EOFBit;        }
	inline bool hasFailed() const { return (m_state & (IOState_FailBit | IOState_BadBit)) != 0; }
	inline bool isBad()     const { return (m_state & IOState_BadBit) == IOState_BadBit;        }
	
	inline IOState getState() const { return m_state; }
	void clear(IOState p_state = IOState_GoodBit);
	void setState(IOState p_state);
	
	inline void useBigEndian()    { m_littleEndian = false; }
	inline void useLittleEndian() { m_littleEndian = true;  }
	
	inline bool usesBigEndian()    const { return m_littleEndian == false; }
	inline bool usesLittleEndian() const { return m_littleEndian; }
	
private:
	BIOStreamBase(const BIOStreamBase&);
	BIOStreamBase& operator=(const BIOStreamBase&);
	
	
	IOState m_state;
	bool    m_littleEndian;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_STREAMS_BIOSTREAMBASE_H)
