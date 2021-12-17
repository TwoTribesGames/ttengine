#if !defined(INC_TT_PRES_FWD_H)
#define INC_TT_PRES_FWD_H


#include <set>
#include <vector>

#include <tt/math/hash/NamedHash.h>
#include <tt/platform/tt_types.h>
#include <tt/engine/renderer/fwd.h>
#include <tt/pres/anim2d/fwd.h>


namespace tt {
namespace pres {

typedef anim2d::Tag Tag;
typedef anim2d::Tags Tags;

typedef math::hash::NamedHash<32> Cue;
typedef std::set<Cue> Cues;

struct DependencyData;
typedef std::vector<DependencyData> Dependencies;


class PresentationObject;
typedef tt_ptr<PresentationObject>::shared PresentationObjectPtr;
typedef tt_ptr<const PresentationObject>::shared ConstPresentationObjectPtr;
typedef tt_ptr<PresentationObject>::weak PresentationObjectWeakPtr;

class PresentationMgr;
typedef tt_ptr<PresentationMgr>::shared PresentationMgrPtr;
typedef tt_ptr<PresentationMgr>::weak PresentationMgrWeakPtr;
class PresentationLoader;

class RootMatrixInterface;
typedef tt_ptr<RootMatrixInterface>::shared RootMatrixInterfacePtr;
typedef tt_ptr<RootMatrixInterface>::weak RootMatrixInterfaceWeakPtr;

class GroupInterface;
typedef tt_ptr<GroupInterface>::shared GroupInterfacePtr;
typedef tt_ptr<GroupInterface>::weak GroupInterfaceWeakPtr;

class GroupFactoryInterface;
typedef tt_ptr<GroupFactoryInterface>::shared GroupFactoryInterfacePtr;

class RenderableInterface;
typedef tt_ptr<RenderableInterface>::shared RenderableInterfacePtr;
typedef tt_ptr<RenderableInterface>::weak RenderableInterfaceWeakPtr;

class ParticlesStack;
class ParticleSpawner;
typedef tt_ptr<ParticleSpawner>::shared ParticleSpawnerPtr;
typedef tt_ptr<ParticleSpawner>::weak ParticleSpawnerWeakPtr;


class TriggerStack;
class TriggerInterface;
typedef tt_ptr<TriggerInterface>::shared TriggerInterfacePtr;
typedef tt_ptr<TriggerInterface>::weak   TriggerInterfaceWeakPtr;
class TriggerFactoryInterface;
typedef tt_ptr<TriggerFactoryInterface>::shared TriggerFactoryInterfacePtr;


class FrameAnimation;
typedef tt_ptr<FrameAnimation>::shared FrameAnimationPtr;
class FrameAnimationStack;

class PresentationQuad;
typedef tt_ptr<PresentationQuad>::shared PresentationQuadPtr;

class Timer;
typedef tt_ptr<Timer>::shared TimerPtr;
class TimerStack;

class CueToTag;
typedef tt_ptr<CueToTag>::shared CueToTagPtr;
typedef tt_ptr<const CueToTag>::shared ConstCueToTagPtr;

class CallbackTrigger;
class CallbackTriggerInterface;
typedef tt_ptr<CallbackTriggerInterface>::shared CallbackTriggerInterfacePtr;


//namespace end
}
}

#endif // !defined(INC_TT_PRES_FWD_H)
