#ifndef MATHS_H
#define MATHS_H
#include <math.h>
#include <iostream>
#include <initializer_list>

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))
#define PI 3.1415927f

//ģ�������ѧ��ƻ�
//1.����ѧ��ĺ���ȫ����Ϊģ�庯��

template<typename T, int dim> 
class vec
{
public:
    vec() = default;
    vec(std::initializer_list<T> list) 
    {
        std::copy(list.begin(), list.end(), data);
    }

    vec(const vec&) = default;
    vec& operator=(const vec&) = default;
    
    vec(vec&&) noexcept = default;
    vec& operator=(vec&&) noexcept = default;

    inline vec operator+(const vec& v) const 
    {
        vec res;
        for (int i = 0; i < dim; ++i) res.data[i] = data[i] + v.data[i];
        return res;
    }

    //����-
    inline vec operator-(const vec& v) const
	{
		vec res;
		for (int i = 0; i < dim; i++)
		{
			res.data[i] = data[i] - v.data[i];
		}
		return res;
	}

    // ���� * ��������֧���������͵�����
    template<typename U>
    vec operator*(U factor) const 
    {
        vec res; // �������
        for (int i = 0; i < dim; ++i) 
        {
            res.data[i] = data[i] * factor; // ��ÿ���������������
        }
        return res; // ���ؽ��
    }

    inline vec operator*(const vec& v) const //�������
	{  
		vec res;
		for (int i = 0; i < dim; i++)
		{
			res.data[i] = data[i] * v.data[i];
		}
		return res;
	}

    // ���� / ��������֧���������͵ĳ���
    template<typename U>
    vec operator/(U divisor) const
    {
        static_assert(std::is_arithmetic<U>::value, "divisor must be an arithmetic type."); // ȷ�� U ����������
        if (divisor == 0) // �������Ƿ�Ϊ��
        { 
            throw std::invalid_argument("Division by zero is not allowed."); // �׳��쳣
        }
		vec res; // �������
		for (int i = 0; i < dim; ++i) 
		{
			res.data[i] = data[i] / divisor; // ��ÿ��������������
		}
		return res; // ���ؽ��
	}

    //����+=
    vec& operator+=(const vec& v)
	{
		for (int i = 0; i < dim; i++)
		{
			data[i] += v.data[i];
		}
		return *this;
	}

    //����-=
    vec& operator-=(const vec& v)
    {
        for (int i = 0; i < dim; i++)
		{
			data[i] -= v.data[i];
		}
		return *this;
    }

    vec cross(const vec& v)
    {
        std::cout << "�����˵�ά���Ƿ���ȷ��2/3" << std::endl;
        return vec();
    }

    //���������ģ
    inline T length() const 
    {
        T sum = 0;
        for (int i = 0; i < dim; ++i) sum += data[i] * data[i];
        return std::sqrt(sum);
    }

    //normalized ������ʵ��
    vec normalized()
	{
		T len = length();
        if (len == 0)
		{
            std::cout << "��������Ϊ0���޷���һ��" << std::endl;
			return vec();
		}
        vec result = *this / len;
        return result;
	}

    void reset()
	{
		for (int i = 0; i < dim; i++)
		{
			data[i] = 0;
		}
	}

    //�������������
    T& operator[](int index)
	{
		return data[index];
	}

    const T& operator[](int index) const 
    {
        return data[index];
    }

    //���ظ���
    vec operator-() const
	{
		vec result;
		for (int i = 0; i < dim; i++)
		{
			result.data[i] = -data[i];
		}
		return result;
	}


    inline void print() const 
    {
        for (int i = 0; i < dim; ++i) std::cout << data[i] << " ";
        std::cout << '\n';
    }

    T data[dim]{};
};

//quat = vec<float,4>
using quat = vec<float, 4>;
using vec4 = vec<float, 4>;
using vec3 = vec<float, 3>;
using vec2 = vec<float, 2>;

//�Ƚ�����vec����������Сֵ��Ȼ�󷵻�һ���µ�vec
template<typename T, int dim>
vec<T, dim> vec_min(const vec<T, dim>& a, const vec<T, dim>& b)
{
    vec<T, dim> result; // �����������
    for (int i = 0; i < dim; ++i) 
    {
        result.data[i] = (a.data[i] < b.data[i]) ? a.data[i] : b.data[i]; // �Ƚϲ���ֵ
    }
    return result; // ���ؽ������
}

//�Ƚ�����vec�����������ֵ��Ȼ�󷵻�һ���µ�vec
template<typename T, int dim>
vec<T, dim> vec_max(const vec<T, dim>& a, const vec<T, dim>& b)
{
	vec<T, dim> result; // �����������
	for (int i = 0; i < dim; ++i) 
    {
		result.data[i] = (a.data[i] > b.data[i]) ? a.data[i] : b.data[i]; // �Ƚϲ���ֵ
	}
	return result; // ���ؽ������
}

//���
template<typename T, int dim>
T dot(const vec<T, dim>& a, const vec<T, dim>& b)
{
	T result = 0; // �������
	for (int i = 0; i < dim; ++i) 
	{
		result += a.data[i] * b.data[i]; // �ۼ�
	}
	return result; // ���ؽ��
}

vec3 cross(const vec3& a, const vec3& b);

//lerp
template<typename T, int dim>
vec<T, dim> lerp(const vec<T, dim>& a, const vec<T, dim>& b, T t)
{
	return a * (1 - t) + b * t;
}

vec4 vec4_from_vec3(const vec3& v, float w);
vec3 vec3_from_vec4(const vec4& v);


typedef struct { float m[3][3]; } mat3_t;
typedef struct { float m[4][4]; } mat4_t;

float float_min(float a, float b);
float float_max(float a, float b);

float float_clamp(float f, float min, float max);
int min_integer(int a, int b);

int max_integer(int a, int b);
float float_from_uchar(unsigned char value);
unsigned char float_to_uchar(float value);

mat4_t mat4_identity(void);
/* transformation matrices */
mat4_t mat4_translate(float tx, float ty, float tz);

mat4_t mat4_rotate(float angle, float vx, float vy, float vz);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_scale(float sx, float sy, float sz);
mat4_t mat4_rotate(float angle, float vx, float vy, float vz);


vec3 mat3_mul_vec3(mat3_t m, vec3 v);

mat4_t mat4_lookat(vec3 eye, vec3 target, vec3 up);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_orthographic(float right, float top, float near, float far);

mat4_t mat4_frustum(float left, float right, float bottom, float top,
	float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);
vec4 mat4_mul_vec4(mat4_t m, vec4 v);
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);

float float_lerp(float a, float b, float t);
quat quat_slerp(quat& a, quat& b, float t);
mat4_t mat4_from_trs(vec3& t, quat& r, vec3& s);
mat4_t mat4_from_quat(quat& q);
mat3_t mat3_inverse_transpose(mat3_t m);
static mat3_t mat3_adjoint(mat3_t m);
static float mat3_determinant(mat3_t m);
mat3_t mat3_from_mat4(mat4_t m);
mat4_t mat4_combine(mat4_t m[4], vec4& weights_);
mat3_t mat3_combine(mat3_t m[4], vec4& weights_);
mat3_t mat3_mul_mat3(mat3_t a, mat3_t b);

vec4 vec4_saturate(vec4 v);
float float_saturate(float f);
mat3_t mat3_from_cols(vec3 c0, vec3 c1, vec3 c2);

float float_linear2srgb(float value);
float float_aces(float value);
#endif