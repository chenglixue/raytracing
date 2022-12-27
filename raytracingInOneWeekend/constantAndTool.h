#pragma once
#ifndef TOOL
#define TOOL


#include <memory>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <vector>
#include <iostream>

using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::flush;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

namespace ray_tracing
{
	const double infinity = std::numeric_limits<double>::infinity();
	const double pi = 3.1415926535897932385;

	inline double degrees_to_radians(double degrees)
	{
		return degrees * pi / 180;
	}

	inline double ffmin(double a, double b)
	{
		return a <= b ? a : b;
	}

	inline double ffmax(double a, double b)
	{
		return a >= b ? a : b;
	}

	inline double random_double()
	{
		return rand() / (RAND_MAX + 1.0);
	}

	inline double random_double(double min, double max)
	{
		return min + (max - min) * random_double();
	}

	inline double clamp(double x, const double min, const double max)
	{
		if (x < min) return min;
		if (x > max) return max;
		return x;
	}

	/*三维vector class*/
	class vec3
	{
	public:
		//表示vector
		//其中double和float都行
		double e[3];

		//构造函数
		vec3() : e{ 0,0,0 } {}
		vec3(double e0, double e1, double e2) : e{ e0,e1,e2 } {}

		//一些重载运算符
		vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
		double operator[](int i) const { return e[i]; }
		double& operator[](int i) { return e[i]; }
		vec3& operator+=(const vec3& v)
		{
			e[0] += v.e[0];
			e[1] += v.e[1];
			e[2] += v.e[2];

			return *this;
		}

		vec3& operator-=(const vec3& v)
		{
			e[0] -= v.e[0];
			e[1] -= v.e[1];
			e[2] -= v.e[2];

			return *this;
		}

		vec3& operator*=(const double t)
		{
			e[0] *= t;
			e[1] *= t;
			e[2] *= t;

			return *this;
		}

		vec3& operator/=(const double t)
		{
			return *this *= 1 / t;
		}

		//一些成员函数
		//返回vector的x/y/z
		double x() const { return e[0]; }
		double y() const { return e[1]; }
		double z() const { return e[2]; }

		//返回vector的长度的平方
		double length_squared() const
		{
			return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
		}

		//返回vector的长度
		double length() const
		{
			return sqrt(length_squared());
		}

		//写入转换后的颜色值
		void write_color(std::ostream& out, int sample_per_pixel)
		{
			//在这之前把所有颜色值加起来，在这一步除以采样数量
			auto scale = 1.0 / sample_per_pixel;

			auto r = sqrt(scale * e[0]);
			auto g = sqrt(scale * e[1]);
			auto b = sqrt(scale * e[2]);


			out << static_cast<int>(255 * clamp(r, 0.0, 0.999)) << ' '
				<< static_cast<int>(255 * clamp(g, 0.0, 0.999)) << ' '
				<< static_cast<int>(255 * clamp(b, 0.0, 0.999)) << endl;
		}

		inline static vec3 random()
		{
			return vec3(random_double(), random_double(), random_double());
		}

		inline static vec3 random(double min, double max)
		{
			return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
		}
	};

	//vec3的工具函数
	inline std::ostream& operator<<(std::ostream& out, const vec3& v)
	{
		return out << v.e[0] << ' '
			<< v.e[1] << ' '
			<< v.e[2];
	}

	inline vec3 operator+(const vec3& u, const vec3& v)
	{
		return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
	}

	inline vec3 operator-(const vec3& u, const vec3& v)
	{
		return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
	}

	inline vec3 operator*(const vec3& u, const vec3& v)
	{
		return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
	}

	inline vec3 operator*(const vec3& v, const double t)
	{
		return vec3(v.e[0] * t, v.e[1] * t, v.e[2] * t);
	}

	inline vec3 operator/(const vec3& v, const double t)
	{
		return v * (1 / t);
	}

	//点乘
	inline double dot(const vec3& u, const vec3& v)
	{
		return u.e[0] * v.e[0] +
			u.e[1] * v.e[1] +
			u.e[2] * v.e[2];
	}

	//叉乘
	inline vec3 cross(const vec3& u, const vec3& v)
	{
		return vec3
		(
			u.e[1] * v.e[2] - u.e[2] * v.e[1],
			u.e[2] * v.e[0] - u.e[0] * v.e[2],
			u.e[0] * v.e[1] - u.e[1] * v.e[0]
		);
	}

	//vec3的规范化，使其长度为1
	inline vec3 unit_vector(vec3 v)
	{
		return v / v.length();
	}

	inline vec3 random_in_unit_sphere()
	{
		while (true)
		{
			auto p = vec3::random(-1, 1);
			//生成的点是否在球内
			if (p.length_squared() >= 1)
			{
				continue;
			}
			return p;
		}
	}

	inline vec3 random_unit_vector()
	{
		//三维极坐标
		auto z = random_double(-1, 1);
		auto r = sqrt(1 - z * z);
		auto theta = random_double(0, 2 * pi);	//水平角
		return vec3(r * cos(theta), r * sin(theta), z);
	}
	inline vec3 random_in_hemisphere(const vec3& normal)
	{
		vec3 in_unit_sphere = random_in_unit_sphere();
		if (dot(normal, in_unit_sphere) > 0.0)
		{
			return in_unit_sphere;
		}
		else
		{
			return -in_unit_sphere;
		}
	}

	//反射
	inline vec3 reflect(const vec3& v, const vec3& n)
	{
		return v - n * 2 * dot(v, n);
	}

	//折射
	inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
	{
		auto cos_theta = dot(-uv, n);
		vec3 r_out_parallel = (uv + n * cos_theta) * etai_over_etat;
		vec3 r_out_perp =  n * ( -sqrt(1 - r_out_parallel.length_squared()));
		return r_out_parallel + r_out_perp;
	}

	//折射概率随入射角改变
	inline double schlick(double cosine, double ref_idx)
	{
		auto r0 = (1 - ref_idx) / (1 + ref_idx);
		r0 *= r0;
		return r0 + (1 - r0) * pow((1 - cosine), 5);
	}

	//从一个单位圆发射出光线
	inline vec3 random_in_unit_disk()
	{
		while (true)
		{
			auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
			if (p.length_squared() >= 1) continue;
			return p;
		}
	}

	/*ray class*/
	class ray
	{
	public:
		ray() {}
		ray(const vec3& origin, const vec3& direction)
			: orig(origin), dir(direction)
		{}

		vec3 get_origin() const { return orig; }
		vec3 get_direction() const { return dir; }
		vec3 at(double t) const
		{
			return orig + dir * t;
		}

	private:
		vec3 orig;
		vec3 dir;
	};

	/*camera class*/
	class camera
	{
	public:
		camera()
		{
			lower_left_corner = vec3(-2.0, -1.0, -1.0);	//渲染矩形界面的左下角坐标
			horizontal = vec3(4.0, 0.0, 0.0);
			vertical = vec3(0.0, 2.0, 0.0);
			origin = vec3(0.0, 0.0, 0.0);
		}

		camera(double boc, double aspect)
		{

			origin = vec3(0.0, 0.0, 0.0);
			auto theta = degrees_to_radians(boc);
			auto half_height = tan(theta / 2);
			auto half_width = aspect * half_height;

			lower_left_corner = vec3(-half_width, -half_height, -1.0);
			horizontal = vec3(2 * half_width, 0.0, 0.0);
			vertical = vec3(0.0, 2 * half_height, 0.0);

		}

		camera(vec3 lookfrom, vec3 lookat, vec3 up_vector, double boc, double aspect)
		{
			origin = lookfrom;
			vec3 u, v, w;
			
			auto theta = degrees_to_radians(boc);
			auto half_height = tan(theta / 2);
			auto half_width = aspect * half_height;
			w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(up_vector, w));
			v = cross(w, u);

			lower_left_corner = origin - u * half_width - v * half_width - w;
			horizontal = u * 2 * half_width;
			vertical = v * 2 * half_height;
		}

		camera(vec3 lookfrom, vec3 lookat, vec3 up_vector, double boc, double aspect, double aperture, double focus_dist)
		{
			origin = lookfrom;
			lens_radius = aperture / 2;

			auto theta = degrees_to_radians(boc);
			auto half_height = tan(theta / 2);
			auto half_width = aspect * half_height;
			w = unit_vector(lookfrom - lookat);
			u = unit_vector(cross(up_vector, w));
			v = cross(w, u);
			lower_left_corner = origin - u * half_width * focus_dist - v * half_width * focus_dist - w * focus_dist;
			horizontal = u * 2 * half_width * focus_dist;
			vertical = v * 2 * half_height * focus_dist;
		}

		ray get_ray(double s, double t)
		{
			//return ray(origin, lower_left_corner + horizontal * s + vertical * t - origin);

			vec3 rd = random_in_unit_disk() * lens_radius;
			vec3 offset = u * rd.x() + v * rd.y();
			return ray(origin + offset, lower_left_corner + horizontal * s + vertical * t - origin - offset);
		}

	private:
		vec3 lower_left_corner;
		vec3 horizontal;
		vec3 vertical;
		vec3 origin;
		vec3 u, v, w;
		double lens_radius;
	};
}

#endif // !TOOL