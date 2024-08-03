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
int min_integer(int a, int b);

int max_integer(int a, int b);
unsigned char float_to_uchar(float value);
vec2_t vec2_sub(vec2_t a, vec2_t b);
vec2_t vec2_new(float x, float y);
vec3_t vec3_new(float x, float y, float z);
vec2_t vec2_min(vec2_t a, vec2_t b);
vec2_t vec2_max(vec2_t a, vec2_t b);

vec4_t vec4_new(float x, float y, float z, float w);

mat4_t mat4_identity(void);
/* transformation matrices */
mat4_t mat4_translate(float tx, float ty, float tz);

vec4_t mat4_mul_vec4(mat4_t& m, vec4_t& v);
mat4_t mat4_rotate_x(float angle);
mat4_t mat4_rotate_y(float angle);
mat4_t mat4_rotate_z(float angle);
mat4_t mat4_scale(float sx, float sy, float sz);
#endif