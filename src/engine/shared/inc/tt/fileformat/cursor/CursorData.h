#if !defined(INC_TT_FILEFORMAT_CURSOR_CURSORDATA_H)
#define INC_TT_FILEFORMAT_CURSOR_CURSORDATA_H


#include <tt/code/fwd.h>
#include <tt/math/Point2.h>


namespace tt {
namespace fileformat {
namespace cursor {

/*! \brief Cursor data for one cursor in a .cur file. */
class CursorData
{
public:
	struct HeaderInfo
	{
		s32          width;
		s32          height;
		s32          colorCount;
		math::Point2 hotSpot;
		
		inline HeaderInfo()
		:
		width(0),
		height(0),
		colorCount(0),
		hotSpot(math::Point2::zero)
		{ }
	};
	
	
	static CursorData* createFromData(const HeaderInfo& p_header, code::BufferReadContext* p_imageDataContext);
	~CursorData();
	
	CursorData* clone() const;
	
	inline s32 getWidth()  const { return m_width;  }
	inline s32 getHeight() const { return m_height; }
	
	/*! \return The position of the hot spot in the cursor in pixels, from the top left of the cursor.
	    \note The hot spot is the point in the cursor that interacts with other elements on the screen. */
	inline const math::Point2& getHotSpot() const { return m_hotSpot; }
	
	/*! \return Cursor image data in 32-bit RGBA format. */
	inline const u8* getImageData() const { return m_imageData; }
	
private:
	explicit CursorData(const HeaderInfo& p_header);
	CursorData(const CursorData& p_rhs);
	CursorData& operator=(const CursorData&);  // disable assignment
	
	
	s32          m_width;
	s32          m_height;
	math::Point2 m_hotSpot;
	u8*          m_imageData;
};

// Namespace end
}
}
}


#endif  // !defined(INC_TT_FILEFORMAT_CURSOR_CURSORDATA_H)
