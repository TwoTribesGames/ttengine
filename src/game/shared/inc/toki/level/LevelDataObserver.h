#if !defined(INC_TOKI_LEVEL_LEVELDATAOBSERVER_H)
#define INC_TOKI_LEVEL_LEVELDATAOBSERVER_H


namespace toki {
namespace level {

/*! \brief Receives notifications about level data changes. */
class LevelDataObserver
{
public:
	LevelDataObserver() { }
	virtual ~LevelDataObserver() { }
	
	virtual void onLevelDataEntitySelectionChanged() { }
};

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_LEVELDATAOBSERVER_H)
