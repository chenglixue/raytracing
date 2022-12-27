#include"hittable.h"
#include"material.h"
#include"constantAndTool.h"

namespace ray_tracing
{

	static hittable_list random_scene()
	{
		hittable_list world;

		world.add(make_shared<sphere>(vec3(0, -1000, 0), 1000, make_shared<lambertian>(vec3(0.5, 0.5, 0.5))));

		int i = 1;
		for (int a = -11; a < 11; ++a)
		{
			for (int b = -11; b < 11; ++b)
			{
				auto choose_mat = random_double();
				vec3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());
				if ((center - vec3(4, 0.2, 0)).length() > 0.9)
				{
					if (choose_mat < 0.8)
					{
						//diffuse
						auto albedo = vec3::random() * vec3::random();
						world.add(make_shared<sphere>(center, 0.2, make_shared<lambertian>(albedo)));
					}
					else if (choose_mat < 0.95)
					{
						//metal
						auto albedo = vec3::random(0.5, 1);
						auto fuzz = random_double(0, 0.5);
						world.add(make_shared<sphere>(center, 0.2, make_shared<metal>(albedo, fuzz)));
					}
					else
					{
						//glass
						world.add(make_shared<sphere>(center, 0.2, make_shared<dielectric>(1.5)));
					}
				}
			}
		}

		world.add(make_shared<sphere>(vec3(0, 1, 0), 1.0, make_shared<dielectric>(1.5)));

		world.add(
			make_shared<sphere>(vec3(-4, 1, 0), 1.0, make_shared<lambertian>(vec3(0.4, 0.2, 0.1))));

		world.add(
			make_shared<sphere>(vec3(4, 1, 0), 1.0, make_shared<metal>(vec3(0.7, 0.6, 0.5), 0.0)));

		return world;
	}

	static vec3 ray_color(const ray& r, const hittable& world, int depth)
	{

		hit_record rec;

		if (depth <= 0)
		{
			return vec3(1.0, 0, 0);
		}
		if (world.hit(r, 0.001, infinity, rec) == true)
		{
			ray scattered;
			vec3 attenuation;
			if (rec.mat_ptr->scatter(r, rec, attenuation, scattered) == true)
			{
				return attenuation * ray_color(scattered, world, depth - 1);
			}
			return vec3(0, 0, 0);
		}

		vec3 unit_direction = unit_vector(r.get_direction());
		//以y轴值作渐变
		auto t = (unit_direction.y() + 1.0) * 0.5;
		return vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
	}

	static double hit_sphere(const vec3& center, double radius, const ray& r)
	{
		vec3 oc = r.get_origin() - center;
		auto a = r.get_direction().length_squared();
		auto half_b = dot(r.get_direction(), oc);
		auto c = oc.length_squared() - radius * radius;
		auto delta = half_b * half_b - a * c;

		if (delta < 0)
		{
			return -1.0;
		}
		else
		{
			return (-half_b - sqrt(delta)) / a;
		}
	}

	static void output_image()
	{
		const int image_width = 1200;
		const int image_height = 1200;
		const int samples_per_pixel = 10;
		const int max_depth = 50;

		cout << "P3" << endl << image_width << ' ';
		cout << image_height << endl << "255" << endl;

		//material instance
		/*
		hittable_list world;
		world.add(make_shared<sphere>(vec3(0, 0, -1), 0.5, make_shared<lambertian>(vec3(0.7, 0.3, 0.3))));
		world.add(make_shared<sphere>(vec3(0, -100.5, -1), 100, make_shared<lambertian>(vec3(0.8, 0.8, 0.0))));
		world.add(make_shared<sphere>(vec3(1, 0, -1), 0.5, make_shared<metal>(vec3(0.8, 0.6, 0.2), 0.0)));
		world.add(make_shared<sphere>(vec3(-1, 0, -1), 0.5, make_shared<metal>(vec3(0.8, 0.8, 0.8), 1.0)));
		world.add(make_shared<sphere>(vec3(-1, 0, -1), 0.5, make_shared<dielectric>(1.5)));
		world.add(make_shared<sphere>(vec3(-1, 0, -1), -0.45, make_shared<dielectric>(1.5)));
		*/

		//camera instance
		/*
		auto R = cos(pi / 4);
		const auto aspect_ratio = double(image_width) / image_height;
		vec3 up_vector(0, 1, 0);
		hittable_list world;
		world.add(make_shared<sphere>(vec3(0, 0, -1), 0.5, make_shared<lambertian>(vec3(0.7, 0.3, 0.3))));
		world.add(make_shared<sphere>(vec3(0, -100.5, -1), 100, make_shared<lambertian>(vec3(0.8, 0.8, 0.0))));
		world.add(make_shared<sphere>(vec3(-1, 0, -1), -0.45, make_shared<dielectric>(1.5)));
		world.add(make_shared<sphere>(vec3(1, 0, -1), 0.5, make_shared<metal>(vec3(0.8, 0.6, 0.2), 0.0)));
		world.add(make_shared<sphere>(vec3(-R, 0, -1), R, make_shared<lambertian>(vec3(0, 0, 1))));
		world.add(make_shared<sphere>(vec3(R, 0, -1), R, make_shared<lambertian>(vec3(1, 0, 0))));

		//defocus blur
		vec3 lookfrom(3, 3, 2);
		vec3 lookat(0, 0, -1);
		auto dist_to_focus = (lookfrom - lookat).length();
		auto aperture = 2.0;
		*/

		//end
		auto world = random_scene();
		vec3 lookfrom(13, 2, 3);
		vec3 lookat(0, 0, 0);
		vec3 up_vector(0, 1, 0);
		auto dist_to_focus = 10.0;
		auto aperture = 0.1;
		const auto aspect_ratio = double(image_width) / image_height;

		camera cam(lookfrom, lookat, up_vector, 20, aspect_ratio, aperture, dist_to_focus);
		//camera cam(vec3( -2, 2, 1), vec3(0,0,-1), up_vector, 90, aspect_ratio);

		for (int h = image_height - 1; h >= 0; --h)
		{
			//进度提示
			cerr << endl << "Scanlines remaining: " << h << ' ' << flush << endl;
			for (int w = 0; w < image_width; ++w)
			{
				vec3 color(0, 0, 0);
				for (int s = 0; s < samples_per_pixel; ++s)
				{
					auto u = ((double)w + random_double()) / image_width;
					auto v = ((double)h + random_double()) / image_height;
					ray r = cam.get_ray(u, v);
					color += ray_color(r, world, max_depth);
				}
				color.write_color(cout, samples_per_pixel);
			}
		}
		cerr << endl << "Done." << endl;
	}
}

int main()
{
	ray_tracing::output_image();


	return 0;
}