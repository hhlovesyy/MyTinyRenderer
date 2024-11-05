#include "maths.h"
#include <assert.h>
#include "macro.h"

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
float float_from_uchar(unsigned char value)
{
    return value / 255.0f;
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

vec3_t mat3_mul_vec3(mat3_t m, vec3_t v) 
{
    float product[3];
    int i;
    for (i = 0; i < 3; i++) 
    {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        product[i] = a + b + c;
    }
    return vec3_new(product[0], product[1], product[2]);
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

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
    float product[4];
    int i;
    for (i = 0; i < 4; i++) {
        float a = m.m[i][0] * v.x;
        float b = m.m[i][1] * v.y;
        float c = m.m[i][2] * v.z;
        float d = m.m[i][3] * v.w;
        product[i] = a + b + c + d;
    }
    return vec4_new(product[0], product[1], product[2], product[3]);
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
    mat4_t m = { {{0}} };
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            for (k = 0; k < 4; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}

vec4_t vec4_div(vec4_t v, float divisor)
{
    return vec4_mul(v, 1 / divisor);
}

vec4_t vec4_mul_vec4(vec4_t a, vec4_t b)
{
	return vec4_new(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
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

vec3_t vec3_lerp(vec3_t a, vec3_t b, float t)
{
    float x = float_lerp(a.x, b.x, t);
    float y = float_lerp(a.y, b.y, t);
    float z = float_lerp(a.z, b.z, t);
    return vec3_new(x, y, z);
}

float float_lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

quat_t quat_new(float x, float y, float z, float w) 
{
    quat_t q;
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
    return q;
}

float quat_dot(quat_t a, quat_t b) 
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/*
 * for spherical linear interpolation, see
 * 3D Math Primer for Graphics and Game Development, 2nd Edition, Chapter 8
 */
quat_t quat_slerp(quat_t a, quat_t b, float t) 
{
    float cos_angle = quat_dot(a, b);
    if (cos_angle < 0) {
        b = quat_new(-b.x, -b.y, -b.z, -b.w);
        cos_angle = -cos_angle;
    }
    if (cos_angle > 1 - EPSILON) {
        float x = float_lerp(a.x, b.x, t);
        float y = float_lerp(a.y, b.y, t);
        float z = float_lerp(a.z, b.z, t);
        float w = float_lerp(a.w, b.w, t);
        return quat_new(x, y, z, w);
    }
    else {
        float angle = (float)acos(cos_angle);
        float sin_angle = (float)sin(angle);
        float angle_a = (1 - t) * angle;
        float angle_b = t * angle;
        float factor_a = (float)sin(angle_a) / sin_angle;
        float factor_b = (float)sin(angle_b) / sin_angle;
        float x = factor_a * a.x + factor_b * b.x;
        float y = factor_a * a.y + factor_b * b.y;
        float z = factor_a * a.z + factor_b * b.z;
        float w = factor_a * a.w + factor_b * b.w;
        return quat_new(x, y, z, w);
    }
}

mat4_t mat4_from_trs(vec3_t t, quat_t r, vec3_t s) 
{
    mat4_t translation = mat4_translate(t.x, t.y, t.z);
    mat4_t rotation = mat4_from_quat(r);
    mat4_t scale = mat4_scale(s.x, s.y, s.z);
    return mat4_mul_mat4(translation, mat4_mul_mat4(rotation, scale));
}

mat4_t mat4_from_quat(quat_t q)
{
    mat4_t m = mat4_identity();
    float xx = q.x * q.x;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float xw = q.x * q.w;
    float yy = q.y * q.y;
    float yz = q.y * q.z;
    float yw = q.y * q.w;
    float zz = q.z * q.z;
    float zw = q.z * q.w;

    m.m[0][0] = 1 - 2 * (yy + zz);
    m.m[0][1] = 2 * (xy - zw);
    m.m[0][2] = 2 * (xz + yw);

    m.m[1][0] = 2 * (xy + zw);
    m.m[1][1] = 1 - 2 * (xx + zz);
    m.m[1][2] = 2 * (yz - xw);

    m.m[2][0] = 2 * (xz - yw);
    m.m[2][1] = 2 * (yz + xw);
    m.m[2][2] = 1 - 2 * (xx + yy);

    return m;
}

/*
 * for determinant, adjoint, and inverse, see
 * 3D Math Primer for Graphics and Game Development, 2nd Edition, Chapter 6
 */

static float mat3_determinant(mat3_t m) {
    float a = +m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]);
    float b = -m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]);
    float c = +m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
    return a + b + c;
}

mat3_t mat3_inverse_transpose(mat3_t m)
{
    mat3_t adjoint, inverse_transpose;
    float determinant, inv_determinant;
    int i, j;

    adjoint = mat3_adjoint(m);
    determinant = mat3_determinant(m);
    inv_determinant = 1 / determinant;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            inverse_transpose.m[i][j] = adjoint.m[i][j] * inv_determinant;
        }
    }
    return inverse_transpose;
}

static mat3_t mat3_adjoint(mat3_t m) {
    mat3_t adjoint;
    adjoint.m[0][0] = +(m.m[1][1] * m.m[2][2] - m.m[2][1] * m.m[1][2]);
    adjoint.m[0][1] = -(m.m[1][0] * m.m[2][2] - m.m[2][0] * m.m[1][2]);
    adjoint.m[0][2] = +(m.m[1][0] * m.m[2][1] - m.m[2][0] * m.m[1][1]);
    adjoint.m[1][0] = -(m.m[0][1] * m.m[2][2] - m.m[2][1] * m.m[0][2]);
    adjoint.m[1][1] = +(m.m[0][0] * m.m[2][2] - m.m[2][0] * m.m[0][2]);
    adjoint.m[1][2] = -(m.m[0][0] * m.m[2][1] - m.m[2][0] * m.m[0][1]);
    adjoint.m[2][0] = +(m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2]);
    adjoint.m[2][1] = -(m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2]);
    adjoint.m[2][2] = +(m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1]);
    return adjoint;
}

mat3_t mat3_from_mat4(mat4_t m) {
    mat3_t n;
    n.m[0][0] = m.m[0][0];
    n.m[0][1] = m.m[0][1];
    n.m[0][2] = m.m[0][2];
    n.m[1][0] = m.m[1][0];
    n.m[1][1] = m.m[1][1];
    n.m[1][2] = m.m[1][2];
    n.m[2][0] = m.m[2][0];
    n.m[2][1] = m.m[2][1];
    n.m[2][2] = m.m[2][2];
    return n;
}

mat4_t mat4_combine(mat4_t m[4], vec4_t weights_) 
{
    mat4_t combined = { {{0}} };
    float weights[4];
    int i, r, c;

    weights[0] = weights_.x;
    weights[1] = weights_.y;
    weights[2] = weights_.z;
    weights[3] = weights_.w;

    for (i = 0; i < 4; i++) {
        float weight = weights[i];
        if (weight > 0) {
            mat4_t source = m[i];
            for (r = 0; r < 4; r++) {
                for (c = 0; c < 4; c++) {
                    combined.m[r][c] += weight * source.m[r][c];
                }
            }
        }
    }

    return combined;
}

mat3_t mat3_combine(mat3_t m[4], vec4_t weights_) {
    mat3_t combined = { {{0}} };
    float weights[4];
    int i, r, c;

    weights[0] = weights_.x;
    weights[1] = weights_.y;
    weights[2] = weights_.z;
    weights[3] = weights_.w;

    for (i = 0; i < 4; i++) {
        float weight = weights[i];
        if (weight > 0) {
            mat3_t source = m[i];
            for (r = 0; r < 3; r++) {
                for (c = 0; c < 3; c++) {
                    combined.m[r][c] += weight * source.m[r][c];
                }
            }
        }
    }

    return combined;
}

mat3_t mat3_mul_mat3(mat3_t a, mat3_t b) {
    mat3_t m = { {{0}} };
    int i, j, k;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                m.m[i][j] += a.m[i][k] * b.m[k][j];
            }
        }
    }
    return m;
}

vec3_t vec3_modulate(vec3_t a, vec3_t b) {
    return vec3_new(a.x * b.x, a.y * b.y, a.z * b.z);
}

vec3_t vec3_negate(vec3_t v) {
    return vec3_new(-v.x, -v.y, -v.z);
}
float float_saturate(float f) {
    return f < 0 ? 0 : (f > 1 ? 1 : f);
}

vec4_t vec4_saturate(vec4_t v) {
    float x = float_saturate(v.x);
    float y = float_saturate(v.y);
    float z = float_saturate(v.z);
    float w = float_saturate(v.w);
    return vec4_new(x, y, z, w);
}