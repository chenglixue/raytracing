#include "material.h"

namespace ray_tracing
{


	bool dielectric::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
	{
		//absorptivity
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

}