#include <cmath>
#include <cstdlib>
#include <vector>
#include "shader_BlinnPhong.h"

//返回模型矩阵，将模型从模型空间变换到世界空间
static mat4_t get_model_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    return uniforms->model_matrix;
}
//返回法线矩阵，将法线从模型空间变换到世界空间
static mat3_t get_normal_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    return uniforms->normal_matrix;
}


static vec4_t common_vertex_shader(attribs_blinnphong* attribs,
    varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms)
{
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat3_t normal_matrix = get_normal_matrix(attribs, uniforms);
    mat4_t camera_vp_matrix = uniforms->camera_vp_matrix;//相机视图矩阵，将世界空间变换到相机空间
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;//光源视图矩阵，将世界空间变换到光源空间

    vec4_t input_position = vec4_from_vec3(attribs->position, 1);
    vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4_t clip_position = mat4_mul_vec4(camera_vp_matrix, world_position);//将世界空间变换到相机空间
    vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);//将世界空间变换到光源空间

    vec3_t input_normal = attribs->normal;
    vec3_t world_normal = mat3_mul_vec3(normal_matrix, input_normal);

    varyings->world_position = vec3_from_vec4(world_position);
    varyings->depth_position = vec3_from_vec4(depth_position);
    varyings->texcoord = attribs->texcoord;
    varyings->normal = vec3_normalize(world_normal);
    return clip_position;
}

vec4_t blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    auto* attribs = static_cast<attribs_blinnphong*>(attribs_);
    auto* varyings = static_cast<varyings_blinnphong*>(varyings_);
    auto* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    return common_vertex_shader(attribs, varyings, uniforms);
}


static vec4_t common_fragment_shader(varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms,
    int* discard,
    int backface)  
{
    /*material_t material = get_material(varyings, uniforms, backface);
        vec3_t color = material.emission;*/

        //vec3_t ambient = material.diffuse;
        //float intensity = uniforms->ambient_intensity;
        //color = vec3_add(color, vec3_mul(ambient, intensity));


        //先test
        /*vec3_t ambient = 
        float intensity = uniforms->ambient_intensity;
        color = vec3_add(color, vec3_mul(ambient, intensity));*/

        //if (uniforms->punctual_intensity > 0) 
        //{
        //    vec3_t light_dir = vec3_negate(uniforms->light_dir);
        //    float n_dot_l = vec3_dot(material.normal, light_dir);
        //    if (n_dot_l > 0 && !is_in_shadow(varyings, uniforms, n_dot_l)) {
        //        vec3_t view_dir = get_view_dir(varyings, uniforms);
        //        vec3_t specular = get_specular(light_dir, view_dir, material);
        //        vec3_t diffuse = vec3_mul(material.diffuse, n_dot_l);
        //        vec3_t punctual = vec3_add(diffuse, specular);
        //        float intensity = uniforms->punctual_intensity;
        //        color = vec3_add(color, vec3_mul(punctual, intensity));
        //    }
        //}

        //实现一个简单的blinn-phong模型
        vec3_t light_dir = vec3_normalize(uniforms->light_dir);
        vec3_t normal = varyings->normal;
        float n_dot_l = vec3_dot(normal, light_dir);
        if (n_dot_l < 0) n_dot_l = 0;
        vec4_t diffuse = vec4_mul(uniforms->basecolor, n_dot_l);


        return diffuse;
        
}

vec4_t blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface)
{
    varyings_blinnphong* varyings = static_cast<varyings_blinnphong*>(varyings_);
    uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    return common_fragment_shader(varyings, uniforms, discard, backface);
}

void shader_BlinnPhong_create_model(std::string mesh, mat4_t transform)
{
    int sizeof_attribs = sizeof(attribs_blinnphong);
    int sizeof_varyings = sizeof(varyings_blinnphong);
    int sizeof_uniforms = sizeof(uniforms_blinnphong);
    uniforms_blinnphong* uniforms;
    Program* program;

    program = new Program(blinnphong_vertex_shader, blinnphong_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms);

    uniforms = static_cast<uniforms_blinnphong*>(program->get_uniforms());

    uniforms->diffuse_map = "_diffuse.tga";

    return;
}

