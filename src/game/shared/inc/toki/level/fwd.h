#if !defined(INC_TOKI_LEVEL_FWD_H)
#define INC_TOKI_LEVEL_FWD_H


#include <vector>

#include <tt/platform/tt_types.h>


namespace toki {
namespace level {

class AttributeLayer;
typedef tt_ptr<AttributeLayer>::shared AttributeLayerPtr;

class AttributeLayerSection;
typedef tt_ptr<AttributeLayerSection>::shared AttributeLayerSectionPtr;

class LevelData;
typedef tt_ptr<LevelData>::shared LevelDataPtr;

class LevelDataObserver;
typedef tt_ptr<LevelDataObserver>::shared LevelDataObserverPtr;
typedef tt_ptr<LevelDataObserver>::weak   LevelDataObserverWeakPtr;

class LevelSection;
typedef tt_ptr<LevelSection>::shared LevelSectionPtr;

class Note;
typedef tt_ptr<Note>::shared NotePtr;
typedef std::vector<NotePtr> Notes;

class TileChangedObserver;
typedef tt_ptr<TileChangedObserver>::shared TileChangedObserverPtr;
typedef tt_ptr<TileChangedObserver>::weak   TileChangedObserverWeakPtr;

class TileRegistrationMgr;
typedef tt_ptr<TileRegistrationMgr>::shared TileRegistrationMgrPtr;

// Namespace end
}
}


#endif  // !defined(INC_TOKI_LEVEL_FWD_H)
