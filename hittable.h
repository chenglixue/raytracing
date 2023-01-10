#pragma once 

#include"aabb.h"
#include<algorithm>
#include"constantAndTool.h"
#include"texture.h"

namespace ray_tracing
{
	class material;
	struct hit_record
	{

		vec3 p;	
		vec3 normal;
		double t;	
		double u;
		double v;
		bool front_face;	
		shared_ptr<material>mat_ptr;

		inline void set_face_normal(const ray& r, const vec3& outward_normal)
		{
			front_face = dot(r.get_direction(), outward_normal) < 0;
			normal = front_face == true ? outward_normal : -outward_normal;
		}

	};

	class hittable
	{
	public:
		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const = 0;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const = 0;
	};

	class sphere : public hittable
	{
	public:
		sphere() = default;
		sphere(vec3 cen, double r, shared_ptr<material>m) : center(cen), radius(r), mat_ptr(m) {}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
			return true;
		}

	private:
		vec3 center;
		double radius;
		shared_ptr<material>mat_ptr;

	};

	class moving_sphere : public hittable
	{
	public:

		moving_sphere() = default;
		moving_sphere(vec3 cen0, vec3 cen1, double t0, double t1, double r, shared_ptr<material> m_ptr)
			: center0(cen0), center1(cen1), time0(t0), time1(t1), radius(r), mat_ptr(m_ptr)
		{}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			aabb box0(center(t0) - vec3(radius, radius, radius), center(t0) + vec3(radius, radius, radius));
			aabb box1(center(t1) - vec3(radius, radius, radius), center(t1) + vec3(radius, radius, radius));
			output_box = surrounding_box(box0, box1);
			return true;
		}
		vec3 center(double time) const
		{
			return center0 + ((center1 - center0) * (time - time0) / (time1 - time0));
		};

	private:
		vec3 center0, center1;
		double time0, time1;
		double radius;
		shared_ptr<material>mat_ptr;
	};


	//store hittable
	class hittable_list : public hittable
	{
	public:
		hittable_list() = default;
		hittable_list(shared_ptr<hittable> object)
		{
			add(object);
		}

		void clear()
		{
			objects.clear();
		}

		void add(shared_ptr<hittable> object)
		{
			objects.push_back(object);
		}

		auto get_objects() { return objects; }

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override;

	private:
		vector<shared_ptr<hittable>> objects;
	};


	class bvh_node : public hittable
	{
	private:
		shared_ptr<hittable>left;
		shared_ptr<hittable>right;
		aabb box;

	public:
		bvh_node() = default;
		bvh_node(hittable_list& list, double time0, double time1)
			: bvh_node(list.get_objects(), 0, list.get_objects().size(), time0, time1)
		{}
		bvh_node(const vector<shared_ptr<hittable>>& objects, size_t start, size_t end, double time0, double time1);

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override
		{
			if (box.hit(r, t_min, t_max) == false)
			{
				return false;
			}

			bool hit_left = left->hit(r, t_min, t_max, rec);
			bool hit_right = right->hit(r, t_min, hit_left == true ? rec.t : t_max, rec);

			return hit_left || hit_right;
		}

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const
		{
			output_box = box;
			return true;
		}
	};


	//xy
	class xy_rect : public hittable
	{
	private:
		double x0, x1, y0, y1, k;
		shared_ptr<material> mp;

	public:
		xy_rect() = default;
		xy_rect( double _x0, double _x1, double _y0, double _y1, double _k, shared_ptr<material> mat)
			: x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = aabb(vec3(x0, y0, k - 0.0001), vec3(x1, y1, k + 0.0001));
			return true;
		}

	};

	//xz
	class xz_rect : public hittable
	{
	private:
		double x0, x1, z0, z1, k;
		shared_ptr<material>mp;

	public:
		xz_rect() = default;
		xz_rect(double _x0, double _x1, double _z0, double _z1, double _k, shared_ptr<material> mat)
			: x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {}
		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = aabb(vec3(x0, k - 0.0001, z0), vec3(x1, k + 0.0001, z1));
			return true;
		}
	};

	//yz
	class yz_rect : public hittable
	{
	private:
		double y0, y1, z0, z1, k;
		shared_ptr<material>mp;
	public:
		yz_rect() = default;
		yz_rect(double _y0, double _y1, double _z0, double _z1, double _k, shared_ptr<material> mat)
			: y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = aabb(vec3(k - 0.0001, y0, z0), vec3(k + 0.0001, y1, z1));
			return true;
		}
	};


	//Axis-aligned cuboid
	class box : public hittable
	{
	private:
		vec3 box_min;
		vec3 box_max;
		hittable_list sides;

	public:
		box() = default;
		box(const vec3& p0, const vec3& p1, shared_ptr<material> ptr);

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override
		{
			return sides.hit(r, t_min, t_max, rec);
		}

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = aabb(box_min, box_max);
			return true;
		}
	};

	inline box::box(const vec3& p0, const vec3& p1, shared_ptr<material> ptr)
	{
		box_min = p0;
		box_max = p1;

		sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr));
		sides.add(make_shared<xy_rect>(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));

		sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr));
		sides.add(make_shared<xz_rect>(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));

		sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr));
		sides.add(make_shared<yz_rect>(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));

	}


	class translate : public hittable
	{
	private:
		shared_ptr<hittable>ptr;
		vec3 offset;

	public:
		translate(shared_ptr<hittable> p, const vec3& displacement) : ptr(p), offset(displacement) {}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;

		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override;
	};

	inline bool translate::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		ray moved_r(r.get_origin() - offset, r.get_direction(), r.get_time());
		if (ptr->hit(moved_r, t_min, t_max, rec) == false)
		{
			return false;
		}

		rec.p += offset;
		rec.set_face_normal(moved_r, rec.normal);

		return true;
	}

	inline bool translate::bounding_box(double t0, double t1, aabb& output_box) const
	{
		if (ptr->bounding_box(t0, t1, output_box) == false)
		{
			return false;
		}

		output_box = aabb(output_box.get_min() + offset, output_box.get_max() + offset);

		return true;
	}


	class rotate_y : public hittable
	{
	private:
		shared_ptr<hittable> ptr;
		double sin_theta;
		double cos_theta;
		bool hasbox;
		aabb bbox;

	public:
		rotate_y(shared_ptr<hittable> p, double angle);

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const override;
		virtual bool bounding_box(double t0, double t1, aabb& output_box) const override
		{
			output_box = bbox;
			return hasbox;
		}
	};


	//the compare function of std::sort
	inline bool box_compare(const shared_ptr<hittable> a, const shared_ptr<hittable>b, int axis)
	{
		aabb box_a;
		aabb box_b;

		if (a->bounding_box(0, 0, box_a) == false || b->bounding_box(0, 0, box_b) == false)
		{
			cerr << "No bounding box in bvh_node constructor." << endl;
		}

		return box_a.get_min().e[axis] < box_b.get_min().e[axis];
	}

	inline bool box_x_compare(const shared_ptr<hittable>a, const shared_ptr<hittable> b)
	{
		return box_compare(a, b, 0);
	}

	inline bool box_y_compare(const shared_ptr<hittable>a, const shared_ptr<hittable> b)
	{
		return box_compare(a, b, 1);
	}

	inline bool box_z_compare(const shared_ptr<hittable>a, const shared_ptr<hittable> b)
	{
		return box_compare(a, b, 2);
	}
	//The UV coordinates of the sphere
	inline void get_sphere_uv(const vec3& p, double& u, double& v)
	{
		auto phi = std::atan2(p.z(), p.x());
		auto theta = std::asin(p.y());
		u = 1 - (phi + pi) / (2 * pi);
		v = (theta + pi / 2) / pi;
	}
}

