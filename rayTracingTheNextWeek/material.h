#pragma once 

#include"constantAndTool.h"
#include "hittable.h"
#include"texture.h"

namespace ray_tracing
{

	class material
	{
	public:
		//scattering 
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
		virtual vec3 emitted(double u, double v, const vec3& p) const
		{
			return vec3(0, 0, 0);
		}
	};
	 
	//diffused reflection material
	class lambertian : public material
	{
	public:
		lambertian( shared_ptr<texture> a ) : albedo(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
		{
			vec3 scatter_direction = rec.normal + random_unit_vector();
			scattered = ray(rec.p, scatter_direction, r_in.get_time());
			attenuation = albedo->value(rec.u, rec.v, rec.p);
			return true;
		}

	private:
		shared_ptr<texture> albedo;
	};


	//metal material
	class metal : public material
	{
	public:
		metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
		{
			vec3 reflected = reflect(unit_vector(r_in.get_direction()), rec.normal);
			scattered = ray(rec.p, reflected + random_in_unit_sphere() * fuzz);
			attenuation = albedo;
			return (dot(scattered.get_direction(), rec.normal) > 0);
		}

	private:
		vec3 albedo;
		double fuzz;
	};


	//dielectric / glass material
	class dielectric : public material
	{
	public:
		dielectric(double ri) : ref_idx(ri) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override;

	private:
		double ref_idx;
	};

	//diffuse light
	class diffus_light : public material
	{
	private:
		shared_ptr<texture> emit;
	public:
		diffus_light(shared_ptr<texture> a) : emit(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
		{
			return false;
		}

		virtual vec3 emitted(double u, double v, const vec3& p) const override
		{
			return emit->value(u, v, p);
		}
	};

	class isotropic : public material
	{
	private:
		shared_ptr<texture> albedo;

	public:
		isotropic(shared_ptr<texture> a) : albedo(a) {}

		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const override
		{
			scattered = ray(rec.p, random_in_unit_sphere(), r_in.get_time());
			attenuation = albedo->value(rec.u, rec.v, rec.p);
			return true;
		}

	};

}
