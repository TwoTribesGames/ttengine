#include <tt/engine/renderer/VertexBuffer.h>
#include <tt/mem/cache.h>
#include <tt/platform/tt_error.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace renderer {


VertexBuffer::VertexBuffer()
:
m_positions(),
m_normals(),
m_colors(),
m_vertexType(0),
m_vertexSize(0),
m_propertyCount(0)
{
	// Not properly initialized yet (must know vertex type)
}


VertexBuffer::VertexBuffer(u32 p_type)
:
m_positions(),
m_normals(),
m_colors(),
m_vertexType(p_type),
m_vertexSize(0),
m_propertyCount(0)
{
	TT_ASSERT(p_type != 0);
	
	// Compute the size of 1 vertex
	computeVertexSize();
}


VertexBuffer::VertexBuffer(const VertexBuffer& p_rhs)
:
m_positions(p_rhs.m_positions),
m_normals(p_rhs.m_normals),
m_colors(p_rhs.m_colors),
m_vertexType(p_rhs.m_vertexType),
m_vertexSize(p_rhs.m_vertexSize),
m_propertyCount(p_rhs.m_propertyCount)
{
	// Update cache
	if(m_positions.empty() == false) mem::storeAsync(&m_positions[0], static_cast<mem::size_type>(m_positions.size() * sizeof(Position)));
	if(m_normals.empty()   == false) mem::storeAsync(&m_normals[0],   static_cast<mem::size_type>(m_normals.size()   * sizeof(Normal)));
	if(m_colors.empty()    == false) mem::storeAsync(&m_colors[0],    static_cast<mem::size_type>(m_colors.size()    * sizeof(ColorRGBA)));

	// Copy texture coordinates
	for(std::size_t i = 0; i < 4; ++i)
	{
		// Copy texture buffer
		m_texcoords[i] = p_rhs.m_texcoords[i];	

		if(m_texcoords[i].empty() == false)
		{
			mem::storeAsync(&m_texcoords[i][0], static_cast<mem::size_type>(m_texcoords[i].size() * sizeof(TexCoord)));
		}
	}
	mem::sync();
}


void VertexBuffer::setVertexType(u32 p_type)
{
	if(p_type == m_vertexType)
	{
		return;
	}
	TT_ASSERT(p_type != 0);

	m_vertexType = p_type;

	// Compute the size of 1 vertex
	computeVertexSize();
}


bool VertexBuffer::load(const fs::FilePtr& p_file, s32 vertexCount)
{
	// Load positions
	s32 positionCount;
	if (vertexCount == -1)
	{
		if (p_file->read(&positionCount, sizeof(positionCount)) != sizeof(positionCount))
		{
			TT_PANIC("Unable to read position count.");
			return false;
		}
	}
	else
	{
		positionCount = vertexCount;
	}
	
	m_positions.resize(static_cast<PositionBuffer::size_type>(positionCount));
	
	fs::size_type positionSize = static_cast<fs::size_type>(positionCount * sizeof(Position));
	// Read the positions which should consist of 3 floats each
	if (p_file->read(&m_positions[0], positionSize) != positionSize)
	{
		TT_PANIC("Positions read failed.");
		return false;
	}
	mem::flush(&m_positions[0], positionCount * sizeof(Position));

	
	// Load normals
	if(hasProperty(Property_Normal))
	{
		s32 normalCount;
		if (vertexCount == -1)
		{
			if (p_file->read(&normalCount, sizeof(normalCount)) != sizeof(normalCount))
			{
				TT_PANIC("Unable to read normal count.");
				return false;
			}
		}
		else
		{
			normalCount = vertexCount;
		}
		m_normals.resize(static_cast<NormalBuffer::size_type>(normalCount));
		
		fs::size_type normalSize = static_cast<fs::size_type>(normalCount * sizeof(Normal));
		// Read the normals which should consist of 3 floats each
		if(p_file->read(&m_normals[0], normalSize) != normalSize)
		{
			TT_PANIC("Normals read failed.");
			return false;
		}
		mem::flush(&m_normals[0], normalCount * sizeof(Normal));

	}
	
	// Load colors
	if(hasProperty(Property_Diffuse))
	{
		s32 colorCount;
		if (vertexCount == -1)
		{
			if (p_file->read(&colorCount, sizeof(colorCount)) != sizeof(colorCount))
			{
				TT_PANIC("Unable to read color count.");
				return false;
			}
		}
		else
		{
			colorCount = vertexCount;
		}
		m_colors.resize(static_cast<ColorBuffer::size_type>(colorCount));
		
		fs::size_type colorSize = static_cast<fs::size_type>(colorCount * sizeof(ColorRGBA));
		// Read the colors which should be the size of an u32 each
		if(p_file->read(&m_colors[0], colorSize) != colorSize)
		{
			TT_PANIC("Colors read failed.");
			return false;
		}
		mem::flush(&m_colors[0], colorCount * sizeof(ColorRGBA));

	}

	for(std::size_t i = 0; i < 4; ++i)
	{
		// Load tex coordinates
		if(m_vertexType & (Property_Texture0 << i))
		{
			s32 texCoordCount;
			if (vertexCount == -1)
			{
				if (p_file->read(&texCoordCount, sizeof(texCoordCount)) != sizeof(texCoordCount))
				{
					TT_PANIC("Unable to read texcoord count.");
					return false;
				}
			}
			else
			{
				texCoordCount = vertexCount;
			}
			m_texcoords[i].resize(static_cast<TexCoordBuffer::size_type>(texCoordCount));
			
			fs::size_type texCoordSize = static_cast<fs::size_type>(texCoordCount * sizeof(TexCoord));

			// Read the texture coordinates which should consist of 2 floats each
			if(p_file->read(&m_texcoords[i][0], texCoordSize) != texCoordSize)
			{
				TT_PANIC("texcoords read failed.");
				return false;
			}
			mem::flush(&m_texcoords[i][0], texCoordCount * sizeof(TexCoord));
		}
	}
	
	return true;
}


////////////////////////////////
// Private

void VertexBuffer::computeVertexSize()
{
	// All vertices have position
	m_vertexSize = sizeof(Position);
	m_propertyCount = 1;

	// Check for other properties
	if(hasProperty(Property_Normal))
	{
		m_vertexSize += sizeof(Normal);
		++m_propertyCount;
	}
	if(hasProperty(Property_Diffuse))
	{
		m_vertexSize += sizeof(ColorRGBA);
		++m_propertyCount;
	}
	if(hasProperty(Property_Texture0))
	{
		m_vertexSize += sizeof(TexCoord);
		++m_propertyCount;
	}
	if(hasProperty(Property_Texture1))
	{
		m_vertexSize += sizeof(TexCoord);
		++m_propertyCount;
	}
	if(hasProperty(Property_Texture2))
	{
		m_vertexSize += sizeof(TexCoord);
		++m_propertyCount;
	}
	if(hasProperty(Property_Texture3))
	{
		m_vertexSize += sizeof(TexCoord);
		++m_propertyCount;
	}
}


// Namespace end
}
}
}

