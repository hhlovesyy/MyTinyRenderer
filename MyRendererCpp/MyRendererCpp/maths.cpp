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

vec3 mat3_mul_vec3(mat3_t m, vec3 v) 
{
    float product[3];
    int i;
    for (i = 0; i < 3; i++) 
    {
        float a = m.m[i][0] * v[0];
        float b = m.m[i][1] * v[1];
        float c = m.m[i][2] * v[2];
        product[i] = a + b + c;
    }
    return vec3{ product[0], product[1], product[2] };
}

vec4 vec4_from_vec3(const vec3& v, float w)
{
    vec4 result;
    result[0] = v[0];
    result[1] = v[1];
    result[2] = v[2];
    result[3] = w;
    return result;
}

vec3 vec3_from_vec4(const vec4& v)
{
    vec3 result;
    result[0] = v[0];
    result[1] = v[1];
    result[2] = v[2];
    return result;
}

//叉乘
vec3 cross(const vec3& a, const vec3& b)
{
	vec3 result; // 创建结果向量
	result[0] = a[1] * b[2] - a[2] * b[1]; // 计算 x 分量
	result[1] = a[2] * b[0] - a[0] * b[2]; // 计算 y 分量
	result[2] = a[0] * b[1] - a[1] * b[0]; // 计算 z 分量
	return result; // 返回结果向量
}

vec4 mat4_mul_vec4(mat4_t m, vec4 v) {
    float product[4];
    int i;
    for (i = 0; i < 4; i++) {
        float a = m.m[i][0] * v[0];
        float b = m.m[i][1] * v[1];
        float c = m.m[i][2] * v[2];
        float d = m.m[i][3] * v[3];
        product[i] = a + b + c + d;
    }
    return vec4{ product[0], product[1], product[2], product[3] };
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
    vec3 n = vec3{vx, vy, vz}.normalized();
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();

    m.m[0][0] = n[0] * n[0] * (1 - c) + c;
    m.m[0][1] = n[1] * n[0] * (1 - c) - s * n[2];
    m.m[0][2] = n[2] * n[0] * (1 - c) + s * n[1];

    m.m[1][0] = n[0] * n[1] * (1 - c) + s * n[2];
    m.m[1][1] = n[1] * n[1] * (1 - c) + c;
    m.m[1][2] = n[2] * n[1] * (1 - c) - s * n[0];

    m.m[2][0] = n[0] * n[2] * (1 - c) - s * n[1];
    m.m[2][1] = n[1] * n[2] * (1 - c) + s * n[0];
    m.m[2][2] = n[2] * n[2] * (1 - c) + c;

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
mat4_t mat4_lookat(vec3 eye, vec3 target, vec3 up) 
{
    vec3 z_axis = (eye - target).normalized();
    vec3 x_axis = cross(up, z_axis).normalized();
    vec3 y_axis = cross(z_axis, x_axis);
    mat4_t m = mat4_identity();

    m.m[0][0] = x_axis[0];
    m.m[0][1] = x_axis[1];
    m.m[0][2] = x_axis[2];

    m.m[1][0] = y_axis[0];
    m.m[1][1] = y_axis[1];
    m.m[1][2] = y_axis[2];

    m.m[2][0] = z_axis[0];
    m.m[2][1] = z_axis[1];
    m.m[2][2] = z_axis[2];

    m.m[0][3] = -dot(x_axis, eye);
    m.m[1][3] = -dot(y_axis, eye);
    m.m[2][3] = -dot(z_axis, eye);

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

float float_lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}


/*
 * for spherical linear interpolation, see
 * 3D Math Primer for Graphics and Game Development, 2nd Edition, Chapter 8
 */
quat quat_slerp(quat& a, quat& b, float t) 
{
    float cos_angle = dot(a, b);
    if (cos_angle < 0) {
        b = -b;
        cos_angle = -cos_angle;
    }
    if (cos_angle > 1 - EPSILON) {
        float x = float_lerp(a[0], b[0], t);
        float y = float_lerp(a[1], b[1], t);
        float z = float_lerp(a[2], b[2], t);
        float w = float_lerp(a[3], b[3], t);
        return quat{x, y, z, w};
    }
    else {
        float angle = (float)acos(cos_angle);
        float sin_angle = (float)sin(angle);
        float angle_a = (1 - t) * angle;
        float angle_b = t * angle;
        float factor_a = (float)sin(angle_a) / sin_angle;
        float factor_b = (float)sin(angle_b) / sin_angle;
        float x = factor_a * a[0] + factor_b * b[0];
        float y = factor_a * a[1] + factor_b * b[1];
        float z = factor_a * a[2] + factor_b * b[2];
        float w = factor_a * a[3] + factor_b * b[3];
        return quat{ x, y, z, w };
    }
}

mat4_t mat4_from_trs(vec3& t, quat& r, vec3& s) 
{
    mat4_t translation = mat4_translate(t[0], t[1], t[2]);
    mat4_t rotation = mat4_from_quat(r);
    mat4_t scale = mat4_scale(s[0], s[1], s[2]);
    return mat4_mul_mat4(translation, mat4_mul_mat4(rotation, scale));
}

mat4_t mat4_from_quat(quat& q)
{
    mat4_t m = mat4_identity();
    float xx = q[0] * q[0];
    float xy = q[0] * q[1];
    float xz = q[0] * q[2];
    float xw = q[0] * q[3];
    float yy = q[1] * q[1];
    float yz = q[1] * q[2];
    float yw = q[1] * q[3];
    float zz = q[2] * q[2];
    float zw = q[2] * q[3];

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

mat4_t mat4_combine(mat4_t m[4], vec4& weights_) 
{
    mat4_t combined = { {{0}} };
    float weights[4];
    int i, r, c;

    weights[0] = weights_[0];
    weights[1] = weights_[1];
    weights[2] = weights_[2];
    weights[3] = weights_[3];

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

mat3_t mat3_combine(mat3_t m[4], vec4& weights_) {
    mat3_t combined = { {{0}} };
    float weights[4];
    int i, r, c;

    weights[0] = weights_[0];
    weights[1] = weights_[1];
    weights[2] = weights_[2];
    weights[3] = weights_[3];

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

float float_saturate(float f) {
    return f < 0 ? 0 : (f > 1 ? 1 : f);
}

vec4 vec4_saturate(vec4 v) {
    float x = float_saturate(v[0]);
    float y = float_saturate(v[1]);
    float z = float_saturate(v[2]);
    float w = float_saturate(v[3]);
    return vec4{ x, y, z, w };
}

mat3_t mat3_from_cols(vec3 c0, vec3 c1, vec3 c2) {
    mat3_t m;
    m.m[0][0] = c0[0];
    m.m[1][0] = c0[1];
    m.m[2][0] = c0[2];
    m.m[0][1] = c1[0];
    m.m[1][1] = c1[1];
    m.m[2][1] = c1[2];
    m.m[0][2] = c2[0];
    m.m[1][2] = c2[1];
    m.m[2][2] = c2[2];
    return m;
}

float float_linear2srgb(float value) {
    return (float)pow(value, 1 / 2.2);
}

/*
 * for aces filmic tone mapping curve, see
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
 */
float float_aces(float value) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    value = (value * (a * value + b)) / (value * (c * value + d) + e);
    return float_saturate(value);
}
