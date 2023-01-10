#pragma once

#include "constantAndTool.h"

namespace ray_tracing
{
	/*Texture mapping*/
	class texture
	{
	public:
		virtual vec3 value(double u, double v, const vec3& p) const = 0;
	};

	class constant_texture : public texture
	{
	public:
		constant_texture() = default;
		constant_texture(vec3 c) : color(c) {}
		virtual vec3 value(double u, double v, const vec3& p) const override
		{
			return color;
		}

	private:
		vec3 color;
	};

	//checkerboard texture
	class checker_texture : public texture
	{
	public:
		checker_texture() = default;
		checker_texture(shared_ptr<texture> t0, shared_ptr<texture> t1) : even(t0), odd(t1) {}

		//use the sine and cosine functions to change periodically to create a checkerboard texture
		virtual vec3 value(double u, double v, const vec3& p) const override
		{
			auto sine = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
			if (sine < 0)
			{
				return odd->value(u, v, p);
			}
			else
			{
				return even->value(u, v, p);
			}
		}

	private:
		//The odd and even pointer points to a static texture
		shared_ptr<texture> odd;
		shared_ptr<texture> even;
	};

	//perlin noise
	class perlin
	{
	private:
		static const int point_count = 256;
		double* ranfloat;	//Get a random floating value
		vec3* ran_vec3;	//Get a random vec3
		//Permutation Table
		int* perm_x;
		int* perm_y;
		int* perm_z;

		//make the value of permutation table out of order
		static void permute(int* p, int n);

		//Generate a permutation table
		static int* perlin_generate_perm();
	public:
		perlin();

		~perlin()
		{
			delete[] ran_vec3;
			delete[] perm_x;
			delete[] perm_y;
			delete[] perm_z;
		}

		//Calculate the random value of position P
		double noise(const vec3& p) const;

		//turbulence
		double turb(const vec3& p, int depth = 7)const;

	};

	class noise_texture : public texture
	{
	private:
		perlin noise;
		double scale;
	public:
		noise_texture() = default;
		noise_texture(double sc) : scale(sc) {}
		virtual vec3 value(double u, double v, const vec3& p) const override
		{
			return vec3(1, 1, 1) * (1 + sin(scale * p.z() + 10 * noise.turb(p))) * 0.7;
		}
	};

	//Texture mapping
	class image_texture : public texture
	{
	private:
		unsigned char* data;
		int nx, ny;
	public:
		image_texture() = default;
		image_texture(unsigned char* pixels, int A, int B)
			: data(pixels), nx(A), ny(B) {}
		~image_texture()
		{
			delete data;
		}

		virtual vec3 value(double u, double v, const vec3& p) const override;

	};


	//三线性插值
	inline double trilinear_interp(double c[2][2][2], double u, double v, double w)
	{
		u = 3 * std::pow(u, 2) - 2 * std::pow(u, 3);
		v = 3 * std::pow(v, 2) - 2 * std::pow(v, 3);
		w = 3 * std::pow(w, 2) - 2 * std::pow(w, 3);
		auto lerp_x_y0_z0 = c[0][0][0] + (u - 0) / 1 * (c[1][0][0] - c[0][0][0]);
		auto lerp_x_y1_z0 = c[0][1][0] + u * (c[1][1][0] - c[0][1][0]);
		auto lerp_x_y0_z1 = c[0][0][1] + u * (c[1][0][1] - c[0][0][1]);
		auto lerp_x_y1_z1 = c[0][1][1] + u * (c[1][1][1] - c[0][1][1]);
		auto lerp_y_z0 = lerp_x_y0_z0 + v * (lerp_x_y1_z0 - lerp_x_y0_z0);
		auto lerp_y_z1 = lerp_x_y0_z1 + v * (lerp_x_y1_z1 - lerp_x_y0_z1);
		auto lerp_z = lerp_y_z0 + w * (lerp_y_z1 - lerp_y_z0);
		return lerp_z;
	}

	//vec3的三线性插值
	inline double vec3_trilinear_interp(vec3 c[2][2][2], double u, double v, double w)
	{
		auto uu = 3 * std::pow(u, 2) - 2 * std::pow(u, 3);
		auto vv = 3 * std::pow(v, 2) - 2 * std::pow(v, 3);
		auto ww = 3 * std::pow(w, 2) - 2 * std::pow(w, 3);

		auto lerp_x_y0_z0 = dot(vec3(u, v, w), c[0][0][0]) + (dot(vec3(u - 1, v, w), c[1][0][0]) - dot(vec3(u, v, w), c[0][0][0])) * uu;
		auto lerp_x_y1_z0 = dot(vec3(u, v - 1, w), c[0][1][0]) + (dot(vec3(u - 1, v - 1, w), c[1][1][0]) - dot(vec3(u, v - 1, w), c[0][1][0])) * uu;
		auto lerp_x_y0_z1 = dot(vec3(u, v, w - 1), c[0][0][1]) + (dot(vec3(u - 1, v, w - 1), c[1][0][1]) - dot(vec3(u, v, w - 1), c[0][0][1])) * uu;
		auto lerp_x_y1_z1 = dot(vec3(u, v - 1, w - 1), c[0][1][1]) + (dot(vec3(u - 1, v - 1, w - 1), c[1][1][1]) - dot(vec3(u, v - 1, w - 1), c[0][1][1])) * uu;
		auto lerp_y_z0 = lerp_x_y0_z0 + (lerp_x_y1_z0 - lerp_x_y0_z0) * vv;
		auto lerp_y_z1 = lerp_x_y0_z1 + (lerp_x_y1_z1 - lerp_x_y0_z1) * vv;
		auto lerp_z = lerp_y_z0 + (lerp_y_z1 - lerp_y_z0) * ww;

		return lerp_z;
	}
}