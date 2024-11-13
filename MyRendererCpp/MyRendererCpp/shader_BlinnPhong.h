#ifndef SHADER_BLINNPHONG_H
#define SHADER_BLINNPHONG_H

#include "maths.h"
#include "mesh.h"
#include "graphics.h"
#include "scene.h"

/* low-level api */

struct attribs_blinnphong 
{
    vec3_t position;
    vec2_t texcoord;
    vec3_t normal;
    vec4_t joint;
    vec4_t weight;
    vec4_t tangent;
};

struct varyings_blinnphong 
{
    vec3_t world_position;
    vec3_t depth_position;
    vec2_t texcoord;
    vec3_t normal;
    vec3_t world_tangent;
    vec3_t world_bitangent;
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
    TGAImage diffuse_map;
    TGAImage specular_map;
    TGAImage emission_map;
    TGAImage normal_map;
    int alpha_blend;
    float shininess;
    float ambient_intensity;
    std::vector<mat4_t> joint_matrices;
    std::vector<mat3_t> joint_n_matrices;
};


struct material_blinnphong 
{
    vec4_t basecolor;
    std::string diffuse_map;
    std::string specular_map;
    std::string emission_map;
    std::string normal_map;
    int alpha_blend;
    float shininess;
};


/* high-level api */
Model* shader_BlinnPhong_create_model(std::string mesh_path, std::string skeleton_path, mat4_t transform, material_blinnphong& material);

vec4_t blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_);

vec4_t blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface);

#endif