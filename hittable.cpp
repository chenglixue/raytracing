#include "hittable.h"

namespace ray_tracing
{

	bool sphere::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		vec3 oc = r.get_origin() - center;
		auto a = r.get_direction().length_squared();
		auto half_b = dot(r.get_direction(), oc);
		auto c = oc.length_squared() - radius * radius;
		auto delta = half_b * half_b - a * c;
		if (delta > 0)
		{

			auto root = sqrt(delta);

			auto temp = (-half_b - root) / a;	//t
			if (temp > t_min && temp < t_max)
			{
				rec.t = temp;
				rec.p = r.at(rec.t);
				vec3 outward_normal = (rec.p - center) / radius;
				rec.set_face_normal(r, outward_normal);
				rec.mat_ptr = mat_ptr;
				get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
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
				get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
				return true;
			}

		}
		return false;
	}


	bool moving_sphere::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		vec3 oc = r.get_origin() - center(r.get_time());
		auto a = r.get_direction().length_squared();
		auto half_b = dot(r.get_direction(), oc);
		auto c = oc.length_squared() - radius * radius;
		auto delta = half_b * half_b - a * c;

		if (delta > 0)
		{

			auto root = sqrt(delta);

			auto temp = (-half_b - root) / a;
			if (temp > t_min && temp < t_max)
			{
				rec.t = temp;
				rec.p = r.at(rec.t);
				vec3 outward_normal = (rec.p - center(r.get_time())) / radius;
				rec.set_face_normal(r, outward_normal);
				rec.mat_ptr = mat_ptr;
				return true;
			}

			temp = (-half_b + root) / a;
			if (temp > t_min && temp < t_max)
			{
				rec.t = temp;
				rec.p = r.at(rec.t);
				vec3 outward_normal = (rec.p - center(r.get_time())) / radius;
				rec.set_face_normal(r, outward_normal);
				rec.mat_ptr = mat_ptr;
				return true;
			}

		}
		return false;
	}

	bool hittable_list::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		hit_record temp_rec;
		bool is_hitted = false;
		auto closet_so_far = t_max;

		for (const auto& object : objects)
		{
			if (object->hit(r, t_min, closet_so_far, temp_rec))
			{
				is_hitted = true;
				closet_so_far = temp_rec.t;
				rec = temp_rec;
			}
		}

		return is_hitted;
	}

	bool hittable_list::bounding_box(double t0, double t1, aabb& output_box) const
	{
		if (objects.empty() == true)
		{
			return false;
		}

		aabb temp_box;
		bool first_box = true;
		for (const auto& object : objects)
		{
			if (object->bounding_box(t0, t1, temp_box) == false)
			{
				return false;
			}
			output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
			first_box = false;
		}

		return true;
	}


	 bvh_node::bvh_node(const vector<shared_ptr<hittable>>& objects, size_t start, size_t end, double time0, double time1)
	{
		auto temp_objects = objects;
		int axis = random_int(0, 2);
		auto comparator = (axis == 0) ? box_x_compare : (axis == 1) ? box_y_compare : box_z_compare;

		//object number
		size_t object_span = end - start;
		if (object_span == 1)
		{
			left = right = temp_objects[start];
		}
		else if (object_span == 2)
		{
			if (comparator(temp_objects[start], temp_objects[start + 1]) == true)
			{
				left = temp_objects[start];
				right = temp_objects[start + 1];
			}
			else
			{
				left = temp_objects[start + 1];
				right = temp_objects[start];
			}
		}
		else
		{
			std::sort(temp_objects.begin() + start, temp_objects.begin() + end, comparator);
			auto mid = start + object_span / 2;
			left = make_shared<bvh_node>(temp_objects, start, mid, time0, time1);
			right = make_shared<bvh_node>(temp_objects, mid, end, time0, time1);
		}

		aabb box_left, box_right;
		if (left->bounding_box(time0, time1, box_left) == false || right->bounding_box(time0, time1, box_right) == false)
		{
			cerr << "No bounding box in bvh_node constructor." << endl;
		}

		box = surrounding_box(box_left, box_right);

	}


	bool xy_rect::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		auto t = (k - r.get_origin().z()) / r.get_direction().z();
		if (t < t_min || t > t_max)
		{
			return false;
		}

		auto x = r.get_origin().x() + t * r.get_direction().x();
		auto y = r.get_origin().y() + t * r.get_direction().y();
		if (x < x0 || x > x1 || y < y0 || y > y1)
		{
			return false;
		}

		rec.u = (x - x0) / (x1 - x0);
		rec.v = (y - y0) / (y1 - y0);
		rec.t = t;
		vec3 outward_normal = vec3(0, 0, 1);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}


	bool xz_rect::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		auto t = (k - r.get_origin().y()) / r.get_direction().y();
		if (t < t_min || t > t_max)
		{
			return false;
		}
		auto x = r.get_origin().x() + t * r.get_direction().x();
		auto z = r.get_origin().z() + t * r.get_direction().z();
		if (x < x0 || x > x1 || z < z0 || z > z1)
		{
			return false;
		}
		rec.u = (x - x0) / (x1 - x0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		vec3 outward_normal = vec3(0, 1, 0);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}


	bool yz_rect::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		auto t = (k - r.get_origin().x()) / r.get_direction().x();
		if (t < t_min || t > t_max)
		{
			return false;
		}
		auto y = r.get_origin().y() + t * r.get_direction().y();
		auto z = r.get_origin().z() + t * r.get_direction().z();
		if (y < y0 || y > y1 || z < z0 || z > z1)
		{
			return false;
		}
		rec.u = (y - y0) / (y1 - y0);
		rec.v = (z - z0) / (z1 - z0);
		rec.t = t;
		vec3 outward_normal = vec3(1, 0, 0);
		rec.set_face_normal(r, outward_normal);
		rec.mat_ptr = mp;
		rec.p = r.at(t);
		return true;
	}

	//Calculate the coordinates of the six rotating points of the axially aligned cube
	rotate_y::rotate_y(shared_ptr<hittable> p, double angle) : ptr(p)
	{
		auto radians = degrees_to_radians(angle);
		sin_theta = sin(radians);
		cos_theta = cos(radians);
		hasbox = ptr->bounding_box(0, 1, bbox);

		vec3 min(infinity, infinity, infinity);
		vec3 max(-infinity, -infinity, -infinity);

		for (int i = 0; i < 2; ++i)
		{
			for (int j = 0; j < 2; ++j)
			{
				for (int k = 0; k < 2; ++k)
				{
					auto x = i * bbox.get_max().x() + (1 - i) * bbox.get_min().x();
					auto y = j * bbox.get_max().y() + (1 - j) * bbox.get_min().y();
					auto z = k * bbox.get_max().z() + (1 - k) * bbox.get_min().z();

					auto newx = cos_theta * x + sin_theta * z;
					auto newz = -sin_theta * x + cos_theta * z;

					vec3 tester(newx, y, newz);

					for (int c = 0; c < 3; ++c)
					{
						min[c] = ffmin(min[c], tester[c]);
						max[c] = ffmax(max[c], tester[c]);
					}
				}
			}
		}

		bbox = aabb(min, max);
	}

	bool rotate_y::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
	{
		//Rotate the theta angle counterclockwise
		vec3 origin = r.get_origin();
		vec3 direction = r.get_direction();

		origin[0] = cos_theta * r.get_origin()[0] - sin_theta * r.get_origin()[2];
		origin[2] = sin_theta * r.get_origin()[0] + cos_theta * r.get_origin()[2];

		direction[0] = cos_theta * r.get_direction()[0] - sin_theta * r.get_direction()[2];
		direction[2] = sin_theta * r.get_direction()[0] + cos_theta * r.get_direction()[2];

		ray rotated_r(origin, direction, r.get_time());
		if (ptr->hit(rotated_r, t_min, t_max, rec) == false)
		{
			return false;
		}

		vec3 normal = rec.normal;
		vec3 p = rec.p;

		p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
		p[1] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
		normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
		normal[0] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

		rec.p = p;
		rec.set_face_normal(rotated_r, normal);
		return true;
	}


}