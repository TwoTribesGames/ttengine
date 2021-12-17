#if !defined(INC_TT_ENGINE_SCENE2D_BINARYPLANEPARTITION_H)
#define INC_TT_ENGINE_SCENE2D_BINARYPLANEPARTITION_H

#include <vector>

#include <tt/math/Rect.h>


namespace tt {
namespace engine {
namespace scene2d {

class PlaneScene;


class BinaryPlanePartition
{
public:
	typedef std::pair<int, PlaneScene*> Plane;
	typedef std::vector<Plane> Planes;
	
	BinaryPlanePartition();
	~BinaryPlanePartition();
	
	void addPlane(PlaneScene* p_plane);
	
	void partition();
	
	void getVisiblePlanes(const math::VectorRect& p_rect, Planes& p_planesOUT) const;
	
	void clear();
	
#ifndef TT_BUILD_FINAL
	void renderDebug() const;
#endif
	
	void initialUpdate(real p_deltaTime);
	void update(real p_deltaTime);
	
	void render(const math::VectorRect& p_rect) const;
	
private:
	struct Partition
	{
		math::VectorRect rect;
		Partition* left;
		Partition* right;
		
		Planes planes;
		
		inline Partition()
		:
		rect(),
		left(0),
		right(0),
		planes()
		{}
		
		inline ~Partition()
		{
			delete left;
			delete right;
		}
		
		bool balance(int p_maxPlanes);
		
		void getVisiblePlanes(const math::VectorRect& p_rect, Planes& p_planesOUT) const;
		
	#ifndef TT_BUILD_FINAL
		void renderDebug(const renderer::ColorRGB& p_color = renderer::ColorRGB::red) const;
	#endif
		
	private:
		// No copying
		Partition(const Partition&);
		Partition& operator=(const Partition&);
	};
	
	
	// No copying
	BinaryPlanePartition(const BinaryPlanePartition&);
	BinaryPlanePartition& operator=(const BinaryPlanePartition&);
	
	
	Planes                   m_planes;
	Planes                   m_notPartitionedPlanes; // FIXME: Workaround getBoundingRect issue with animations.
	mutable Planes           m_visiblePlanes;
	mutable math::VectorRect m_lastRect;
	Partition*               m_partition;
};

//namespace end
}
}
}

#endif // !defined(INC_TT_ENGINE_SCENE2D_BINARYPLANEPARTITION_H)
