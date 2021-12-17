#include <tt/engine/animation/HermiteFloatController.h>
#include <tt/math/math.h>
#include <tt/fs/File.h>
#include <tt/code/helpers.h>


namespace tt {
namespace engine {
namespace animation {


HermiteFloatController::HermiteFloatController() 
:
m_defaultValue(0.0f),
m_keys()
{
}


HermiteFloatController::~HermiteFloatController()
{
	code::helpers::freeContainer(m_keys);
}


real HermiteFloatController::getValue(real p_time) const
{
	if(m_keys.empty()) return m_defaultValue;

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

		// NOTE: We can make this even more efficient if we know which direction
		// the animation is in and if we cache the last returned keys
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

		if(math::realEqual(m_keys[previous].time, p_time))
		{
			return m_keys[previous].value;
		}
		else if(math::realEqual(m_keys[next].time, p_time))
		{
			if(next < m_keys.size() - 1 && m_keys[next].time == m_keys[next + 1].time)
			{
				// If a key having a different value exists for the same frame
				// return the one after (enables instant jumps in value)
				return m_keys[next + 1].value;
			}
			else
			{
				return m_keys[next].value;
			}
		}
		else
		{
			// Compute the value on the hermite curve based on the key pair
			real t1(p_time - m_keys[previous].time);
			real t2(1.0f / (m_keys[next].time - m_keys[previous].time));

			real v0(m_keys[previous].value);
			real v1(m_keys[next].value);
			
			real s0(m_keys[previous].slope);
			real s1(m_keys[next].slope);

			real t1t1t2(t1 * t1 * t2);
			real t1t1t2t2(t1t1t2 * t2);
			real t1t1t1t2t2(t1 * t1t1t2t2);
			real t1t1t1t2t2t2(t1t1t1t2t2 * t2);

			// Compute current value
			return v0 * (2.0f * t1t1t1t2t2t2 - 3.0f * t1t1t2t2 + 1.0f) + 
				   v1 * (-2.0f * t1t1t1t2t2t2 + 3.0f * t1t1t2t2) +
				   s0 * (t1t1t1t2t2 - 2.0f * t1t1t2 + t1) +
				   s1 * (t1t1t1t2t2 - t1t1t2);
		}
	}
}


void HermiteFloatController::load(const fs::FilePtr& p_file, code::ErrorStatus* p_errStatus)
{
	TT_ERR_CHAIN_VOID("Loading Hermite Float Controller");

	// Skip type info -- Needed when we support other controllers as well
	p_file->seek(sizeof(s32), fs::SeekPos_Cur);

	if (p_file->read(&m_defaultValue, sizeof(m_defaultValue)) != sizeof(m_defaultValue))
	{
		TT_ERR_AND_RETURN("Failed to load default value");
	}

	s32 keyCount(0);
	if (p_file->read(&keyCount, sizeof(keyCount)) != sizeof(keyCount))
	{
		TT_ERR_AND_RETURN("Failed to load key count");
	}

	if(keyCount > 0)
	{
		TT_ERR_ASSERTMSG(static_cast<KeyContainer::size_type>(keyCount) < m_keys.max_size(),
			"Exceeded maximum key capacity. Most likely the data is wrong.");

		// Allocate space for the key frame data
		m_keys.resize(static_cast<KeyContainer::size_type>(keyCount));

		fs::size_type dataSize(static_cast<fs::size_type>(keyCount * sizeof(HermiteFloatKey)));

		// Read all key frame data
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

