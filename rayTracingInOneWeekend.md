# 前言

- 本系列总结自[raytracing.github.io](https://raytracing.github.io/books/RayTracingInOneWeekend.html)，需要一定的图形学基础，不懂的同学可以移至[GAMES101-现代计算机图形学入门-闫令琪_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1X7411F744/?spm_id_from=333.337.search-card.all.click)
- 这个版本的光线追踪非常适合初学者，难度既不大但也感受到光追的魅力



# ray tracing?

- 光线追踪过程：

  1. 每个像素点随机发射多条光线
  2. 每条光线在场景中相交并弹射，累加颜色值
  3. 将累加的颜色值返回并取平均，将得到当前像素的颜色值

- 如图所示：![image-20221225195518753](assets/image-20221225195518753.png)

- 本篇ray tracing的思路：![image-20221225201604927](assets/image-20221225201604927.png)

  

  

  

  





# ray tracing 的 hello world

​	当我们自己写渲染器时，肯定需要考虑输出渲染的图形，毕竟没有图像怎能体现渲染的趣味性。没错我想你可能猜到了，ray tracing的hello world就是输出一副图像

- 选择什么样的图片格式？

  我们知道一副图像有不同种类的格式，但问题是现在主流的图片格式都过于复杂，而写ray tracing不应聚焦于如何输出复杂格式的图片，因此我们需要一个简单的格式且它可以显示RGB颜色图，这个图片格式就是PPM

- 简单了解PPM

  PPM其实是一家三口，其中：

  1. PBM 是位图（bitmap），仅有黑与白，没有灰
  2. PGM 是灰度图（grayscale）
  3. PPM 是以RGB三种颜色显现的图像（pixmaps）

  每个图像文件的开头通过2个字节「magic number」来表明文件格式的类型，以及编码方式。具体类型如下：

  | Magic Number | Type    | Encoding |
  | ------------ | ------- | -------- |
  | P1           | Bitmap  | ASCII    |
  | P2           | Graymap | ASCII    |
  | P3           | Pixmap  | ASCII    |
  | P4           | Bitmap  | Binary   |
  | P5           | Graymap | Binary   |
  | P6           | Pixmap  | Binary   |

  我们采用的是P3，但要想查看ppm文件，我们还需要以ASCII表示图像的宽度和高度，以及最大像素值

  总的来说，PPM格式如下：

  1. 第一部分：magic number,表明文件格式的类型，以及编码方式
  2. 第二部分：图像的宽度和高度
  3. 最大像素值，0-255字节表示

- 实战

  ```c++
  void ray_tracing::output_image()
  {
      //图像的最大宽高
  	const int image_width = 1920;
  	const int image_height = 1080;
  
      //PPM格式
  	cout << "P3" << endl << image_width << ' ';
  	cout << image_height << endl << "255" << endl;
      
      //从上到下，从左到右，涂色
  	for (int h = image_height - 1; h >= 0; --h)
  	{
          //进度提示，亦可来判断程序是否卡住或死循环
  		cerr << endl << "Scanlines remaining: " << h << ' ' << flush << endl;
  		for (int w = 0; w < image_width; ++w)
  		{
  			auto r = double(h) / image_height;
  			auto g = double(w) / image_width;
  			auto b = 0.2;
  			int ir = static_cast<int>(255 * r);
  			int ig = static_cast<int>(255 * g);
  			int ib = static_cast<int>(255 * b);
  			cout << ig << ' ' << ir << ' ' << ib << endl;
  		}
  	}
  }
  ```

  注意：

  1. 通常，我们都是把RGB通道的值限定在[0.0,1.0]，但后面我们计算颜色值时会使用一个更大的动态范围，这个范围超过了[0.0,1.0]，但输出图像之前，我们会通过色调映射(Tonemapping)的方式从宽范围重新映射到[0.0,1.0]

  然后，我们需要把cout的输出流写入文件中，可以采用的方案是用windows的cmd的命名行操作符`>`来定向输出流

  具体操作如下：
  
  1. 打开cmd,cd转换路径至您系统生成的".exe"文件位置
  1. 再输入".exe"文件名 + ">image.ppm"



输出结果如下：![image-20221215235756261](assets/image-20221215235756261.png)



# 工具和常数

- 我们自己定义一些后面会用到的工具和常数。如下：

  - ```c++
    //constantAndTool.h
    #pragma once
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
    ```

# vector、ray、camera

## vector

- vector用于存储几何向量和颜色。大部分程序都是四维的，因为这可用于齐次坐标/RGB透明通道，但对于我们的程序三维足以
- 实现：

```c++
/*三维vector class*/
class vec3
{
    public:
    //表示vector
    //其中double和float都行
    double e[3];

    //构造函数
    vec3() : e{0,0,0} {}
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
        return *this *= 1/t;
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
    void write_color(std::ostream& out)
    {
        out << static_cast<int>(255 * e[0]) << ' '
            << static_cast<int>(255 * e[1]) << ' '
            << static_cast<int>(255 * e[2]) << endl;
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
    return v * (1/t);
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
```

## ray

- 光线的公式是：$\large p(t) = a + t \overrightarrow{b}$。其中：

  1. p为三维射线上一点
  2. a为射线原点
  3. b为射线方向
  4. t为实数，可以理解为时间

- 实现：

  ```c++
  /*ray class*/
  class ray
  {
      public:
      ray() {}
      ray( const vec3& origin, const vec3& direction)
          : orig( origin ), dir( direction )
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
  
  
  ```



## camera

- 我们通过从像素发射射线，并计算这些射线，来得到颜色。步骤如下：

  1. 把射线从视点转化为像素坐标
  2. 判断射线是否和场景中的物体相交
  3. 若相交，计算交点的颜色

- 用一个color()来返回渲染的颜色值，利用线性插值做混合。我们将y值范围从[-1.0,1.0]变换至[0.0,1.0]。线性插值如下形式：$\large (1-t) · startValue + t · endValue$

- 后续会进一步包装camer，先使用一个简单的

- 实现：

  ```c++
  ray_tracing::vec3 ray_tracing::ray_color(const ray& r)
  {
  	vec3 unit_direction = unit_vector(r.get_direction());
  	//以y轴值作渐变
  	auto t = (unit_direction.y() + 1.0) * 0.5;
  	return vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
  }
  ```

- 输出：![image-20221221001332383](assets/image-20221221001332383.png)



# sphere

- 做光线追踪用球体计算会非常简单，因为计算射线是否和球体相交只需初中知识。思路如下：

  1. 对于一个半径为r，圆心为(0,0,0)的球体来说，用方程$\large x^2 + y^2 + z^2 = R^2$可以表示它。若点在球体内部，那么"="改为"<"；其他情况类似。若圆心为(c$x$, c$y$, c$z$)，只需稍稍改动上述方程:$\large (x-c_x)^2 + (y - c_y)^2 + (z - c_z)^2 = R^2$
  2. 渲染中，我们希望上述所用的尽量以向量表达，因此我们设圆心$\large c = (c_x, c_y, c_z)$,球面上的点$\large p = (x,y.z)$，上述方程改为如下形式：$\large (p-c)(p-c) = R^2$。再用ray class来替换,也就是$\large (a+t \overrightarrow{b} - c)(a+t \overrightarrow{b}- c) = R^2$。最终形式如下：$$\LARGE t^2 \overrightarrow{b} · \overrightarrow{b} + 2t \overrightarrow{b} · (\overrightarrow{a} - \overrightarrow{c} ) + (\overrightarrow{a} - \overrightarrow{c} ) · (\overrightarrow{a} - \overrightarrow{c} ) - R^2 = 0$$
  3. 向量和球体的交点可能是0个，1个，2个,因此我们还需要判断射线和球体相交的个数.就是初中公式:$\large \Delta = b^2 - 4ac$

- 实现：

  ```c++
  double ray_tracing::hit_sphere(const vec3& center, double radius, const ray& r)
  {
  	vec3 oc = r.get_origin() - center;
  	auto a = dot(r.get_direction(), r.get_direction());
  	auto b = dot(oc, r.get_direction()) * 2.0;
  	auto c = dot(oc, oc) - radius * radius;
  	auto delta = b * b - 4 * a * c;
  	return (delta > 0);
  }
  
  ray_tracing::vec3 ray_tracing::ray_color(const ray& r)
  {
      if( hit_sphere(vec3(0,0,-1), 0.5, r) == true )
      {
          return vec3(1,0,0);
      }
  	vec3 unit_direction = unit_vector(r.get_direction());
  	//以y轴值作渐变
  	auto t = (unit_direction.y() + 1.0) * 0.5;
  	return vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
  }
  ```

- 输出：![image-20221221003039491](assets/image-20221221003039491.png)



# 着色

## 法线着色

- 思路：

  - 为了给球体着色，我们利用法线来着色——垂直于交点所在平面的三维向量，也就是从球心到交点再向外延伸的方向

  - xyz分量的值经常被映射到[0,1]，随后再被赋值给rgb

  - 仅仅判断射线是否和球体相交还差点味道，我们还需求出每个交点的坐标，再取最近的交点。用初中所学的公式来判断：$$\LARGE \frac {-b \pm \sqrt{b^2 - 4ac} } {2a} $$

    

- 代码：

  ```c++
  double ray_tracing::hit_sphere(const vec3& center, double radius, const ray& r)
  {
  	vec3 oc = r.get_origin() - center;
  	auto a = dot(r.get_direction(), r.get_direction());
  	auto b = 2.0 * dot(r.get_direction(), oc);
  	auto c = dot(oc, oc) - radius * radius;
  	auto delta = b * b - 4 * a * c;
  	if (delta < 0)
  	{
  		return -1.0;
  	}
  	else
  	{
  		return (-b - sqrt(delta)) / (2.0 * a);
  	}
  }
  
  ray_tracing::vec3 ray_tracing::ray_color(const ray& r)
  {
  	auto t = hit_sphere(vec3(0, 0, -1), 0.5, r);
  	if (t >= 0.0)
  	{
  		vec3 normal = unit_vector(r.at(t) - vec3(0, 0, -1));
  		return vec3(normal.x() + 1, normal.y() + 1, normal.z() + 1) * 0.5;
  	}
  	vec3 unit_direction = unit_vector(r.get_direction());
  	t = (unit_direction.y() + 1.0) * 0.5;
  	return vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
  }
  ```

- 输出：![image-20221220152306096](assets/image-20221220152306096.png)

- 简化

  - 上述代码中,b有个系数2，我们可以设b = 2h。可以被简化为如下模样:

    ```c++
    double ray_tracing::hit_sphere(const vec3& center, double radius, const ray& r)
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
    ```



## 渲染多个球体

- 思路：

  - 使用一个抽象class,任何可能和光线求交的物体 的实现都继承此class
  - 加入一个区间$\large t_{min} < t < t_{max}$来判断相交是否有效
  - 每次求交时不必都要去计算法线，我们只需计算离射线原点最近的的交点法线即可，往后的都会被遮挡

- 实现：

  ```c++
  //射线相交
  struct hit_record
  {
      vec3 p;	//交点位置
      vec3 normal;	//法线
      double t;	//ray参数
  };
  
  //物体，所有求相交和法线的物体都派生自这个class
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
      sphere(vec3 cen, double r) : center(cen), radius(r) {}
  
      virtual bool hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const;
  
      private:
      vec3 center;
      double radius;
  
  };
  
  bool ray_tracing::sphere::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
  {
  	vec3 oc = r.get_origin() - center;
  	auto a = r.get_direction().length_squared();
  	auto half_b = dot(r.get_direction(), oc);
  	auto c = oc.length_squared() - radius * radius;
  	auto delta = half_b * half_b - a * c;
      
  	if (delta > 0)
  	{
  		auto root = sqrt(delta);
  		auto temp = (-half_b - root) / a;	//ray定义：a + tb
  		if (temp > t_min && temp < t_max)
  		{
  			rec.t = temp;
  			rec.p = r.at(rec.t);
  			rec.normal = ( rec.p - center ) / radius;
  			return true;
  		}
  		temp = (-half_b + root) / a;
  		if (temp > t_min && temp < t_max)
  		{
  			rec.t = temp;
  			rec.p = r.at(rec.t);
  			rec.normal = (rec.p - center) / radius;
  			return true;
  		}
  	}
      
  	return false;
  }
  ```





## 法线朝向

- 若要求法线`朝外`，那么我们需要做到当光线从 球外 击中球体，法线 与 光线方向相反(大致反向，并非严格).当光线从 球内 击中球体，法线 与 光线方向相同，这种实现方式是用`点乘`；若要求法线`朝内`，那么我们需要做到当光线从球外击中球体，法线与光线方向相相同.当光线从球内击中球体，法线与光线方向相反,这种实现方式是用一个变量`存储入射面的信息`

- 以上两种策略都有用处，关键在于想把这部分放在着色阶段/几何阶段

- 为了更少的代码量，我们采用在几何部分来判别 入射面是内侧还是外侧

- 实现：

  ```c++
  
  struct hit_record
  {
  
      vec3 p;	//交点位置
      vec3 normal;	//法线
      double t;	//ray参数时间
      bool front_face;
  
      //让法线永远朝外
      inline void set_face_normal(const ray& r, const vec3& outward_normal)
      {
          front_face = dot(r.get_direction(), outward_normal) < 0;
          normal = front_face == true ? outward_normal : -outward_normal;
      }
  
  };
  
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
      std::vector<shared_ptr<hittable>> objects;
  };
  
  bool ray_tracing::sphere::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
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
  			return true;
  		}
  
  		temp = (-half_b + root) / a;
  		if (temp > t_min && temp < t_max)
  		{
  			rec.t = temp;
  			rec.p = r.at(rec.t);
  			vec3 outward_normal = (rec.p - center) / radius;
  			rec.set_face_normal(r, outward_normal);
  			return true;
  		}
  
  	}
  	return false;
  }
  
  bool ray_tracing::hittable_list::hit(const ray& r, const double t_min, const double t_max, hit_record& rec) const
  {
  	hit_record temp_rec;
  	bool is_hitted = false;
  	auto closet_so_far = t_max;
  
  	for (const auto& object : objects)
  	{
  		//只渲染最前面的物体颜色
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
  ```

- 输出：![image-20221220231148964](assets/image-20221220231148964.png)



# 反走样

- 真实世界中的摄像机拍摄的照片是没有像素状的锯齿。因为边缘像素是由背景和前景混合而成

- 对光线追踪器来说分层采样十分重要, 但是对于我们写的小光线追踪器并不会有很大的提升, 只会降低代码可读性,因此我们不采用分层采样

- 我们需要实现一个返回随机数的随机数生成器,目的是为了对于给定像素进行多次采样。此函数默认返回范围[0,1)。注意！此范围没取到1，这十分重要，有时可以从中受益。

- 随机数生成器实现：

  ```c++
  //constantAndTool.h
  inline double random_double()
  {
  	return rand() / (RAND_MAX + 1.0);
  }
  
  inline double random_double(double min, double max)
  {
  	return min + (max - min) * random_double();
  }
  ```

- 再包装一下camera。实现：

  ```c++
  class camera
  {
      public:
      camera()
      {
          lower_left_corner = vec3(-2.0, -1.0, -1.0);
          horizontal = vec3(4.0, 0.0, 0.0);
          vertical = vec3(0.0, 2.0, 0.0);
          origin = vec3(0.0, 0.0, 0.0);
      }
  
      ray get_ray(double u, double v)
      {
          return ray(origin, lower_left_corner + horizontal * u + vertical * v - origin);
      }
  
      private:
      vec3 lower_left_corner;
      vec3 horizontal;
      vec3 vertical;
      vec3 origin;
  };
  ```

- 为了进行多重采样，我们不会在每次发出射线采样时都计算0-1的颜色值，而是把所颜色加起来，再除以采样个数即可

- 实现：

  ```c++
  //将x限制在[min,max]
  inline double clamp(double x, double min, double max)
  {
  	if (x < min) return min;
  	if (x > max) return max;
  	return x;
  }
  
  //改动如下
  void write_color(std::ostream& out, int sample_per_pixel)
  {
      //在这之前把所有颜色值加起来，在这一步除以采样数量
      auto scale = 1.0 / sample_per_pixel;
      auto r = scale * e[0];
      auto g = scale * e[1];
      auto b = scale * e[2];
  
      out << static_cast<int>(255 * clamp( r, 0.0, 0.999) ) << ' '
          << static_cast<int>(255 * clamp( g, 0.0, 0.999) ) << ' '
          << static_cast<int>(255 * clamp( b, 0.0, 0.999) ) << endl;
  }
  
  void ray_tracing::output_image()
  {
  	const int image_width = 1920;
  	const int image_height = 1080;
  	const int samples_per_pixel = 100;
  
  	cout << "P3" << endl << image_width << ' ';
  	cout << image_height << endl << "255" << endl;
  
  	hittable_list world;
  	world.add(make_shared<sphere>(vec3(0, 0, -1), 0.5));
  	world.add(make_shared<sphere>(vec3(0, -100.5, -1), 100));
  	camera cam;
  
      for (int h = image_height - 1; h >= 0; --h)
      {
          //进度提示
          cerr << endl << "Scanlines remaining: " << h << ' ' << flush << endl;
          for (int w = 0; w < image_width; ++w)
          {
              vec3 color(0, 0, 0);
              for (int s = 0; s < samples_per_pixel; ++s)
              {
                  auto u = ((double)w + random_double() ) / image_width;
                  auto v = ((double)h + random_double() ) / image_height;
                  ray r = cam.get_ray(u, v);
                  color += ray_color(r, world);
              }
              color.write_color(cout, samples_per_pixel);
          }
      }
      cerr << endl << "Done." << endl;
  }
  ```

- 输出.边缘像素由背景和前景混合而成：![image-20221221141654836](assets/image-20221221141654836.png)



# 材质

## 漫反射材质

- 我们应该把材质和物体分开设计还是结合起来呢？

  - 若分开设计，如此可以把材质赋值给物体class的成员变量
  - 若结合起来，如此对于使用几何信息生成纹理的程序会十分方便
  - 我们采取分开设计，实际上大多数渲染器都是这样做的，其中有两种设计方法
- 漫反射材质的特点：

  - 漫反射材质不仅会接受环境光，还会在散射时让光线变成自己本身的颜色
  - 光线打到物体表面会反射到各个方向，但是表面朝向和光照夹角不同时，物体所呈现的明暗程序也会不同
  - 大部分的光线都会被吸收, 而不是被反射.表面越暗, 吸收发生的概率越大
- 生成一个粗糙不平的漫反射材质：任意的算法生成随机的反射方向(并不是Lambertian着色方程)

  - 如何求漫反射材质的反射光线？

    - 现有两个单位球体相切与点p，这两个球体的球心分别为$\large (p + \overrightarrow{N})和(p - \overrightarrow{N})$，$\large \overrightarrow{N}$为球体表面法向量，球心为$large (p + \overrightarrow{N})$的球在表面的外部，球心为$\large (p - \overrightarrow{N})$的球在表面的内部
    - 选择和光线原点位于表面同侧的单位球，并从求种随机选取一点s，向量(s-p)即为我们所求的反射光线方向


### 否定法

- 生成球体内的随机点算法——`否定法`。在一个xyz取值范围为[-1,1]的单位立方体取一个随机点，若这个随机点在球外就重新生成直到它生成在球内

  - 如下图所示：![image-20221221224747555](assets/image-20221221224747555.png)

  - 实现：

    ```c++
    //class vec3中
    inline static vec3 random()
    {
        return vec3(random_double(), random_double(), random_double());
    }
    
    inline static vec3 random(double min, double max)
    {
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }
    
    ray_tracing::vec3 ray_tracing::ray_color(const ray& r, const hittable& world,int depth)
    {
    
    	hit_record rec;
    
        //depth用于终止递归。某些条件如镜子材质的密封物体内会反复折射，这会导致无限递归
    	if (depth <= 0)
    	{
            //黑色
    		return vec3(0, 0, 0);
    	}
    	if (world.hit(r, 0, infinity, rec) == true)
    	{
    		vec3 target = rec.p + rec.normal + random_in_unit_sphere();
    		//生成的新光线继续与物体求交,求颜色值
    		return ray_color( ray(rec.p, target - rec.p), world , depth) * 0.5;
    	}
    
    	vec3 unit_direction = unit_vector(r.get_direction());
    	//以y轴值作渐变
    	auto t = (unit_direction.y() + 1.0) * 0.5;
    	return vec3(1.0, 1.0, 1.0) * (1.0 - t) + vec3(0.5, 0.7, 1.0) * t;
    }
    
    void ray_tracing::output_image()
    {
        //...
        const int max_depth = 50;
        //...
        for (int s = 0; s < samples_per_pixel; ++s)
        {
            auto u = ((double)w + random_double() ) / image_width;
            auto v = ((double)h + random_double() ) / image_height;
            ray r = cam.get_ray(u, v);
            color += ray_color(r, world,max_depth);
        }
        //...
    }
    ```

  - 输出：![image-20221221235334828](assets/image-20221221235334828.png)

  

- 注意看上图，球下面有影子且我们设计的球散射时只吸收一半的能量，且这个图片看起来和现实世界的球的亮度完全不同，这是因为所有看图软件都默认对图像进行`伽马校正`，也就是对颜色值进行了转化。我们使用"gamma 2"空间，也就是最终的颜色值加上指数1/gamma，为1/2

  - 实现：

    ```c++
    void write_color(std::ostream& out, int sample_per_pixel)
    {
        //在这之前把所有颜色值加起来，在这一步除以采样数量
        auto scale = 1.0 / sample_per_pixel;
        auto r = sqrt(scale * e[0]);
        auto g = sqrt(scale * e[1]);
        auto b = sqrt(scale * e[2]);
    
        out << static_cast<int>(255 * clamp( r, 0.0, 0.999) ) << ' '
            << static_cast<int>(255 * clamp( g, 0.0, 0.999) ) << ' '
            << static_cast<int>(255 * clamp( b, 0.0, 0.999) ) << endl;
    }
    ```

  - 输出：![image-20221222000440505](assets/image-20221222000440505.png)

  - 值得注意的是，有些物体反射的光线会在t = 0时再度打中自身。这一现象主要表现在`精度`，这个值可能是任意接近0的浮点数，因此我们需要忽略0附近的部分范围，防止这一现象(shadow ance 阴影痤疮)

    - 改动如下：

      ```c++
      if (world.hit(r, 0.001, infinity, rec) == true)
      ```


### 真正的lambertian

- 拒绝法生成的点是单位球内的随机点，这种形成的向量大概率和法线方向近似(毕竟越靠近法线长度越长)，且极小概率会沿入射方向反射回去，因此这种方法散射的光线是不均匀。这种分布律的表达式有个为$\large cos^3\theta$的系数(Θ为反射光线与法向量的夹角)。这样即使入射光线和表面的夹角很小，也会散射到很大的区域，对颜色值的影响也会更低

- 注意，真正的`lambertian`分布律的表达式的系数为$\large cos \theta$，lambertian散射的光线大多离法线更近，但更均匀。这是因为我们选取的是`单位球面的点`

- 实现：

  ```c++
  ray_tracing::vec3 ray_tracing::random_in_unit_vector()
  {
  	//三维极坐标
  	auto z = random_double(-1, 1);
  	auto r = sqrt(1 - z * z);
  	auto theta = random_double(0, 2 * pi);	//水平角
  	return vec3(r * cos(theta), r * sin(theta), z);
  }
  
  ray_tracing::vec3 ray_tracing::ray_color(const ray& r, const hittable& world,int depth)
  {
  
  	//...
  	if (world.hit(r, 0.001, infinity, rec) == true)
  	{
  		vec3 target = rec.p + rec.normal + random_in_unit_vector();
  		//生成的新光线继续与物体求交,求颜色值
  		return ray_color( ray(rec.p, target - rec.p), world , depth-1) * 0.5;
  	}
  	//...
  }
  ```

- 输出：![image-20221222143217051](assets/image-20221222143217051.png)

- 你会发现这和否定法得出的图片相差无几，这是因为我们现在用的场景过于简单。它俩的差异有：

  1. 阴影部分变少
  2. 两个球体都更亮了

- 造成这一差异的原因是散射光线的单位规一化，更少的光线会朝着法线方向散射，更均匀。对于漫反射材质，更多光线朝着摄像机反射，因此球体更亮;对于阴影部分，更少的光线朝上反射，因此这里阴影部分变少



### 早期的漫反射方法

- 从入射点选取任意一个的方向，判断是否在法向量所处的半球

- 实现：

  ```c++
  ray_tracing::vec3 ray_tracing::random_in_hemisphere( const vec3& normal )
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
  
  ray_tracing::vec3 ray_tracing::ray_color(const ray& r, const hittable& world,int depth)
  {
  
  	//...
  	if (world.hit(r, 0.001, infinity, rec) == true)
  	{
  		vec3 target = rec.p + rec.normal + random_in_hemisphere(rec.normal);
  		//生成的新光线继续与物体求交,求颜色值
  		return ray_color( ray(rec.p, target - rec.p), world , depth-1) * 0.5;
  	}
  	//...
  }
  ```

- 输出：![image-20221222153705609](assets/image-20221222153705609.png)



### 总结

- 光线追踪漫反射模型有三种方法：
  1. 否定法
     1. 思路：在单位球中任取一个点s，与两个球体相切的点p生成一条光线(s-p)
     2. 特点：
        1. 散射的光线大多靠近法线，不均匀
        2. 可以散射到很大的区域，对颜色值的影响更低
  2. 真正的lambertian
     1. 思路：在单位球面任取一个点s，与两个球体相切的点p生成一条光线(s-p)
     2. 特点：
        1. 散射的光线虽然大多靠近法线，但更均匀
        2. 阴影更少
        3. 更亮
  3. 早期的漫反射
     1. 从入射点选取任意一个的方向，判断是否在法向量所处的半球



## 金属材质(镜面反射)

- 若我们想让不同的物体拥有不同的材质，我们可以采用设计封装一个抽象的材质class.有如下两功能：

  1. 吸收入射光线，生成散射后的光线
  2. 若发生散射，应由它决定光线变暗多少

- 实现：

  ```c++
  class material
  {
      public:
      virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
  };
  
  struct hit_record
  {
      //...
      shared_ptr<material>mat_ptr;
      //...
  };
  ```

- 光线如何与表面交互由材质决定,hit_record就是为了把一堆需要的参数打包在一起。当光线于物体相交，hit_record的材质指针被物体的材质赋值。当ray_color()获取hit_record后，它根据其中的材质函数来决定散射情况。因此我们需要在物体class的加入材质指针，以便传给hit_record

- 实现：

  ```c++
  class sphere : public hittable
  {
  public:
      //...
      sphere(vec3 cen, double r, shared_ptr<material>m) : center(cen), radius(r), mat_ptr(m) {}
  
  private:
  	//...
      shared_ptr<material>mat_ptr;
  };
  
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
  			return true;
  		}
  
  	}
  	return false;
  }
  ```

- 对于`lambertian材质`，我们有如下四种思路：

  1. 让光线永远散射但每次衰减至R
  2. 让光线不衰减，而是物体吸收(1-R)的光线
  3. 上述两种的结合
  4. 让光线以一定概率p散射，并使光线的衰减率为 albedo / p

- 实现：

  ```c++
  class lambertian : public material
  {
      public:
      virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
      {
          vec3 scatter_direction = rec.normal + random_in_unit_vector();
          scattered = ray(rec.p, scatter_direction);
          attenuation = albedo;
          return true;
      }
  
      private:
      vec3 albedo;
  };
  ```

- 对于`光滑的金属材质`，光线不会如漫反射一般 随机散射，而是反射

- 如何反射？如图所示，n为单位向量法线，v为入射光线。b长度为v · n，因为v和n方向相反，因此需要加负号：![image-20221222231120653](assets/image-20221222231120653.png)

- 实现：

  ```c++
  //反射函数
  inline vec3 reflect(const vec3& v, const vec3& n)
  {
      return v - n * 2 * dot(v, n);
  }
  
  class metal : public material
  {
      public:
      metal( const vec3& a ) : albedo(a){}
  
      virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
      {
          vec3 reflected = reflect(unit_vector(r_in.get_direction()), rec.normal);
          scattered = ray(rec.p, reflected);
          attenuation = albedo;
          return (dot(scattered.get_direction(), rec.normal) > 0);
      }
  
      private:
      vec3 albedo;
  };
  
  vec3 ray_color(const ray& r, const hittable& world,int depth)
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
  
  
  ```

- 输出：![image-20221223140949691](assets/image-20221223140949691.png)

- `模糊`的金属

  - 我们可以给反射方向增添点随机性，只需在求出反射向量后，在其终点为球心的球内随机选取一个点作为最终的终点

  - 我们引入一个fuzz变量表示模糊程度，可以理解为物体吸收光线

  - 实现：

    ```c++
    class metal : public material
    {
        public:
        metal(const vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}
    
        virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
        {
            vec3 reflected = reflect(unit_vector(r_in.get_direction()), rec.normal);
            scattered = ray(rec.p, reflected + random_in_unit_sphere() * fuzz );
            attenuation = albedo;
            return (dot(scattered.get_direction(), rec.normal) > 0);
        }
    
        private:
        vec3 albedo;
        double fuzz;
    };
    
    void output_image()
    {
        world.add(make_shared<sphere>(vec3(1, 0, -1), 0.5, make_shared<metal>(vec3(0.8, 0.6, 0.2), 0.3)));
    		world.add(make_shared<sphere>(vec3(-1, 0, -1), 0.5, make_shared<metal>(vec3(0.8, 0.8, 0.8), 1.0)));
    }
    ```

  - 输出：![image-20221223151240422](assets/image-20221223151240422.png)




## 绝缘体材质(折射)

- 透明的材料，如水、玻璃等都是绝缘体。当光线打中这类材料时，光线会被分成两条，一条反射，一条折射。对于绝缘体材质，我们采取这样的方案：当光线和绝缘体材质的物体相交时，要么反射，要么折射，随机选取。最后对求得的值取平均

- 折射法则：

  - $\large \eta · sin \theta = \eta' · sin \theta'$。其中，Θ和Θ‘分别为入射光线 和 折射光线距离法线的夹角
  - 过程如下：![image-20221223184117243](assets/image-20221223184117243.png)
  - 我们需要的是折射光线的方向，解得$\large sin \theta' = \frac{\eta}{\eta'} · sin \theta$

- 在折射介质部分有未知的折射光线 R' 和 法向量 N’，都为单位向量.为了求R'我们可以把R'分解为垂直N'的向量 和 平行N'的向量，其中:

  1. $\large R'_{\|} = \frac{\eta}{\eta'}(R + (-R · N) N) . R'_{\|} = \frac{\eta}{\eta'}(R + cos \theta N), cos \theta = A · B$

  2. $\large R'_{\perp} = -\sqrt{1 - |R'_{\|}|^2 }N$

     

     推导过程如下:![image-20221224002236561](assets/image-20221224002236561.png)

- 实现：

  ```c++
  //折射
  inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat)
  {
      auto cos_theta = dot(-uv, n);
      vec3 r_out_parallel = (uv + n * cos_theta) * etai_over_etat;
      vec3 r_out_perp =  n * ( -sqrt(1 - r_out_parallel.length_squared()));
      return r_out_parallel + r_out_perp;
  }
  
  //绝缘体
  class dielectric : public material
  {
      public:
      dielectric(double ri) : ref_idx(ri) {}
  
      virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
      {
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
          vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
          scattered = ray(rec.p, refracted);
          return true;
      }
  
      private:
      double ref_idx;
  };
  
  void output_image()
  {
      //...
      world.add(make_shared<sphere>(vec3(-1, 0, -1), 0.5, make_shared<dielectric>(1.5)));
      //...
  }
  
  ```

- 输出：![image-20221224134918660](assets/image-20221224134918660.png)

- 可以看到上图左下角有小黑点,为什么会出现这种情况？因为当光线从高折射率介质射入低折射率介质时，上面的折射法则并不满足sinΘ > 1这一情况。这时并不发生折射

  - 证明如下：若光线从玻璃(η = 1.5)到空气(η = 1.0)：$$\LARGE sin\theta' = \frac{1.5}{1.0} · sin\theta \\ \LARGE \frac{1.5}{1.0}·sin \theta > 1.0 \\ \large因为sin\theta'不可能>1，所以一旦上述式子成立，这时将是无解的$$

  - 改进：当光线无法折射时，我们应让他反射。因为这类情况经常发生在实心物体内部，又被称为"全内反射"

    $$\LARGE sin \theta = \sqrt{1 - cos^2 \theta}, cos\theta = -R · N$$

  - 实现：

    ```c++
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
            vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
            scattered = ray(rec.p, refracted);
            return true;
        }
    
        private:
        double ref_idx;
    };
    ```

    

  - 输出：![image-20221224211314004](assets/image-20221224211314004.png)

- 玻璃发生折射的概率随入射角改变,但这个式子实在复杂又臭又长，我们这里用一个数学上的近似等式

  - 实现：

    ```c++
    double schlick(double cosine, double ref_idx)
    {
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 *= r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
    ```

- 最终的玻璃材质：

  ```c++
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
  ```

- 通透的玻璃球：将球的半径设为负值，法相全部翻转至内部。也就是把一个小球套在大球里，光线发生两次折射，负负得正

  - 实现：

    ```c++
    world.add(make_shared<sphere>(vec3(-1, 0, -1), -0.45, make_shared<dielectric>(1.5)));
    ```

  - 输出：![image-20221224224337433](assets/image-20221224224337433.png)

  



# 自定义位置的摄像机

- 垂直视场角(BOC)：摄像机垂直方向能看到的视野范围；水平视场角（AOB）：摄像机垂直方向能看到的视野范围

- 为了方便，我们使用角度制传参，而在构造函数中转为弧度

- 我们让射线从原点射向z = -1的平面。如图，很显然$\large h = tan(\theta) $：![image-20221224215228002](assets/image-20221224215228002.png)

  - 实现：

    ```c++
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
    
    //main.cpp
    auto R = cos(pi / 4);
    hittable_list world;
    world.add(make_shared<sphere>(vec3(-R, 0, -1), R, make_shared<lambertian>(vec3(0, 0, 1))));
    world.add(make_shared<sphere>(vec3(R, 0, -1), R, make_shared<lambertian>(vec3(1, 0, 0))));
    ```

  - 输出：![image-20221224233619022](assets/image-20221224233619022.png)

- 很轻易的看出目前camera只能上下倾斜，而不能左右倾斜，因此我们需要一个vector来指定摄像机坐标系的正上方方向up vector。如图，以u,v,w来描述摄像机的转向，其中uvw处于同一平面，摄像机对着-w方向。一般而言我们将up vector定为(0,1,0)，这样很方便：![image-20221224223143584](assets/image-20221224223143584.png)

  - 实现：

    ```c++
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
    
    //main.cpp
    const auto aspect_ratio = double(image_width) / image_height;
    vec3 up_vector(0, 1, 0);
    camera cam(vec3( -2, 2, 1), vec3(0,0,-1), up_vector, 90, aspect_ratio);
    ```

  - 输出：![image-20221225002842783](assets/image-20221225002842783.png)







# 景深

- 相机在对焦时模糊是因为他的孔很小，这会导致所有光线被散焦。但若我们在孔中加入一块透镜，在一段距离内的物体都会被对焦。也就是说，所有光线从一点分散射出，击中透镜后又聚焦在图像传感器上的一点。如图：![image-20221225001122525](assets/image-20221225001122525.png)

- 对于相机来说，物体在哪里被聚焦是由透镜 距离成像平面 和 距离聚焦平面 的距离长度所决定。当改变对焦时，相机中的透镜位置也会发生改变。对于一个虚拟摄像机，我们只需要一个传感器足以，因此我们只需传入透镜大小

- 模拟摄像机顺序：图像传感器->透镜->快门->射入光线->翻转图片。但我们不需要模拟摄像机内部，我们只需从一个虚拟的透镜范围中发射光线到摄像机平面即可

- 之前我们的实现中所有光线由lookfrom发出的，但现在加入了景深，所有光线应该都从内部的一个虚拟透镜发出，再经过lookfrom点，透镜半径越大，图像越模糊，也就是说之前的camera其实可以认为透镜半径为0

- 实现：

  ```c++
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
  
  
  ```

- 输出：![image-20221225123523035](assets/image-20221225123523035.png)





# 融合所学

- 把前面所学的融合一下，用以下代码输出一个经典例子：

  ```c++
  hittable_list random_scene()
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
  
  void output_image()
  {
      //...
      auto world = random_scene();
      vec3 lookfrom(13, 2, 3);
      vec3 lookat(0, 0, 0);
      vec3 up_vector(0, 1, 0);
      auto dist_to_focus = 10.0;
      auto aperture = 0.1;
      const auto aspect_ratio = double(image_width) / image_height;
  
      camera cam(lookfrom, lookat, up_vector, 20, aspect_ratio, aperture, dist_to_focus);
      //...
  }
  ```

- 输出：



# reference

[数字图像处理-PPM图像格式 - 简书 (jianshu.com)](https://www.jianshu.com/p/e809269b4ad7)

[Ray Tracing in One Weekend V3.0中文翻译（上） - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/128582904)













