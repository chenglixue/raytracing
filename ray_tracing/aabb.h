#pragma once

#include"constantAndTool.h"

namespace ray_tracing
{
	class aabb
	{
	private:
		vec3 m_min;
		vec3 m_max;

	public:
		aabb() = default;
		aabb(const vec3& a, const vec3& b) { m_min = a; m_max = b; }

		vec3 get_min() const { return m_min; }
		vec3 get_max() const { return m_max; }

		bool hit(const ray& r, double tmin, double tmax) const;

	};

	aabb surrounding_box(aabb box0, aabb box1);
}
