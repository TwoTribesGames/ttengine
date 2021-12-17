/*
	GWEN
	Copyright (c) 2010 Facepunch Studios
	See license in Gwen.h
*/

#pragma once
#ifndef GWEN_CONTROLS_RGBCOLORPICKER_H
#define GWEN_CONTROLS_RGBCOLORPICKER_H

#include "Gwen/Controls/Base.h"
#include "Gwen/Gwen.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/ColorControls.h"
#include "Gwen/Controls/ColorPicker.h"


namespace Gwen
{
	namespace Controls
	{
		class GWEN_EXPORT RGBColorPicker : public Controls::Base
		{
			public:
				GWEN_CONTROL( RGBColorPicker, Controls::Base );
				
				inline const Gwen::Color& GetColor() const { return m_color;              }
				inline Gwen::Color GetDefaultColor() const { return m_before->GetColor(); }
				void SetColor(const Gwen::Color& p_color, bool p_onlyHue = false, bool p_reset = false);
				
				Gwen::Event::Caller onColorChanged;
				
			protected:
				virtual void OnBoundsChanged(Gwen::Rect p_oldBounds);
				
			private:
				void ColorBoxChanged   (Gwen::Controls::Base* p_control);
				void ColorSliderChanged(Gwen::Controls::Base* p_control);
				void NumericTyped      (Gwen::Controls::Base* p_control);
				
				void UpdateControls();
				
				
				Gwen::Controls::ColorLerpBox*         m_lerpBox;
				Gwen::Controls::ColorSlider*          m_colorSlider;
				Gwen::ControlsInternal::ColorDisplay* m_before;
				Gwen::ControlsInternal::ColorDisplay* m_after;
				Gwen::Color                           m_color;
				bool                                  m_ignoreCallbacks;
		};
	}
}


#endif
