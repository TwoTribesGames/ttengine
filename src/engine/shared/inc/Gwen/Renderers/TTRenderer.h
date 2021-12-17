#if !defined(INC_GWEN_RENDERERS_TTRENDERER_H)
#define INC_GWEN_RENDERERS_TTRENDERER_H


#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

#include <tt/engine/renderer/fwd.h>
#include <tt/engine/renderer/ColorRGBA.h>
#include <tt/engine/renderer/QuadBuffer.h>


namespace Gwen {
namespace Renderer {

class GWEN_EXPORT TTRenderer : public Gwen::Renderer::Base
{
	public:
		TTRenderer();
		virtual ~TTRenderer();
		
		virtual void Begin();
		virtual void End();
		
		virtual void SetDrawColor(Gwen::Color color);
		virtual void DrawFilledRect( Gwen::Rect rect );
		
		virtual void LoadFont( Gwen::Font* pFont );
		virtual void FreeFont( Gwen::Font* pFont );
		virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
		using Gwen::Renderer::Base::RenderText;
		virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
		using Gwen::Renderer::Base::MeasureText;
		
		virtual void StartClip();
		virtual void EndClip();
		
		virtual void DrawTexturedRect( Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1=0.0f, float v1=0.0f, float u2=1.0f, float v2=1.0f );
		virtual void LoadTexture( Gwen::Texture* pTexture, bool p_paintable = false );
		virtual void FreeTexture( Gwen::Texture* pTexture );
		virtual Gwen::Color PixelColour( Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default );
		
	private:
		void AddToBatch(const Gwen::Rect& p_rect);
		void Flush();
		
		
		Gwen::Texture* m_fontTexture;
		float m_fontScale[2];  // X [0] and Y [1] scale.
		float m_letterSpacing;
		
		Gwen::Texture* m_currentTexture;
		
		tt::engine::renderer::BatchQuadCollection m_quadCollection;
		tt::engine::renderer::BatchQuad           m_quad;
		tt::engine::renderer::ColorRGBA           m_color;

		// FIXME: Use a persistent vertex buffer that survives the frame
		tt::engine::renderer::QuadBufferPtr m_quadBuffer;
};

// Namespace end
}
}


#endif  // !defined(INC_GWEN_RENDERERS_TTRENDERER_H)
