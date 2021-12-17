#include <tt/engine/scene/Bone.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/math/Vector3.h>
#include <tt/fs/File.h>


namespace tt {
namespace engine {
namespace scene {


Bone::Bone()
:
m_name(),
m_sceneObject(),
m_defaultMatrix(),
m_worldMatrix(),
m_weightCount(0),
m_boneWeights(0)
{
}


Bone::~Bone()
{
	if (m_boneWeights != 0)
	{
		delete [] m_boneWeights;
		m_boneWeights = 0;
	}
}


bool Bone::load(const fs::FilePtr& p_file)
{
	// read the names
	s16 len = 0;
	if ( p_file->read(&len, sizeof(len)) != sizeof(len))
	{
		return false;
	}

	std::vector<char> name(len);
	if (p_file->read(&name[0], len) != len )
	{
		return false;
	}
	// Read string from vector, skip the trailing \0
	m_name = std::string(name.begin(), name.begin() + len - 1);

	if (m_defaultMatrix.load(p_file) == false)
	{
		return false;
	}

	// Now we can load our bone weight information
	if (p_file->read(&m_weightCount, sizeof(m_weightCount)) != sizeof(m_weightCount))
	{
		return false;
	}

	m_boneWeights = new BoneWeight[m_weightCount];

	for (s32 i = 0; i < m_weightCount; ++i)
	{
		s16 index;
		real weight;

		if (p_file->read(&index, sizeof(index)) != sizeof(index))
		{
			return false;
		}

		if (p_file->read(&weight, sizeof(weight)) != sizeof(weight))
		{
			return false;
		}

		m_boneWeights[i].setIndex(index);
		m_boneWeights[i].setWeight(weight);
	}

	// Now we can load our information
	EngineID boneID(0,0);
	if(boneID.load(p_file) == false)
	{
		return false;
	}

	// Find the object
	m_sceneObject = SceneObjectCache::find(boneID);

	return true;
}


bool Bone::update()
{
	using renderer::MatrixStack;

	// Push the matrix
	MatrixStack::getInstance()->push();

	// Add our matrix on
	if (m_sceneObject != 0)
	{
		math::Matrix44 world = m_sceneObject->getWorldMatrix();
		MatrixStack::getInstance()->multiply44(world);

		// TODO: Is this still right?
		real scale = 4096.0f / m_sceneObject->getDefaultScaleFactor();

		MatrixStack::getInstance()->uniformScale(scale);

		MatrixStack::getInstance()->multiply44(m_defaultMatrix);

		MatrixStack::getInstance()->getCurrent(m_worldMatrix);
	}
	else
	{
		m_worldMatrix = m_defaultMatrix;
	}

	// Now pop back
	MatrixStack::getInstance()->pop();

	return true;
}


// Namespace end
}
}
}

