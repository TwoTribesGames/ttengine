#include <Gwen/Anim.h>
#include <Gwen/Utility.h>

#include <tt/gwen/BusyBar.h>


namespace tt {
namespace gwen {

class BusyBarThink : public Gwen::Anim::Animation
{
public:
	inline BusyBarThink()
	:
	m_lastFrame(0.0f)
	{ }
	
	virtual void Think()
	{
		const float diff = Gwen::Platform::GetTimeInSeconds() - m_lastFrame;
		gwen_cast<BusyBar>(m_Control)->CycleThink(Gwen::Clamp(diff, 0.0f, 0.3f));
		m_lastFrame = Gwen::Platform::GetTimeInSeconds();
	}
	
private:
	float m_lastFrame;
};


//--------------------------------------------------------------------------------------------------
// Public member functions

GWEN_CONTROL_CONSTRUCTOR(BusyBar)
,
m_ticker(0),
m_tickerWidth(40),
m_tickerPixelPos(0.0),
m_cyclesPerSecond(0.5f)
{
	SetBounds(Gwen::Rect(0, 0, 128, 32));
	
	m_ticker = new Gwen::Controls::ProgressBar(this);
	m_ticker->SetAutoLabel(false);
	m_ticker->SetValue(1.0f);
	m_ticker->SetBounds(-m_tickerWidth, 0, m_tickerWidth, Height());
	m_tickerPixelPos = static_cast<double>(-m_tickerWidth);
	
	Gwen::Anim::Add(this, new BusyBarThink);
}


void BusyBar::Layout(Gwen::Skin::Base* /*p_skin*/)
{
	m_ticker->SetSize(m_tickerWidth, Height());
}


void BusyBar::Render(Gwen::Skin::Base* p_skin)
{
	// Render ourselves to look like a progress bar background
	p_skin->DrawProgressBar(this, true, 0.0f);
}


void BusyBar::SetTickerWidth(int p_width)
{
	if (p_width > 0)
	{
		m_tickerWidth = p_width;
		m_ticker->SetWidth(m_tickerWidth);
	}
}


void BusyBar::SetCycleSpeed(float p_cyclesPerSecond)
{
	m_cyclesPerSecond = p_cyclesPerSecond;
}


void BusyBar::CycleThink(float p_elapsedSeconds)
{
	if (Visible() == false) return;
	
	// Move the ticker along the bar
	m_tickerPixelPos += m_cyclesPerSecond * p_elapsedSeconds * Width();
	if (m_tickerPixelPos > static_cast<double>(Width()))
	{
		// Ticker exited the right side of the bar: restart to the left of the bar
		m_tickerPixelPos -= static_cast<double>(Width() + m_tickerWidth);
	}
	m_ticker->SetPos(static_cast<int>(m_tickerPixelPos), 0);
}

// Namespace end
}
}
