#if !defined(INC_TT_ENGINE_EFFECT_MODELWRAPPER_H)
#define INC_TT_ENGINE_EFFECT_MODELWRAPPER_H


#include <spark/Core/SPK_Model.h>
#include <spark/Core/SPK_Interpolator.h>
#include <squirrel/sqbind.h>

#include <tt/script/VirtualMachine.h>

namespace tt {
namespace engine {
namespace effect {
namespace wrapper {


class ModelWrapper
{
public:
	ModelWrapper()
	:
	m_particleModel(0)
	{
	}


	~ModelWrapper()
	{
		SPK_Destroy(m_particleModel);
	}


	inline void construct(int p_enableFlag, int p_mutableFlag, int p_randomFlag, int p_interpolatedFlag)
	{
		/*
		// Check input flags (all flags that are mutable, random or interpolated must be enabled)
		TT_ASSERTMSG((p_mutableFlag      & p_enableFlag) == p_mutableFlag,
			"Mutable flag specified that isn't enabled");
		TT_ASSERTMSG((p_randomFlag       & p_enableFlag) == p_randomFlag,
			"Random flag specified that isn't enabled");
		TT_ASSERTMSG((p_interpolatedFlag & p_enableFlag) == p_interpolatedFlag,
			"Interpolated flag specified that isn't enabled");
		*/
		// Removed the assert and auto set the mutiable, random and interpolated flags as enabled.
		
		m_particleModel = SPK::Model::create(p_enableFlag | p_mutableFlag | p_randomFlag | p_interpolatedFlag,
		                                     p_mutableFlag, p_randomFlag, p_interpolatedFlag);

		// Set defaults
		setLifeTime(10,10); // Lifetime = 10 seconds
	}

	inline void setLifeTime(float p_min, float p_max)
	{
		TT_NULL_ASSERT(m_particleModel);
		m_particleModel->setLifeTime(p_min, p_max);
	}


	inline void setImmortal(bool p_immortal)
	{
		TT_NULL_ASSERT(m_particleModel);
		m_particleModel->setImmortal(p_immortal);
	}


	inline void setParam(SPK::ModelParam p_type, float p_value)
	{
		TT_NULL_ASSERT(m_particleModel);
		bool result = m_particleModel->setParam(p_type, p_value);
		TT_ASSERTMSG(result,
		             "Failed to set parameter '%s'! Check your construction flags.",
		             getParamName(p_type));
	}


	inline void setParamRange(SPK::ModelParam p_type, float p_value0, float p_value1)
	{
		TT_NULL_ASSERT(m_particleModel);
		bool result = m_particleModel->setParam(p_type, p_value0, p_value1);
		TT_ASSERTMSG(result,
		             "Failed to set parameter '%s'! Check your construction flags.",
		             getParamName(p_type));
	}


	inline void setParamRandom(SPK::ModelParam p_type,
							  float p_startMin, float p_startMax,
							  float p_endMin,   float p_endMax)
	{
		TT_NULL_ASSERT(m_particleModel);
		bool result = m_particleModel->setParam(p_type, p_startMin, p_startMax, p_endMin, p_endMax);
		TT_ASSERTMSG(result,
		             "Failed to set parameter '%s'! Check your construction flags.",
		             getParamName(p_type));
	}


	inline bool isImmortal()
	{
		TT_NULL_ASSERT(m_particleModel);
		return m_particleModel->isImmortal();
	}


	/*! \brief Standard interpolator implementing the ADSR principle */
	inline void setADSR(SPK::ModelParam p_param, float p_ax, float p_ay,
						float p_dx, float p_dy, float p_sx)
	{
		TT_NULL_ASSERT(m_particleModel);
		TT_ASSERT(p_ax >= 0.0f && p_ax <= 1.0f);
		TT_ASSERT(p_dx >= 0.0f && p_dx <= 1.0f);
		TT_ASSERT(p_sx >= 0.0f && p_sx <= 1.0f);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->clearGraph();
			interpolator->enableLooping(false);
			interpolator->setType(SPK::INTERPOLATOR_LIFETIME);
			interpolator->addEntry(0.0f, 0.0f);
			interpolator->addEntry(p_ax, p_ay);
			interpolator->addEntry(p_dx, p_dy);
			interpolator->addEntry(p_sx, p_dy);
			interpolator->addEntry(1.0f, 0.0f);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void setType(SPK::ModelParam p_param, SPK::InterpolationType p_type,
									 SPK::ModelParam p_ipParam)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->setType(p_type, p_ipParam);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void addEntry(SPK::ModelParam p_param, float p_x, float p_y)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->addEntry(p_x, p_y);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void addEntryRange(SPK::ModelParam p_param, float p_x, float p_minY, float p_maxY)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->addEntry(p_x, p_minY, p_maxY);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void clearGraph(SPK::ModelParam p_param)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->clearGraph();
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void enableLooping(SPK::ModelParam p_param, bool p_loop)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->enableLooping(p_loop);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void setScaleXVariation(SPK::ModelParam p_param, float p_scale)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->setScaleXVariation(p_scale);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	inline void setOffsetXVariation(SPK::ModelParam p_param, float p_offset)
	{
		TT_NULL_ASSERT(m_particleModel);

		SPK::Interpolator* interpolator = m_particleModel->getInterpolator(p_param);
		if(interpolator != 0)
		{
			interpolator->setOffsetXVariation(p_offset);
		}
		else
		{
			TT_PANIC("Interpolator does not exist for parameter '%s',"
				" did you add it to the interpolated flags?", getParamName(p_param));
		}
	}


	static void bind(const tt::script::VirtualMachinePtr& p_vm)
	{
		SqBind<ModelWrapper>::init(p_vm->getVM(), _SC("Model"));
		sqbind_method(p_vm->getVM(), _SC("construct"),      &ModelWrapper::construct);
		sqbind_method(p_vm->getVM(), _SC("setLifeTime"),    &ModelWrapper::setLifeTime);
		sqbind_method(p_vm->getVM(), _SC("isImmortal"),     &ModelWrapper::isImmortal);
		sqbind_method(p_vm->getVM(), _SC("setImmortal"),    &ModelWrapper::setImmortal);
		sqbind_method(p_vm->getVM(), _SC("setParam"),       &ModelWrapper::setParam);
		sqbind_method(p_vm->getVM(), _SC("setParamRange"),  &ModelWrapper::setParamRange);
		sqbind_method(p_vm->getVM(), _SC("setParamRandom"), &ModelWrapper::setParamRandom);
		sqbind_method(p_vm->getVM(), _SC("setADSR"),        &ModelWrapper::setADSR);
		sqbind_method(p_vm->getVM(), _SC("setType"),        &ModelWrapper::setType);
		sqbind_method(p_vm->getVM(), _SC("addEntry"),       &ModelWrapper::addEntry);
		sqbind_method(p_vm->getVM(), _SC("addEntryRange"),  &ModelWrapper::addEntryRange);
		sqbind_method(p_vm->getVM(), _SC("clearGraph"),     &ModelWrapper::clearGraph);
		sqbind_method(p_vm->getVM(), _SC("enableLooping"),  &ModelWrapper::enableLooping);
		sqbind_method(p_vm->getVM(), _SC("setScaleXVariation"),  &ModelWrapper::setScaleXVariation);
		sqbind_method(p_vm->getVM(), _SC("setOffsetXVariation"), &ModelWrapper::setOffsetXVariation);
	}

	inline SPK::Model* getModel() const {return m_particleModel;}
	

private:
	const char* getParamName(SPK::ModelParam p_param)
	{
		using namespace SPK;

		switch(p_param)
		{
		case PARAM_RED:            return "PARAM_RED";
		case PARAM_GREEN:          return "PARAM_GREEN";
		case PARAM_BLUE:           return "PARAM_BLUE";
		case PARAM_ALPHA:          return "PARAM_ALPHA";
		case PARAM_SIZE:           return "PARAM_SIZE";
		case PARAM_MASS:           return "PARAM_MASS";
		case PARAM_ANGLE:          return "PARAM_ANGLE";
		case PARAM_TEXTURE_INDEX:  return "PARAM_TEXTURE_INDEX";
		case PARAM_ROTATION_SPEED: return "PARAM_ROTATION_SPEED";
		case PARAM_CUSTOM_0:       return "PARAM_CUSTOM_0";
		case PARAM_CUSTOM_1:       return "PARAM_CUSTOM_1";
		case PARAM_CUSTOM_2:       return "PARAM_CUSTOM_2";
		default: return "UNKNOWN";
		}
	}

	SPK::Model* m_particleModel;
};

// Namespace end
}
}
}
}


#endif // INC_TT_ENGINE_EFFECT_MODELWRAPPER_H
