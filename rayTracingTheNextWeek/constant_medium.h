#pragma once

#include"constantAndTool.h"
#include"hittable.h"
#include"material.h"
#include"texture.h"

namespace ray_tracing
{
	//The boundary of a volume
	class constant_medium : public hittable
	{
	private:
		shared_ptr<hittable> boundary;
		shared_ptr<material> phase_function;
		double neg_inv_density;
		constant_medium() = default;

	public:
		constant_medium(shared_ptr<hittable> b, double d, shared_ptr<texture> a)
			: boundary(b), neg_inv_density(-1 / d)
		{
			phase_function = make_shared<isotropic>(a);
		}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			return boundary->bounding_box(t0, t1, output_box);
		}
	};
}