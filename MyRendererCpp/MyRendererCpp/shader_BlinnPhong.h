#ifndef SHADER_BLINNPHONG_H
#define SHADER_BLINNPHONG_H

#include "maths.h"
#include "mesh.h"
#include "graphics.h"
/* low-level api */

struct attribs_blinnphong 
{
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
};

struct varyings_blinnphong 
{
    vec3_t world_position;
    vec3_t depth_position;
    vec2_t texcoord;
    vec3_t normal;
};

struct uniforms_blinnphong 
{
    vec3_t light_dir;
    vec3_t camera_pos;
    mat4_t model_matrix;
    mat3_t normal_matrix;
    mat4_t light_vp_matrix;
    mat4_t camera_vp_matrix;
    
    vec4_t basecolor;
    std::string diffuse_map;
};


struct material_blinnphong 
{
    vec4_t basecolor;
    std::string diffuse_map;
};


/* high-level api */
void shader_BlinnPhong_create_model(std::string mesh, mat4_t transform);

vec4_t blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_);

vec4_t blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface);

#endif