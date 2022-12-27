#pragma once 
#ifndef HITTABLE
#define HITTABLE
#include"constantAndTool.h"

namespace ray_tracing
{
	extern class material;
	struct hit_record
	{

		vec3 p;	
		vec3 normal;
		double t;	
		bool front_face;	//法线朝向
		shared_ptr<material>mat_ptr;

		//让法线永远朝外
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
	};

	//球体class
	class sphere : public hittable
	{
	public:
		sphere() = default;
		sphere(vec3 cen, double r, shared_ptr<material>m) : center(cen), radius(r), mat_ptr(m) {}

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const;


	private:
		vec3 center;
		double radius;
		shared_ptr<material>mat_ptr;

	};

	inline bool sphere::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		vec3 oc = r.get_origin() - center;
		auto a = r.get_direction().length_squared();
		auto half_b = dot(r.get_direction(), oc);
		auto c = oc.length_squared() - radius * radius;
		auto delta = half_b * half_b - a * c;
		if (delta > 0)
		{

			auto root = sqrt(delta);

			auto temp = (-half_b - root) / a;	//t时间
			if (temp > t_min && temp < t_max)
			{
				rec.t = temp;
				rec.p = r.at(rec.t);
				vec3 outward_normal = (rec.p - center) / radius;
				rec.set_face_normal(r, outward_normal);
				rec.mat_ptr = mat_ptr;
				return true;
			}

			temp = (-half_b + root) / a;
			if (temp > t_min && temp < t_max)
			{
				rec.t = temp;
				rec.p = r.at(rec.t);
				vec3 outward_normal = (rec.p - center) / radius;
				rec.set_face_normal(r, outward_normal);
				rec.mat_ptr = mat_ptr;
				return true;
			}

		}
		return false;
	}

	//存放物体的列表
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

		virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const;

	private:
		vector<shared_ptr<hittable>> objects;
	};

	inline bool hittable_list::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		hit_record temp_rec;
		bool is_hitted = false;
		auto closet_so_far = t_max;

		for (const auto& object : objects)
		{
			//只渲染最前面的物体颜色，忽略背面
			//光线打中object则更新hit_record
			if (object->hit(r, t_min, closet_so_far, temp_rec))
			{
				is_hitted = true;
				//不用max()，因为t在(t_min,t_max)范围内
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return is_hitted;
	}
}
#endif // !HITTABLE

