#if !defined(INC_TT_MATH_PREDICATES_RECTPREDICATES_H)
#define INC_TT_MATH_PREDICATES_RECTPREDICATES_H

//#include <tt/math/math.h>

namespace predicates {

/*! \brief Compare PointRects on their surface. */
class PredicatePointRectSize
{
public:
	bool operator() ( const PointRect & lhs, const PointRect & rhs )
	{
		return (lhs.getWidth() * lhs.getHeight()) > (rhs.getWidth() * rhs.getHeight());
	}
};

/*! \brief Compare VectorRects on their surface. */
class PredicateVectorRectSize
{
public:
	bool operator() ( const VectorRect & lhs, const VectorRect & rhs )
	{
		return (lhs.getWidth() * lhs.getHeight()) > (rhs.getWidth() * rhs.getHeight());
	}
};

/*! \brief Compare PointRects on their position. (top-left based) */
class PredicatePointRectPosition
{
public:
	bool operator() ( const PointRect & lhs, const PointRect & rhs )
	{
		return (lhs.getLeft() + (lhs.getTop() * lhs.getWidth())) > 
			   (rhs.getLeft() + (rhs.getTop() * rhs.getWidth()));
	}
};

/*! \brief Compare VectorRects on their position. (top-left based) */
class PredicateVectorRectPosition
{
public:
	bool operator() ( const VectorRect & lhs, const VectorRect & rhs )
	{
		return (lhs.getLeft() + (lhs.getTop() * lhs.getWidth())) > 
			   (rhs.getLeft() + (rhs.getTop() * rhs.getWidth()));
	}
};

/*! \brief Compare PointRects on their surface size. If equal, the position is compared instead.*/
class PredicatePointRectSizePosition
{
public:
	bool operator() ( const PointRect & lhs, const PointRect & rhs )
	{
		s32 lhsSurface = lhs.getWidth() * lhs.getHeight();
		s32 rhsSurface = rhs.getWidth() * rhs.getHeight();

		if (lhsSurface == rhsSurface)
		{
			return (lhs.getLeft() + (lhs.getTop() * lhs.getWidth())) > 
				   (rhs.getLeft() + (rhs.getTop() * rhs.getWidth()));
		}
		else
		{
			return lhsSurface > rhsSurface;
		}
	}
};

/*! \brief Compare VectorRects on their surface size. If equal, the position is compared instead.*/
class PredicateVectorRectSizePosition
{
public:
	bool operator() ( const VectorRect & lhs, const VectorRect & rhs )
	{
		real lhsSurface = lhs.getWidth() * lhs.getHeight();
		real rhsSurface = rhs.getWidth() * rhs.getHeight();

		if (realEqual(lhsSurface, rhsSurface))
		{
			return (lhs.getLeft() + (lhs.getTop() * lhs.getWidth())) > 
				   (rhs.getLeft() + (rhs.getTop() * rhs.getWidth()));
		}
		else
		{
			return realGreaterThan(lhsSurface, rhsSurface);
		}
	}
};

/*! \brief Compare PointRects on position. If equal, the surface size is compared instead.*/
class PredicatePointRectPositionSize
{
public:
	bool operator() ( const PointRect & lhs, const PointRect & rhs )
	{
		s32 lhsPosition = lhs.getLeft() + (lhs.getTop() * lhs.getWidth());
		s32 rhsPosition = rhs.getLeft() + (rhs.getTop() * rhs.getWidth());

		if (lhsPosition == rhsPosition)
		{
			return (lhs.getWidth() * lhs.getHeight()) > (rhs.getWidth() * rhs.getHeight());
		}
		else
		{
			return lhsPosition > rhsPosition;
		}
	}
};

/*! \brief Compare VectorRects on position. If equal, the surface size is compared instead.*/
class PredicateVectorRectPositionSize
{
public:
	bool operator() ( const VectorRect & lhs, const VectorRect & rhs )
	{
		real lhsPosition = lhs.getLeft() + (lhs.getTop() * lhs.getWidth());
		real rhsPosition = rhs.getLeft() + (rhs.getTop() * rhs.getWidth());

		if (realEqual(lhsPosition, rhsPosition))
		{
			return (lhs.getWidth() * lhs.getHeight()) > (rhs.getWidth() * rhs.getHeight());
		}
		else
		{
			return realGreaterThan(lhsPosition, rhsPosition);
		}
	}
};

// end namespace 
}

#endif // !defined(INC_TT_MATH_PREDICATES_RECTPREDICATES_H)
