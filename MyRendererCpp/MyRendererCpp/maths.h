#ifndef MATHS_H
#define MATHS_H
#include <math.h>
#include <iostream>
#include <initializer_list>

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))
#define PI 3.1415927f

//模板改造数学库计划
//1.将数学库的函数全部改为模板函数

template<typename T, int dim> 
class vec
{
public:
	vec() { for (int i = 0; i < dim; i++) data[i] = 0; }

    vec(std::initializer_list<T> list)
    {
        int i = 0;
        for (auto& e : list)
        {
            data[i] = e;
            i++;
        }
    }

    // 拷贝构造函数
    vec(const vec& v)
    {
        for (int i = 0; i < dim; i++)
        {
            data[i] = v.data[i];
        }
    }

    //// 移动构造函数
    //vec(vec&& v)
    //{
    //    data = v.data;
    //    v.data = nullptr;
    //}

    //重载赋值，移动语义
    /*vec& operator=(vec&& v)
    {
        data = v.data;
        v.data = nullptr;
        return *this;
    }*/

    vec& operator=(const vec& v)
    {
        for (int i = 0; i < dim; i++)
        {
            data[i] = v.data[i];
        }
        return *this;
    }

    //重载+
    vec operator+(const vec& v)
    {
        vec res;
        for (int i = 0; i < dim; i++)
        {
            res.data[i] = data[i] + v.data[i];
        }
        return res;
    }

    //重载-
    vec operator-(const vec& v)
	{
		vec res;
		for (int i = 0; i < dim; i++)
		{
			res.data[i] = data[i] - v.data[i];
		}
		return res;
	}

    // 重载 * 操作符，支持任意类型的因子
    template<typename U>
    vec operator*(U factor) const 
    {
        vec res; // 结果向量
        for (int i = 0; i < dim; ++i) 
        {
            res.data[i] = data[i] * factor; // 将每个分量与因子相乘
        }
        return res; // 返回结果
    }

    // 重载 / 操作符，支持任意类型的除数
    template<typename U>
    vec operator/(U divisor) const
    {
        static_assert(std::is_arithmetic<U>::value, "divisor must be an arithmetic type."); // 确保 U 是算术类型
        if (divisor == 0) // 检查除数是否为零
        { 
            throw std::invalid_argument("Division by zero is not allowed."); // 抛出异常
        }
		vec res; // 结果向量
		for (int i = 0; i < dim; ++i) 
		{
			res.data[i] = data[i] / divisor; // 将每个分量与除数相除
		}
		return res; // 返回结果
	}

    //重载+=
    vec& operator+=(const vec& v)
	{
		for (int i = 0; i < dim; i++)
		{
			data[i] += v.data[i];
		}
		return *this;
	}

    vec cross(const vec& v)
    {
        std::cout << "请检查叉乘的维度是否正确！2/3" << std::endl;
        return vec();
    }

    //求解向量的模
    T length()
    {
		T sum = 0;
		for (int i = 0; i < dim; i++)
		{
			sum += data[i] * data[i];
		}
		return sqrt(sum);
    }

    //normalized 函数的实现
    vec normalized()
	{
		T len = length();
        if (len == 0)
		{
            std::cout << "向量长度为0，无法归一化" << std::endl;
			return vec();
		}
        vec result;
        for (int i = 0; i < dim; i++)
        {
            result.data[i] = data[i] / len;
        }
        return result;
	}

    void reset()
	{
		for (int i = 0; i < dim; i++)
		{
			data[i] = 0;
		}
	}

    //重载索引运算符
    T& operator[](int index)
	{
		return data[index];
	}

    const T& operator[](int index) const 
    {
        return data[index];
    }

    void print() const
    {
        if (data == nullptr)
        {
            std::cout << "data is nullptr" << std::endl;
            return;
        }
        for (int i = 0; i < dim; i++)
        {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }


protected:
	T data[dim];
};

//比较两个vec各分量的最小值，然后返回一个新的vec
template<typename T, int dim>
vec<T, dim> vec_min(const vec<T, dim>& a, const vec<T, dim>& b)
{
    vec<T, dim> result; // 创建结果向量
    for (int i = 0; i < dim; ++i) 
    {
        result.data[i] = (a.data[i] < b.data[i]) ? a.data[i] : b.data[i]; // 比较并赋值
    }
    return result; // 返回结果向量
}

//比较两个vec各分量的最大值，然后返回一个新的vec
template<typename T, int dim>
vec<T, dim> vec_max(const vec<T, dim>& a, const vec<T, dim>& b)
{
	vec<T, dim> result; // 创建结果向量
	for (int i = 0; i < dim; ++i) 
    {
		result.data[i] = (a.data[i] > b.data[i]) ? a.data[i] : b.data[i]; // 比较并赋值
	}
	return result; // 返回结果向量
}

// vec2 类，继承自 vec<T, 2>
template<typename T>
class vec2 : public vec<T, 2> {
public:
    using vec<T, 2>::vec; // 继承构造函数

    // 特定于2D的交叉乘积返回标量
    T cross(const vec2<T>& v) const {
        return this->data[0] * v.data[1] - this->data[1] * v.data[0];
    }

    // 从 vec<T, 2> 转换构造函数
    vec2(const vec<T, 2>& v) {
        this->data[0] = v[0];
        this->data[1] = v[1];
    }
};

// vec3 类，继承自 vec<T, 3>
template<typename T>
class vec3 : public vec<T, 3> {
public:
    using vec<T, 3>::vec; // 继承构造函数

    // 特定于3D的交叉乘积，返回vec3对象
    vec3<T> cross(const vec3<T>& v) const {
        vec3<T> result;
        result[0] = this->data[1] * v.data[2] - this->data[2] * v.data[1];
        result[1] = this->data[2] * v.data[0] - this->data[0] * v.data[2];
        result[2] = this->data[0] * v.data[1] - this->data[1] * v.data[0];
        return result;
    }

    // 从 vec<T, 3> 转换构造函数
    vec3(const vec<T, 3>& v) {
        this->data[0] = v[0];
        this->data[1] = v[1];
        this->data[2] = v[2];
    }
};


typedef struct { float x, y; } vec2_t;
typedef struct { float x, y, z; } vec3_t;
typedef struct { float x, y, z, w; } vec4_t;
typedef struct { float x, y, z, w; } quat_t;

typedef struct { float m[3][3]; } mat3_t;
typedef struct { float m[4][4]; } mat4_t;

float float_min(float a, float b);
float float_max(float a, float b);

float float_clamp(float f, float min, float max);
int min_integer(int a, int b);

int max_integer(int a, int b);
float float_from_uchar(unsigned char value);
unsigned char float_to_uchar(float value);
vec2_t vec2_sub(vec2_t a, vec2_t b);
float vec2_length(vec2_t v);
vec2_t vec2_new(float x, float y);
vec3_t vec3_new(float x, float y, float z);
vec2_t vec2_min(vec2_t a, vec2_t b);
vec2_t vec2_max(vec2_t a, vec2_t b);
vec2_t vec2_add(vec2_t a, vec2_t b);
vec2_t vec2_mul(vec2_t v, float factor);
vec3_t vec3_add(vec3_t a, vec3_t b);
vec3_t vec3_max(vec3_t a, vec3_t b);
vec3_t vec3_min(vec3_t a, vec3_t b);
vec2_t vec2_div(vec2_t v, float divisor);
vec3_t vec3_sub(vec3_t a, vec3_t b);
vec3_t vec3_cross(vec3_t a, vec3_t b);
vec4_t vec4_new(float x, float y, float z, float w);
vec4_t vec4_from_vec3(vec3_t v, float w);
vec3_t vec3_from_vec4(vec4_t v);

vec3_t vec3_add(vec3_t a, vec3_t b);

vec4_t vec4_mul(vec4_t v, float factor);
vec4_t vec4_div(vec4_t v, float divisor);

vec4_t vec4_add(vec4_t a, vec4_t b);
vec4_t vec4_mul_vec4(vec4_t a, vec4_t b);

mat4_t mat4_identity(void);
/* transformation matrices */
mat4_t mat4_translate(float tx, float ty, float tz);

mat4_t mat4_rotate(float angle, float vx, float vy, float vz);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate(float angle, float vx, float vy, float vz);
vec3_t vec3_normalize(vec3_t v);
float vec3_dot(vec3_t a, vec3_t b);
float vec3_length(vec3_t v);
vec3_t vec3_mul(vec3_t v, float factor);
vec3_t vec3_div(vec3_t v, float divisor);
vec3_t mat3_mul_vec3(mat3_t m, vec3_t v);

mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_orthographic(float right, float top, float near, float far);

mat4_t mat4_frustum(float left, float right, float bottom, float top,
	float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);

vec3_t vec3_lerp(vec3_t a, vec3_t b, float t);
float float_lerp(float a, float b, float t);
quat_t quat_new(float x, float y, float z, float w);
quat_t quat_slerp(quat_t a, quat_t b, float t);
float quat_dot(quat_t a, quat_t b);
mat4_t mat4_from_trs(vec3_t t, quat_t r, vec3_t s);
mat4_t mat4_from_quat(quat_t q);
mat3_t mat3_inverse_transpose(mat3_t m);
static mat3_t mat3_adjoint(mat3_t m);
static float mat3_determinant(mat3_t m);
mat3_t mat3_from_mat4(mat4_t m);
mat4_t mat4_combine(mat4_t m[4], vec4_t weights_);
mat3_t mat3_combine(mat3_t m[4], vec4_t weights_);
mat3_t mat3_mul_mat3(mat3_t a, mat3_t b);
vec3_t vec3_modulate(vec3_t a, vec3_t b);
vec3_t vec3_negate(vec3_t v);

vec4_t vec4_saturate(vec4_t v);
float float_saturate(float f);
mat3_t mat3_from_cols(vec3_t c0, vec3_t c1, vec3_t c2);

float float_linear2srgb(float value);
float float_aces(float value);

vec4_t vec4_lerp(vec4_t a, vec4_t b, float t);
#endif