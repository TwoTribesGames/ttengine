#if !defined(INC_TT_GWEN_BUSYBAR_H)
#define INC_TT_GWEN_BUSYBAR_H


#include <Gwen/Controls/ProgressBar.h>


namespace tt {
namespace gwen {

class BusyBar : public Gwen::Controls::Base
{
public:
	GWEN_CONTROL(BusyBar, Base);
	
	virtual void Layout(Gwen::Skin::Base* p_skin);
	virtual void Render(Gwen::Skin::Base* p_skin);
	
	inline int GetTickerWidth() const { return m_tickerWidth; }
	void SetTickerWidth(int p_width);
	
	inline float GetCycleSpeed() const { return m_cyclesPerSecond; }
	void  SetCycleSpeed(float p_cyclesPerSecond);
	
	void CycleThink(float p_elapsedSeconds);
	
private:
	Gwen::Controls::ProgressBar* m_ticker;
	int                          m_tickerWidth;
	double                       m_tickerPixelPos;
	float                        m_cyclesPerSecond;
};

// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_BUSYBAR_H)
