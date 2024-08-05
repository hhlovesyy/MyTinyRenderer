#include "maths.h"
#include <assert.h>

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


vec3_t vec3_min(vec3_t a, vec3_t b) 
{
    float x = float_min(a.x, b.x);
    float y = float_min(a.y, b.y);
    float z = float_min(a.z, b.z);
    return vec3_new(x, y, z);
}

vec3_t vec3_max(vec3_t a, vec3_t b)
{
    float x = float_max(a.x, b.x);
    float y = float_max(a.y, b.y);
    float z = float_max(a.z, b.z);
    return vec3_new(x, y, z);
}

vec3_t vec3_add(vec3_t a, vec3_t b) 
{
    return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z);
}

vec3_t vec3_sub(vec3_t a, vec3_t b) 
{
    return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec3_t vec3_mul(vec3_t v, float factor) 
{
    return vec3_new(v.x * factor, v.y * factor, v.z * factor);
}

vec3_t vec3_div(vec3_t v, float divisor) 
{
    return vec3_mul(v, 1 / divisor);
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

mat4_t mat4_identity(void)
{
    mat4_t m =
    { {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1},
    } };
    return m;
}

mat4_t mat4_translate(float tx, float ty, float tz)
{
    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

vec4_t mat4_mul_vec4(mat4_t& m, vec4_t& v) 
{
    float product[4];
    int i;
    for (i = 0; i < 4; i++) 
    {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        float d = m.m[i][3] * v.w;
        product[i] = a + b + c + d;
    }
    return vec4_new(product[0], product[1], product[2], product[3]);
}

/*
 * angle: the angle of rotation, in radians
 *
 *  1  0  0  0
 *  0  c -s  0
 *  0  s  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_x(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c  0  s  0
 *  0  1  0  0
 * -s  0  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_y(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c -s  0  0
 *  s  c  0  0
 *  0  0  1  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_z(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return m;
}

/*
 * sx, sy, sz: scale factors along the x, y, and z axes, respectively
 *
 * sx  0  0  0
 *  0 sy  0  0
 *  0  0 sz  0
 *  0  0  0  1
 *
 * see http://docs.gl/gl2/glScale
 */
mat4_t mat4_scale(float sx, float sy, float sz) 
{
    mat4_t m = mat4_identity();
    assert(sx != 0 && sy != 0 && sz != 0);
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}