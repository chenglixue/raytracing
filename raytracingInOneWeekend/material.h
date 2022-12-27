#pragma once 
#ifndef MATERIAL
#define MATERIAL

#include"constantAndTool.h"

namespace ray_tracing
{
	

	extern struct hit_record;
	
	class material
	{
	public:
		//attenuation吸收能量
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
	};

	//漫反射材质
	class lambertian : public material
	{
	public:
		lambertian( const vec3& a) : albedo(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
		{
			vec3 scatter_direction = rec.normal + random_unit_vector();
			scattered = ray(rec.p, scatter_direction);
			attenuation = albedo;
			return true;
		}

	private:
		vec3 albedo;
	};

	//金属
	class metal : public material
	{
	public:
		metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
		{
			vec3 reflected = reflect(unit_vector(r_in.get_direction()), rec.normal);
			scattered = ray(rec.p, reflected + random_in_unit_sphere() * fuzz );
			attenuation = albedo;	//吸收率
			return (dot(scattered.get_direction(), rec.normal) > 0);
		}

	private:
		vec3 albedo;
		double fuzz;
	};

	//绝缘体
	class dielectric : public material
	{
	public:
		dielectric(double ri) : ref_idx(ri) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
		{
			//玻璃表面不吸收能量
			attenuation = vec3(1.0, 1.0, 1.0);
			double etai_over_etat;
			if (rec.front_face == true)
			{
				etai_over_etat = 1.0 / ref_idx;
			}
			else
			{
				etai_over_etat = ref_idx;
			}

			vec3 unit_direction = unit_vector(r_in.get_direction());

			double cos_theta = ffmin(dot(-unit_direction, rec.normal), 1.0);
			double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
			if (etai_over_etat * sin_theta > 1.0)
			{
				vec3 reflected = reflect(unit_direction, rec.normal);
				scattered = ray(rec.p, reflected);
				return true;
			}

			double reflect_prob = schlick(cos_theta, etai_over_etat);
			if (random_double() < reflect_prob)
			{
				vec3 reflected = reflect(unit_direction, rec.normal);
				scattered = ray(rec.p, reflected);
				return true;
			}

			vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
			scattered = ray(rec.p, refracted);
			return true;
		}

	private:
		double ref_idx;
	};
}

#endif // !MATERIAL