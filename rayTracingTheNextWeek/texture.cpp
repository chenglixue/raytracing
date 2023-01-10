#include "texture.h"

namespace ray_tracing
{

	//make the value of permutation table out of order
	void perlin::permute(int* p, int n)
	{
		for (int i = n - 1; i > 0; --i)
		{
			int target = random_int(0, i);
			int tmp = p[i];
			p[i] = p[target];	//Get a random value
			p[target] = tmp;	//Prevent duplicate 
		}
	}

	int* perlin::perlin_generate_perm()
	{
		auto p = new int[point_count];
		for (int i = 0; i < perlin::point_count; ++i)
		{
			p[i] = i;
		}
		permute(p, point_count);
		return p;
	}

	perlin::perlin()
	{
		ran_vec3 = new vec3[point_count];
		for (int i = 0; i < point_count; ++i)
		{
			ran_vec3[i] = unit_vector(vec3::random(-1, 1));
		}

		perm_x = perlin_generate_perm();
		perm_y = perlin_generate_perm();
		perm_z = perlin_generate_perm();
	}

	double perlin::noise(const vec3& p) const
	{
		//decimal part
		auto u = p.x() - floor(p.x());
		auto v = p.y() - floor(p.y());
		auto w = p.z() - floor(p.z());

		//integral part
		int i = floor(p.x());
		int j = floor(p.y());
		int k = floor(p.z());
		vec3 c[2][2][2];

		//xor
		//Record the Hasche transform to get the gradient of the reference point
		//The limiting gradient value is [0,255]
		for (int di = 0; di < 2; ++di)
		{
			for (int dj = 0; dj < 2; ++dj)
			{
				for (int dk = 0; dk < 2; ++dk)
				{
					//hash transform
					//Finds the index of the gradient table by the index values of the permutation table
					//The gradient value is found by the index of the gradient table
					//Here, the operands between Perm have no effect, just the color of the result
					c[di][dj][dk] = ran_vec3[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^ perm_z[(k + dk) & 255]];
				}
			}
		}

		return vec3_trilinear_interp(c, u, v, w);
	}

	double perlin::turb(const vec3& p, int depth) const
	{
		auto accum = 0.0;
		vec3 temp_p = p;
		auto weight = 1.0;

		for (int i = 0; i < depth; ++i)
		{
			accum += weight * noise(temp_p);
			weight *= 0.5;	//amplitude
			temp_p *= 2;	//frequence
		}
		return std::fabs(accum);
	}

	vec3 image_texture::value(double u, double v, const vec3& p) const 
	{
		if (data == nullptr)
		{
			return vec3(0, 1, 1);
		}

		//texture-space locations
		auto i = static_cast<int>(u * nx);
		auto j = static_cast<int>((1 - v) * ny - 0.001);

		if (i < 0)
			i = 0;
		if (j < 0)
			j = 0;
		if (i > nx - 1)
			i = nx - 1;
		if (j > ny - 1)
			j = ny - 1;

		auto r = static_cast<int>(data[3 * i + 3 * nx * j + 0]) / 255.0;
		auto g = static_cast<int>(data[3 * i + 3 * nx * j + 1]) / 255.0;
		auto b = static_cast<int>(data[3 * i + 3 * nx * j + 2]) / 255.0;

		return vec3(r, g, b);
	}
}