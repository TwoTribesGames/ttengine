#include <tt/engine/physics/CollisionModel.h>
#include <tt/engine/physics/CollisionFace.h>
#include <tt/engine/physics/Collision.h>
#include <tt/engine/renderer/MatrixStack.h>
#include <tt/engine/renderer/Face.h>
#include <tt/engine/scene/Model.h>
#include <tt/engine/renderer/SubModel.h>
#include <tt/engine/scene/SceneObject.h>
#include <tt/math/Matrix44.h>


namespace tt {
namespace engine {
namespace physics {


CollisionModel::CollisionModel()
:
m_model(0),
m_collisionFaceCount(0),
m_collidedFace(0),
m_collisionFaces(0)
{
}


CollisionModel::~CollisionModel()
{
	delete[] m_collisionFaces;
}


s32 CollisionModel::countTriangles(const scene::SceneObjectPtr& p_object,
								   s32 p_currentCount) const
{
	using renderer::SubModel;
	using scene::Model;
	using scene::SceneObject;
	
	if (p_object->getType() == SceneObject::Type_Model)
	{
		Model* model = static_cast<Model*>(p_object.get());

		for (s32 i = 0; i < model->getSubModelCount(); ++i)
		{
			SubModel* subModel = model->getSubModel(i);
			p_currentCount += subModel->getTriangleCount();
		}
	}

	// If we have got any children - handle them
	if (p_object->getChild() != 0)
	{
		p_currentCount = countTriangles(p_object->getChild(), p_currentCount);
	}

	if (p_object->getSibling() != 0)
	{
		p_currentCount = countTriangles(p_object->getSibling(), p_currentCount);
	}

	return p_currentCount;
}


void CollisionModel::extractPlane(const scene::SceneObjectPtr& p_object,
                                  s32 p_currentCount)
{
	using renderer::SubModel;
	using scene::Model;
	using scene::SceneObject;
	using renderer::Face;
	using renderer::MatrixStack;
	using math::Matrix44;
	using math::Vector3;

	// Pop the stack
	MatrixStack::getInstance()->push();
	MatrixStack::getInstance()->multiply44(p_object->getDefaultMatrix());

	// Now handle the children and siblings
	if (p_object->getChild() != 0)
	{
		extractPlane(p_object->getChild(), p_currentCount);
	}

	if ( p_object->getType() == SceneObject::Type_Model)
	{
		Model* model = static_cast<Model*>(p_object.get());

		// Translate by the pivot
		MatrixStack::getInstance()->translate(p_object->getPivot());

		// Handle the scale
		MatrixStack::getInstance()->uniformScale(p_object->getDefaultScaleFactor());

		// Get the current matrix
		Matrix44 matrix;
		MatrixStack::getInstance()->getCurrent(matrix);

		// Process all our sub models
		for (s32 i = 0; i < model->getSubModelCount(); ++i)
		{
			SubModel* subModel = model->getSubModel(i);

			for (s32 f = 0; f < subModel->getTriangleCount(); ++f)
			{
				size_t baseIndex = static_cast<size_t>(3 * f);
				Vector3 v0(subModel->getVertexBuffer().getPositions().at(baseIndex));
				Vector3 v1(subModel->getVertexBuffer().getPositions().at(baseIndex + 1));
				Vector3 v2(subModel->getVertexBuffer().getPositions().at(baseIndex + 2));

				// Process these vectors
				v0 = v0 * matrix;
				v1 = v1 * matrix;
				v2 = v2 * matrix;

				// Create the collision face
				m_collisionFaces[p_currentCount].create(v0, v1, v2);
				++p_currentCount;
			}
		}
	}

	// Pop the stack
	MatrixStack::getInstance()->pop();

	// Now handle the children and siblings
	if (p_object->getSibling() != 0)
	{
		extractPlane(p_object->getSibling(), p_currentCount);
	}
}


bool CollisionModel::generate(scene::Model* p_model)
{
	if (m_collisionFaces != 0)
	{
		delete [] m_collisionFaces;
	}

	// Add a ref to the model
	m_model = p_model;

	// Count the number of faces we have in the entire modelset
	// FIXME: SMART POINTERS: m_collisionFaceCount = countTriangles(m_model);
	m_collisionFaces = new CollisionFace[m_collisionFaceCount];

	// We can now extract all the details
	// FIXME: SMART POINTERS: extractPlane(m_model);

	return true;
}


/*! \brief Checks for a collision between a ray and a model
	
	Checks collision and returns true if a collision is detected

	\param p_start - The start position of the ray
	\param p_dir - The direction of the ray (infinite in both directions)
	\param p_collide - The collision point is returned here 
					   if the function returns true
    \return True if a collision is detected or 
			False if the line fails to intersect..
*/
bool CollisionModel::rayIntersect(const math::Vector3& p_start, 
								  const math::Vector3& p_dir, 
								  math::Vector3* p_collide)
{
	// Check if we collide with any of the faces
	Collision collision;

	for (s32 i = 0; i < m_collisionFaceCount; ++i)
	{
		if (m_collisionFaces[i].rayIntersect(p_start, p_dir, &collision))
		{
			// Store point of collision
			*p_collide = collision.getCollisionPoint();
			return true;
		}
	}
	return false;
}


/*! \brief Checks for a collision between a line and a model
	
	Checks collision and returns true if a collision is detected

	\param p_start - The start position of the line
	\param p_dir - The direction (and magnitude) of the line
	\param p_collide - The collision point is returned here 
					   if the function returns true
    \return True if a collision is detected or 
			False if the line fails to intersect..
*/
bool CollisionModel::lineIntersect(const math::Vector3& p_start, 
								   const math::Vector3& p_dir, 
								   math::Vector3* p_collide)
{
	// Check if we collide with any of the faces
	Collision collision;

	for (s32 i = 0; i < m_collisionFaceCount; ++i)
	{
		if (m_collisionFaces[i].lineIntersect(p_start, p_dir, &collision))
		{
			// Store point of collision
			*p_collide = collision.getCollisionPoint();
			return true;
		}
	}
	return false;
}


// Namespace end
}
}
}

