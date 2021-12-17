#if !defined(INC_TT_GWEN_GWENMGR_H)
#define INC_TT_GWEN_GWENMGR_H

#include <Gwen/Renderers/TTRenderer.h>


namespace tt {
namespace gwen {


class GwenMgr
{
public:
	static void createInstance();
	static GwenMgr* getInstance();
	static void destroyInstance();
	
	
private:
	Gwen::Renderer::TTRenderer m_gwenRenderer;
	
	static GwenMgr* ms_instance;
	
	GwenMgr();
	inline ~GwenMgr() {}
	
	GwenMgr(const GwenMgr&);                 // Disable copy.
	const GwenMgr operator=(const GwenMgr&); // Disable assigment.
};


// Namespace end
}
}


#endif  // !defined(INC_TT_GWEN_GWENMGR_H)
