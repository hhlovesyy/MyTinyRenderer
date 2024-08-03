#ifndef MATHS_H
#define MATHS_H

typedef struct { float x, y; } vec2_t;
typedef struct { float x, y, z; } vec3_t;
typedef struct { float x, y, z, w; } vec4_t;

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

#endif