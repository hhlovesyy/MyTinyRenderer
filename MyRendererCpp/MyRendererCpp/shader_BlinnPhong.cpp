#include <cmath>
#include <cstdlib>
#include <vector>
#include "shader_BlinnPhong.h"
#include "texture2D.h"
#include "skeleton.h"
#include "scene.h"
#include <iostream>

//����ģ�;��󣬽�ģ�ʹ�ģ�Ϳռ�任������ռ�
static mat4_t get_model_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    if (uniforms->joint_matrices.size() > 0) {
        mat4_t joint_matrices[4];
        mat4_t skin_matrix;

        joint_matrices[0] = uniforms->joint_matrices[(int)attribs->joint.x];
        joint_matrices[1] = uniforms->joint_matrices[(int)attribs->joint.y];
        joint_matrices[2] = uniforms->joint_matrices[(int)attribs->joint.z];
        joint_matrices[3] = uniforms->joint_matrices[(int)attribs->joint.w];

        skin_matrix = mat4_combine(joint_matrices, attribs->weight);
        return mat4_mul_mat4(uniforms->model_matrix, skin_matrix);
    }
    else 
    {
        return uniforms->model_matrix;
    }
}
//���ط��߾��󣬽����ߴ�ģ�Ϳռ�任������ռ�
static mat3_t get_normal_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    if (uniforms->joint_n_matrices.size() > 0) 
    {
        mat3_t joint_n_matrices[4];
        mat3_t skin_n_matrix;

        joint_n_matrices[0] = uniforms->joint_n_matrices[(int)attribs->joint.x];
        joint_n_matrices[1] = uniforms->joint_n_matrices[(int)attribs->joint.y];
        joint_n_matrices[2] = uniforms->joint_n_matrices[(int)attribs->joint.z];
        joint_n_matrices[3] = uniforms->joint_n_matrices[(int)attribs->joint.w];

        skin_n_matrix = mat3_combine(joint_n_matrices, attribs->weight);
        return mat3_mul_mat3(uniforms->normal_matrix, skin_n_matrix);
    }
    else 
    {
        return uniforms->normal_matrix;
    }
}

static void update_model(Model* model, Camera* perframe)
{
    //todo:�����������find_bbox�ĺ���������ҲҪ���¹�����bbox���ο�skeleton_update_joints�ĵ���ʱ��
    Skeleton* skeleton = model->skeleton;
    mat4_t model_matrix = model->transform;
    mat3_t normal_matrix;
    std::vector<mat4_t> joint_matrices;
    std::vector<mat3_t> joint_n_matrices;
    uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(model->program->get_uniforms());

    if (skeleton)
    {
        skeleton_update_joints(skeleton, FrameInfo::get_frame_time());
        joint_matrices = skeleton->joint_matrices;
        joint_n_matrices = skeleton->normal_matrices;
    }
    normal_matrix = mat3_inverse_transpose(mat3_from_mat4(model_matrix));
    uniforms->camera_pos = perframe->position;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->joint_matrices = joint_matrices;
    uniforms->joint_n_matrices = joint_n_matrices;
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
    //half-lambert ��������
    n_dot_l = n_dot_l * 0.5 + 0.5;
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

Model* shader_BlinnPhong_create_model(std::string mesh_path,std::string skeleton_path, mat4_t transform, material_blinnphong& material)
{
    int sizeof_attribs = sizeof(attribs_blinnphong);
    int sizeof_varyings = sizeof(varyings_blinnphong);
    int sizeof_uniforms = sizeof(uniforms_blinnphong);
    uniforms_blinnphong* uniforms;
    Program* program;
    

    program = new Program(blinnphong_vertex_shader, blinnphong_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms,material.alpha_blend);

    uniforms = static_cast<uniforms_blinnphong*>(program->get_uniforms());

    Mesh* mesh = Mesh::load(mesh_path);
    if (mesh == nullptr)
    {
        std::cerr << "Failed to load model: " << (mesh_path) << std::endl;
        return nullptr;
    }
    Model* model = new Model();
    Skeleton* skeleton = Skeleton::load(skeleton_path);
    model->program = program;
    model->mesh = mesh; //todo:����������ƿ��֮һ����Ϊû��cached��ÿ�ζ������¼��أ����̫������Ҫ�Ż�
    model->transform = transform;
    model->transparent = material.alpha_blend;//�������͸���Ȼ�ϣ�����Ϊ��͸������
    model->skeleton = skeleton;
    model->update = update_model;

    uniforms->diffuse_map = mesh->load_texture(material.diffuse_map);
    uniforms->basecolor = material.basecolor;
    uniforms->alpha_blend = material.alpha_blend;
    uniforms->model_matrix = transform;

    return model;
}

