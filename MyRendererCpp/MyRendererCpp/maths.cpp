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

float float_clamp(float f, float min, float max) 
{
    return f < min ? min : (f > max ? max : f);
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

vec2_t vec2_mul(vec2_t v, float factor)
{
    return vec2_new(v.x * factor, v.y * factor);
}

vec2_t vec2_div(vec2_t v, float divisor)
{
    return vec2_mul(v, 1 / divisor);
}

float vec2_length(vec2_t v) 
{
    return (float)sqrt(v.x * v.x + v.y * v.y);
}

vec3_t vec3_sub(vec3_t a, vec3_t b)
{
    return vec3_new(a.x - b.x, a.y - b.y, a.z - b.z);
}

vec2_t vec2_new(float x, float y)
{
    vec2_t v;
    v.x = x;
    v.y = y;
    return v;
}

vec2_t vec2_add(vec2_t a, vec2_t b)
{
    return vec2_new(a.x + b.x, a.y + b.y);
}

vec3_t vec3_new(float x, float y, float z) 
{
    vec3_t v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

vec3_t vec3_add(vec3_t a, vec3_t b)
{
    return vec3_new(a.x + b.x, a.y + b.y, a.z + b.z);
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

float vec3_dot(vec3_t a, vec3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(vec3_t v) 
{
    return (float)sqrt(vec3_dot(v, v));
}

vec3_t vec3_mul(vec3_t v, float factor) 
{
    return vec3_new(v.x * factor, v.y * factor, v.z * factor);
}

vec3_t vec3_div(vec3_t v, float divisor) 
{
    return vec3_mul(v, 1 / divisor);
}

vec3_t vec3_normalize(vec3_t v) 
{
    return vec3_div(v, vec3_length(v));
}

vec3_t vec3_cross(vec3_t a, vec3_t b)
{
    float x = a.y * b.z - a.z * b.y;
    float y = a.z * b.x - a.x * b.z;
    float z = a.x * b.y - a.y * b.x;
    return vec3_new(x, y, z);
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


vec4_t vec4_add(vec4_t a, vec4_t b) 
{
    return vec4_new(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

vec4_t vec4_from_vec3(vec3_t v, float w)
{
    return vec4_new(v.x, v.y, v.z, w);
}

vec3_t vec3_from_vec4(vec4_t v) 
{
    return vec3_new(v.x, v.y, v.z);
}

vec4_t vec4_mul(vec4_t v, float factor) 
{
    return vec4_new(v.x * factor, v.y * factor, v.z * factor, v.w * factor);
}

vec4_t vec4_div(vec4_t v, float divisor)
{
    return vec4_mul(v, 1 / divisor);
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

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) 
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
 * angle: the angle of rotation, in radians
 * vx, vy, vz: the x, y, and z coordinates of a vector, respectively
 *
 * nx*nx*(1-c)+c     ny*nx*(1-c)-s*nz  nz*nx*(1-c)+s*ny  0
 * nx*ny*(1-c)+s*nz  ny*ny*(1-c)+c     nz*ny*(1-c)-s*nx  0
 * nx*nz*(1-c)-s*ny  ny*nz*(1-c)+s*nx  nz*nz*(1-c)+c     0
 * 0                 0                 0                 1
 *
 * nx, ny, nz: the normalized coordinates of the vector, respectively
 * s, c: sin(angle), cos(angle)
 *
 * see http://docs.gl/gl2/glRotate
 */
mat4_t mat4_rotate(float angle, float vx, float vy, float vz) 
{
    vec3_t n = vec3_normalize(vec3_new(vx, vy, vz));
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();

    m.m[0][0] = n.x * n.x * (1 - c) + c;
    m.m[0][1] = n.y * n.x * (1 - c) - s * n.z;
    m.m[0][2] = n.z * n.x * (1 - c) + s * n.y;

    m.m[1][0] = n.x * n.y * (1 - c) + s * n.z;
    m.m[1][1] = n.y * n.y * (1 - c) + c;
    m.m[1][2] = n.z * n.y * (1 - c) - s * n.x;

    m.m[2][0] = n.x * n.z * (1 - c) - s * n.y;
    m.m[2][1] = n.y * n.z * (1 - c) + s * n.x;
    m.m[2][2] = n.z * n.z * (1 - c) + c;

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

/*
 * eye: the position of the eye point
 * target: the position of the target point
 * up: the direction of the up vector
 *
 * x_axis.x  x_axis.y  x_axis.z  -dot(x_axis,eye)
 * y_axis.x  y_axis.y  y_axis.z  -dot(y_axis,eye)
 * z_axis.x  z_axis.y  z_axis.z  -dot(z_axis,eye)
 *        0         0         0                 1
 *
 * z_axis: normalize(eye-target), the backward vector
 * x_axis: normalize(cross(up,z_axis)), the right vector
 * y_axis: cross(z_axis,x_axis), the up vector
 *
 * see http://www.songho.ca/opengl/gl_camera.html
 */
mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up) 
{
    vec3_t z_axis = vec3_normalize(vec3_sub(eye, target));
    vec3_t x_axis = vec3_normalize(vec3_cross(up, z_axis));
    vec3_t y_axis = vec3_cross(z_axis, x_axis);
    mat4_t m = mat4_identity();

    m.m[0][0] = x_axis.x;
    m.m[0][1] = x_axis.y;
    m.m[0][2] = x_axis.z;

    m.m[1][0] = y_axis.x;
    m.m[1][1] = y_axis.y;
    m.m[1][2] = y_axis.z;

    m.m[2][0] = z_axis.x;
    m.m[2][1] = z_axis.y;
    m.m[2][2] = z_axis.z;

    m.m[0][3] = -vec3_dot(x_axis, eye);
    m.m[1][3] = -vec3_dot(y_axis, eye);
    m.m[2][3] = -vec3_dot(z_axis, eye);

    return m;
}

/*
 * left, right: the coordinates for the left and right clipping planes
 * bottom, top: the coordinates for the bottom and top clipping planes
 * near, far: the distances to the near and far depth clipping planes
 *
 * 2/(r-l)        0         0  -(r+l)/(r-l)
 *       0  2/(t-b)         0  -(t+b)/(t-b)
 *       0        0  -2/(f-n)  -(f+n)/(f-n)
 *       0        0         0             1
 *
 * see http://docs.gl/gl2/glOrtho
 */
mat4_t mat4_ortho(float left, float right, float bottom, float top,
    float near, float far)
{
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2 / x_range;
    m.m[1][1] = 2 / y_range;
    m.m[2][2] = -2 / z_range;
    m.m[0][3] = -(left + right) / x_range;
    m.m[1][3] = -(bottom + top) / y_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

/*
 * right: the coordinates for the right clipping planes (left == -right)
 * top: the coordinates for the top clipping planes (bottom == -top)
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/r    0         0             0
 *   0  1/t         0             0
 *   0    0  -2/(f-n)  -(f+n)/(f-n)
 *   0    0         0             1
 *
 * this is the same as
 *     float left = -right;
 *     float bottom = -top;
 *     mat4_ortho(left, right, bottom, top, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
mat4_t mat4_orthographic(float right, float top, float near, float far) 
{
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(right > 0 && top > 0 && z_range > 0);
    m.m[0][0] = 1 / right;
    m.m[1][1] = 1 / top;
    m.m[2][2] = -2 / z_range;
    m.m[2][3] = -(near + far) / z_range;
    return m;
}

/*
 * left, right: the coordinates for the left and right clipping planes
 * bottom, top: the coordinates for the bottom and top clipping planes
 * near, far: the distances to the near and far depth clipping planes
 *
 * 2n/(r-l)         0   (r+l)/(r-l)           0
 *        0  2n/(t-b)   (t+b)/(t-b)           0
 *        0         0  -(f+n)/(f-n)  -2fn/(f-n)
 *        0         0            -1           0
 *
 * see http://docs.gl/gl2/glFrustum
 */
mat4_t mat4_frustum(float left, float right, float bottom, float top,
    float near, float far)
{
    float x_range = right - left;
    float y_range = top - bottom;
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(near > 0 && far > 0);
    assert(x_range > 0 && y_range > 0 && z_range > 0);
    m.m[0][0] = 2 * near / x_range;
    m.m[1][1] = 2 * near / y_range;
    m.m[0][2] = (left + right) / x_range;
    m.m[1][2] = (bottom + top) / y_range;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2 * near * far / z_range;
    m.m[3][2] = -1;
    m.m[3][3] = 0;
    return m;
}

/*
 * fovy: the field of view angle in the y direction, in radians
 * aspect: the aspect ratio, defined as width divided by height
 * near, far: the distances to the near and far depth clipping planes
 *
 * 1/(aspect*tan(fovy/2))              0             0           0
 *                      0  1/tan(fovy/2)             0           0
 *                      0              0  -(f+n)/(f-n)  -2fn/(f-n)
 *                      0              0            -1           0
 *
 * this is the same as
 *     float half_h = near * (float)tan(fovy / 2);
 *     float half_w = half_h * aspect;
 *     mat4_frustum(-half_w, half_w, -half_h, half_h, near, far);
 *
 * see http://www.songho.ca/opengl/gl_projectionmatrix.html
 */
mat4_t mat4_perspective(float fovy, float aspect, float near, float far) 
{
    float z_range = far - near;
    mat4_t m = mat4_identity();
    assert(fovy > 0 && aspect > 0);
    assert(near > 0 && far > 0 && z_range > 0);
    m.m[1][1] = 1 / (float)tan(fovy / 2);
    m.m[0][0] = m.m[1][1] / aspect;
    m.m[2][2] = -(near + far) / z_range;
    m.m[2][3] = -2 * near * far / z_range;
    m.m[3][2] = -1;
    m.m[3][3] = 0;
    return m;
}