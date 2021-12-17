#include <tt/engine/animation/StepFloatController.h>
#include <tt/math/math.h>
#include <tt/fs/File.h>
#include <tt/code/helpers.h>


namespace tt {
namespace engine {
namespace animation {


StepFloatController::StepFloatController()
:
m_keys()
{
}


StepFloatController::~StepFloatController()
{
	code::helpers::freeContainer(m_keys);
}


real StepFloatController::getValue(real p_time) const
{
	if(m_keys.empty() == false)
	{
		// Are we outside the range
		if(m_keys.size() == 1 || p_time <= m_keys.front().time)
		{
			return m_keys.front().value;
		}
		else if(p_time >= m_keys.back().time)
		{
			return m_keys.back().value;
		}
		else
		{
			// get the keys either side of p_time
			KeyContainer::size_type previous(0);
			KeyContainer::size_type next(m_keys.size() - 1);

			while (previous != (next - 1) && previous != next)
			{
				KeyContainer::size_type center = (previous + next) / 2;

				if(p_time <= m_keys[center].time)
				{
					next = center;
				}
				else
				{
					previous = center;
				}
			}

			if(math::realEqual(m_keys[next].time, p_time))
			{
				return m_keys[next].value;
			}
			else
			{
				return m_keys[previous].value;
			}
		}
	}

	return 0.0f;
}


void StepFloatController::load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Step Float Controller");

	s32 keyCount(0);
	if (p_file->read(&keyCount, sizeof(keyCount)) != sizeof(keyCount))
	{
		TT_ERR_AND_RETURN("Failed to load key count");
	}

	if(keyCount > 0)
	{
		TT_ERR_ASSERT(static_cast<KeyContainer::size_type>(keyCount) < m_keys.max_size());

		m_keys.resize(static_cast<KeyContainer::size_type>(keyCount));

		fs::size_type dataSize(static_cast<fs::size_type>(keyCount * sizeof(StepFloatKey)));

		if(p_file->read(&m_keys[0], dataSize) != dataSize)
		{
			TT_ERR_AND_RETURN("Failed to load key frame data");
		}
	}
}


// Namespace end
}
}
}

