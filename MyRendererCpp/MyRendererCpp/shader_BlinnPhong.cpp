#include <cmath>
#include <cstdlib>
#include <vector>
#include "shader_BlinnPhong.h"
#include "texture2D.h"
#include <iostream>

//����ģ�;��󣬽�ģ�ʹ�ģ�Ϳռ�任������ռ�
static mat4_t get_model_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    return uniforms->model_matrix;
}
//���ط��߾��󣬽����ߴ�ģ�Ϳռ�任������ռ�
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
    mat4_t camera_vp_matrix = uniforms->camera_vp_matrix;//�����ͼ���󣬽�����ռ�任������ռ�
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;//��Դ��ͼ���󣬽�����ռ�任����Դ�ռ�

    vec4_t input_position = vec4_from_vec3(attribs->position, 1);
    vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4_t clip_position = mat4_mul_vec4(camera_vp_matrix, world_position);//������ռ�任������ռ�
    vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);//������ռ�任����Դ�ռ�

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
    //sample texture
    vec2_t texcoord = varyings->texcoord;
    vec4_t albedo = sample2D(uniforms->diffuse_map, texcoord);

    //ʵ��һ���򵥵�blinn-phongģ��
    vec3_t light_dir = vec3_normalize(uniforms->light_dir);
    vec3_t normal = varyings->normal;
    float n_dot_l = vec3_dot(normal, light_dir);
    if (n_dot_l < 0) n_dot_l = 0;
    vec4_t diffuse = vec4_mul(uniforms->basecolor, n_dot_l);

    diffuse = vec4_mul_vec4(diffuse, albedo);
    //diffuse = vec4_t{ 1, 1, 1, 1 };

    return diffuse;
        
}

vec4_t blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface)
{
    varyings_blinnphong* varyings = static_cast<varyings_blinnphong*>(varyings_);
    uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    return common_fragment_shader(varyings, uniforms, discard, backface);
}

Model* shader_BlinnPhong_create_model(std::string mesh_path, mat4_t transform, material_blinnphong& material)
{
    int sizeof_attribs = sizeof(attribs_blinnphong);
    int sizeof_varyings = sizeof(varyings_blinnphong);
    int sizeof_uniforms = sizeof(uniforms_blinnphong);
    uniforms_blinnphong* uniforms;
    Program* program;
    

    program = new Program(blinnphong_vertex_shader, blinnphong_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms);

    uniforms = static_cast<uniforms_blinnphong*>(program->get_uniforms());

    Mesh* mesh = Mesh::load(mesh_path);
    if (mesh == nullptr)
    {
        std::cerr << "Failed to load model: " << (mesh_path) << std::endl;
        return nullptr;
    }
    Model* model = new Model();
    model->program = program;
    model->mesh = mesh;
    model->transform = transform;

    uniforms->diffuse_map = mesh->load_texture(material.diffuse_map);
    uniforms->basecolor = material.basecolor;
    uniforms->model_matrix = transform;

    return model;
}
