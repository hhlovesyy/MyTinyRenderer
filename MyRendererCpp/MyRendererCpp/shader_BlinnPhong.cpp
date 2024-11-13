#include <cmath>
#include <cstdlib>
#include <vector>
#include "shader_BlinnPhong.h"
#include "texture2D.h"
#include "skeleton.h"
#include "scene.h"
#include "rasterization.h"
#include <iostream>

class Material_BlinnPhong
{
public:
    vec3_t diffuse;
    vec3_t specular;
    float alpha;
    float shininess;
    vec3_t normal;
    vec3_t emission;
};

//返回模型矩阵，将模型从模型空间变换到世界空间
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
//返回法线矩阵，将法线从模型空间变换到世界空间
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
    //todo:如果有类似于find_bbox的函数，可能也要更新骨骼的bbox，参考skeleton_update_joints的调用时机
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
    uniforms->light_dir = vec3_new(0.5f, 0.8f, 0.9f);
    uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*perframe), camera_get_view_matrix(*perframe));
    uniforms->ambient_intensity = FrameInfo::ambient_intensity;
}

static void draw_model(Model* model, framebuffer_t* framebuffer)
{
     Mesh* mesh = model->mesh;
     Program* program = model->program;
     uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();

     rasterization_tri(mesh, program, framebuffer);
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
    //std::cout<<attribs->tangent.x<<","<<attribs->tangent.y<<","<<attribs->tangent.z<<","<<attribs->tangent.w<<std::endl;
    vec3_t input_normal = attribs->normal;
    vec3_t world_normal = mat3_mul_vec3(normal_matrix, input_normal);
    if (uniforms->normal_map.height() > 0)
    {
        mat3_t tangent_matrix = mat3_from_mat4(model_matrix);
        vec3_t input_tangent = vec3_from_vec4(attribs->tangent);
        vec3_t world_tangent = mat3_mul_vec3(tangent_matrix, input_tangent);
        vec3_t world_bitangent;

        world_normal = vec3_normalize(world_normal);
        world_tangent = vec3_normalize(world_tangent);
        world_bitangent = vec3_cross(world_normal, world_tangent);
        world_bitangent = vec3_mul(world_bitangent, attribs->tangent.w);

        varyings->normal = world_normal;
        varyings->world_tangent = world_tangent;
        varyings->world_bitangent = world_bitangent;
    }
    else
    {
        varyings->normal = vec3_normalize(world_normal);
    }

    varyings->world_position = vec3_from_vec4(world_position);
    varyings->depth_position = vec3_from_vec4(depth_position);
    varyings->texcoord = attribs->texcoord;
    
    return clip_position;
}

vec4_t blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    auto* attribs = static_cast<attribs_blinnphong*>(attribs_);
    auto* varyings = static_cast<varyings_blinnphong*>(varyings_);
    auto* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    return common_vertex_shader(attribs, varyings, uniforms);
}

static vec3_t get_normal_dir(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms, int backface)
{
    //添加法线贴图的相关逻辑
    vec3_t normal_dir = varyings->normal;
    if (uniforms->normal_map.height() > 0) //用这种方式判断有使用法线贴图
    {
        vec4_t sample= sample2D(uniforms->normal_map, varyings->texcoord);
        vec3_t tangent_normal = vec3_new(sample.x * 2 - 1,
            sample.y * 2 - 1,
            sample.z * 2 - 1);
        float bump_intensity = 1;
        vec2_t tangent_normal_xy = vec2_new(tangent_normal.x, tangent_normal.y);
        tangent_normal_xy = vec2_mul(tangent_normal_xy, bump_intensity);
        float new_tangent_z = sqrt(1 - tangent_normal_xy.x * tangent_normal_xy.x - tangent_normal_xy.y * tangent_normal_xy.y);
        tangent_normal = vec3_new(tangent_normal_xy.x, tangent_normal_xy.y, new_tangent_z);
        mat3_t tbn_matrix = mat3_from_cols(varyings->world_tangent,
            varyings->world_bitangent,
            varyings->normal);
        vec3_t world_normal = mat3_mul_vec3(tbn_matrix, tangent_normal);
        normal_dir = vec3_normalize(world_normal);
    }
    else
    {
        normal_dir = vec3_normalize(varyings->normal);
	}
    //vec4_t sample1 = sample2D(uniforms->normal_map, varyings->texcoord);
    //return vec3_from_vec4(sample1);
    return backface ? vec3_negate(normal_dir) : normal_dir;
}

static Material_BlinnPhong get_material(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms, int backface)
{
    vec2_t texcoord = varyings->texcoord;
    vec3_t diffuse, specular, normal, emission;
    float alpha, shininess;
	Material_BlinnPhong material;
    diffuse = vec3_from_vec4(uniforms->basecolor);
    alpha = uniforms->basecolor.w;
    
    vec4_t albedo = sample2D(uniforms->diffuse_map, texcoord);
    diffuse = vec3_modulate(diffuse, vec3_from_vec4(albedo));
    alpha *= albedo.w;

    vec4_t specular_map = sample2D(uniforms->specular_map, texcoord);
    //std::cout<<"specular_map:"<<specular_map.x<<","<<specular_map.y<<","<<specular_map.z<<","<<specular_map.w<<std::endl;
    specular = vec3_from_vec4(specular_map);
    //todo:test uniform specular value
    //specular = vec3_new(0.5, 0.5, 0.5);
    shininess = uniforms->shininess;
    /*normal = varyings->normal;
    if (backface) {
        normal = vec3_negate(normal);
    }*/
    normal = get_normal_dir(varyings, uniforms, backface);

    vec4_t emission_map = sample2D(uniforms->emission_map, texcoord);
    emission = vec3_from_vec4(emission_map);

    material.diffuse = diffuse;
    material.specular = specular;
    material.alpha = alpha;
    material.shininess = shininess;
    material.normal = normal;
    material.emission = emission;
    return material;
}

static vec3_t get_view_dir(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms)
{
	vec3_t view_dir = vec3_sub(uniforms->camera_pos, varyings->world_position);
	return vec3_normalize(view_dir);
}

static int is_zero_vector(vec3_t v) {
    return v.x == 0 && v.y == 0 && v.z == 0;
}

static vec3_t get_specular(vec3_t light_dir, vec3_t view_dir, Material_BlinnPhong material)
{
    return material.specular;
    if (!is_zero_vector(material.specular))
    {
        vec3_t half_dir = vec3_normalize(vec3_add(light_dir, view_dir));
        float n_dot_h = vec3_dot(material.normal, half_dir);
        if (n_dot_h > 0)
        {
            float strength = (float)pow(n_dot_h, material.shininess);
            return vec3_mul(material.specular, strength);
        }
        return vec3_new(0, 0, 0);
    }
    else
    {
		return vec3_new(0, 0, 0);
	}
   
}

static vec4_t common_fragment_shader(varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms,
    int* discard,
    int backface)  
{
    //sample texture
    Material_BlinnPhong material = get_material(varyings, uniforms, backface);
    vec3_t color = material.emission;
    if (uniforms->ambient_intensity > 0)
    {
        vec3_t ambient = material.diffuse;  //kd/ka
        float intensity = uniforms->ambient_intensity; //Ia
        color = vec3_add(color, vec3_mul(ambient, intensity));
    }
    //lambert and blinn_phong
    vec3_t light_dir = vec3_normalize(uniforms->light_dir); //注意Light方向要归一化
    
    float n_dot_l = vec3_dot(material.normal, light_dir);
    //half lambert
    //n_dot_l = n_dot_l * 0.5 + 0.5;
    if (n_dot_l < 0)
    {
		n_dot_l = 0;
	}
    //vec3_t test = vec3_new(n_dot_l, n_dot_l, n_dot_l);
    vec3_t view_dir = get_view_dir(varyings, uniforms);
    vec3_t specular = get_specular(light_dir, view_dir, material);
    vec3_t diffuse = vec3_mul(material.diffuse, n_dot_l);
    color = vec3_add(color, vec3_add(diffuse, specular));
    //diffuse = material.diffuse;
    vec3_t ndotL = vec3_new(n_dot_l, n_dot_l, n_dot_l);
    return vec4_from_vec3(color , material.alpha);
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
    model->mesh = mesh; //todo:可能是性能瓶颈之一，因为没有cached，每次都会重新加载，如果太慢了需要优化
    model->transform = transform;
    model->transparent = material.alpha_blend;//如果允许透明度混合，则认为是透明物体
    model->skeleton = skeleton;
    model->update = update_model;
    model->draw = draw_model;

    uniforms->diffuse_map = mesh->load_texture(material.diffuse_map);
    uniforms->specular_map = mesh->load_texture(material.specular_map);
    uniforms->emission_map = mesh->load_texture(material.emission_map);
    uniforms->normal_map = mesh->load_texture(material.normal_map);
    uniforms->shininess = material.shininess;
    uniforms->basecolor = material.basecolor;
    uniforms->alpha_blend = material.alpha_blend;
    uniforms->model_matrix = transform;

    return model;
}

