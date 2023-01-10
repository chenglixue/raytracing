#include "aabb.h"

namespace ray_tracing
{
	aabb surrounding_box(aabb box0, aabb box1)
	{
		vec3 small(ffmin(box0.get_min().x(), box1.get_min().x()),
			ffmin(box0.get_min().y(), box1.get_min().y()),
			ffmin(box0.get_min().z(), box1.get_min().z()));
		vec3 big(ffmax(box0.get_max().x(), box1.get_max().x()),
			ffmax(box0.get_max().y(), box1.get_max().y()),
			ffmax(box0.get_max().z(), box1.get_max().z()));
		return aabb(small, big);
	}

	bool aabb::hit(const ray& r, double tmin, double tmax) const
	{
		for (int i = 0; i < 3; ++i)
		{
			//If the ray emit in the negative direction of the x-axis
			// The library functions consider exceptions, NaN, so slower
			//the following, consider the case where the denominator is 0
			auto t0 = ffmin((m_min[i] - r.get_origin()[i]) / r.get_direction()[i],
				(m_max[i] - r.get_origin()[i]) / r.get_direction()[i]);
			auto t1 = ffmax((m_min[i] - r.get_origin()[i]) / r.get_direction()[i],
				(m_max[i] - r.get_origin()[i]) / r.get_direction()[i]);
			//It is only when the ray enters all the planes that it really enters the aabb
			tmin = ffmax(t0, tmin);
			//It is only when the ray leaves any plane that it really leaves the aabb
			tmax = ffmin(t1, tmax);
			if (tmax <= tmin)
			{
				return false;
			}
		}
		return true;
	}
}