#include "maths.h"

float float_min(float a, float b) 
{
    return a < b ? a : b;
}

float float_max(float a, float b) 
{
    return a > b ? a : b;
}

int min_integer(int a, int b) 
{
    return a < b ? a : b;
}

int max_integer(int a, int b) 
{
    return a > b ? a : b;
}

unsigned char float_to_uchar(float value) 
{
    return (unsigned char)(value * 255);
}

vec2_t vec2_sub(vec2_t a, vec2_t b) 
{
    return vec2_new(a.x - b.x, a.y - b.y);
}

vec2_t vec2_new(float x, float y)
{
    vec2_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec3_t vec3_new(float x, float y, float z) 
{
    vec3_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}
//返回一个vec2_t，其中的x和y分别取两个vec2_t的x和y的最小值
vec2_t vec2_min(vec2_t a, vec2_t b)
{
    float x = float_min(a.x, b.x);
    float y = float_min(a.y, b.y);
    return vec2_new(x, y);
}

vec2_t vec2_max(vec2_t a, vec2_t b)
{
    float x = float_max(a.x, b.x);
    float y = float_max(a.y, b.y);
    return vec2_new(x, y);
}

vec4_t vec4_new(float x, float y, float z, float w) 
{
    vec4_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}