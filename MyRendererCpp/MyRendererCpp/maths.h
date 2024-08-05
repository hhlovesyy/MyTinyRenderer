#ifndef MATHS_H
#define MATHS_H
#include <math.h>

#define TO_RADIANS(degrees) ((PI / 180) * (degrees))
#define TO_DEGREES(radians) ((180 / PI) * (radians))
#define PI 3.1415927f
typedef struct { float x, y; } vec2_t;
typedef struct { float x, y, z; } vec3_t;
typedef struct { float x, y, z, w; } vec4_t;

typedef struct { float m[3][3]; } mat3_t;
typedef struct { float m[4][4]; } mat4_t;

float float_min(float a, float b);
float float_max(float a, float b);

float float_clamp(float f, float min, float max);
int min_integer(int a, int b);

int max_integer(int a, int b);
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

mat4_t mat4_identity(void);
/* transformation matrices */
mat4_t mat4_translate(float tx, float ty, float tz);

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
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

mat4_t mat4_lookat(vec3_t eye, vec3_t target, vec3_t up);
mat4_t mat4_ortho(float left, float right, float bottom, float top, float near, float far);
mat4_t mat4_orthographic(float right, float top, float near, float far);

mat4_t mat4_frustum(float left, float right, float bottom, float top,
	float near, float far);
mat4_t mat4_perspective(float fovy, float aspect, float near, float far);

#endif