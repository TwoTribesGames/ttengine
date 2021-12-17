#if !defined(INC_TT_ENGINE_EFFECT_SPARKRENDERER_H)
#define INC_TT_ENGINE_EFFECT_SPARKRENDERER_H


namespace tt {
namespace engine {
namespace effect {

class SparkRenderer
{
public:
	static void initialize() {}
	static void shutdown() {}

	// Windows only (Must be called from Client!)
	static void onLostDevice() {}
	static void onResetDevice() {}
};

}
}
}

#endif // INC_TT_ENGINE_EFFECT_SPARKRENDERER_H
