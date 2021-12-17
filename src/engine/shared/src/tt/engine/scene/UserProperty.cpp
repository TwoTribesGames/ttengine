#include <vector>

#include <tt/engine/scene/UserProperty.h>
#include <tt/platform/tt_error.h>
#include <tt/platform/tt_printf.h>

#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


UserProperty::UserProperty()
:
m_name(),
m_type(DataType_Integer),
m_integer(0),
m_real(0)
{
}


UserProperty::~UserProperty()
{
	if (m_type == DataType_String && m_string != 0)
	{
		delete [] m_string;
	}
}


bool UserProperty::load(const fs::FilePtr& p_file)
{
	// Read length of name
	s16 len = 0;
	if (p_file->read(&len, sizeof(len)) != sizeof(len))
	{
		return false;
	}

	// Read name
	std::vector<char> name(len);
	if (p_file->read(&name[0], len) != len)
	{
		return false;
	}
	m_name = std::string(name.begin(), name.begin() + len - 1);

	// Now read the type
	s16 type = -1;
	if (p_file->read(&type, sizeof(s16)) != sizeof(s16))
	{
		return false;
	}
	m_type = static_cast<DataType>(type);

	// Now read the correct data
	switch (m_type)
	{
		case DataType_Integer:
		{
			if (p_file->read(&m_integer, sizeof(s32)) != sizeof(s32))
			{
				return false;
			}
			break;
		}

		case DataType_Real:
		{
			if (p_file->read(&m_real, sizeof(real)) != sizeof(real))
			{
				return false;
			}
			break;
		}

		case DataType_String:
		{
			// read the name
			if (p_file->read(&len, sizeof(len)) != sizeof(len))
			{
				return false;
			}

			m_string = new char[len];

			if (p_file->read(m_string, len) != len)
			{
				return false;
			}
			break;
		}

		default:
			TT_PANIC("Unknown datatype!");
	}
	return true;
}


// Namespace end
}
}
}

