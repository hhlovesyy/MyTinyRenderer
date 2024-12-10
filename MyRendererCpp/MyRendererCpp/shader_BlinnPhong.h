#ifndef SHADER_BLINNPHONG_H
#define SHADER_BLINNPHONG_H

#include "maths.h"
#include "mesh.h"
#include "graphics.h"
#include "scene.h"

/* low-level api */

struct attribs_blinnphong 
{
    vec3 position;
    vec2 texcoord;
    vec3 normal;
    vec4 joint;
    vec4 weight;
    vec4 tangent;
};

struct varyings_blinnphong 
{
    vec3 world_position;
    vec3 depth_position;
    vec2 texcoord;
    vec3 normal;
    vec3 world_tangent;
    vec3 world_bitangent;
};

struct uniforms_blinnphong 
{
    vec3 light_dir;
    vec3 camera_pos;
    mat4_t model_matrix;
    mat3_t normal_matrix;
    mat4_t light_vp_matrix;
    mat4_t camera_vp_matrix;
    
    vec4 basecolor;
    Texture diffuse_map;
    Texture specular_map;
    Texture emission_map;
    Texture normal_map;
    
    int alpha_blend;
    float shininess;
    float ambient_intensity;
    bool isDrawShadowMap;
    Texture* shadowmap;
    std::vector<mat4_t> joint_matrices;
    std::vector<mat3_t> joint_n_matrices;
};


struct material_blinnphong 
{
    vec4 basecolor;
    std::string diffuse_map;
    std::string specular_map;
    std::string emission_map;
    std::string normal_map;
    int alpha_blend;
    float shininess;
};


/* high-level api */
Model* shader_BlinnPhong_create_model(std::string mesh_path, std::string skeleton_path, mat4_t transform, material_blinnphong& material);

vec4 blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_);

vec4 blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface);

#endif