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
    vec3 diffuse;
    vec3 specular;
    float alpha;
    float shininess;
    vec3 normal;
    vec3 emission;
};

//返回模型矩阵，将模型从模型空间变换到世界空间
static mat4_t get_model_matrix(attribs_blinnphong* attribs, uniforms_blinnphong* uniforms)
{
    if (uniforms->joint_matrices.size() > 0) {
        mat4_t joint_matrices[4];
        mat4_t skin_matrix;

        joint_matrices[0] = uniforms->joint_matrices[(int)attribs->joint[0]];
        joint_matrices[1] = uniforms->joint_matrices[(int)attribs->joint[1]];
        joint_matrices[2] = uniforms->joint_matrices[(int)attribs->joint[2]];
        joint_matrices[3] = uniforms->joint_matrices[(int)attribs->joint[3]];

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

        joint_n_matrices[0] = uniforms->joint_n_matrices[(int)attribs->joint[0]];
        joint_n_matrices[1] = uniforms->joint_n_matrices[(int)attribs->joint[1]];
        joint_n_matrices[2] = uniforms->joint_n_matrices[(int)attribs->joint[2]];
        joint_n_matrices[3] = uniforms->joint_n_matrices[(int)attribs->joint[3]];

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
    uniforms->light_dir = FrameInfo::light_dir;//vec3_new(0.5f, 0.8f, 0.9f);
    uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*perframe), camera_get_view_matrix(*perframe));
    uniforms->light_vp_matrix = mat4_mul_mat4(FrameInfo::get_light_proj_matrix(), FrameInfo::get_light_view_matrix());
    uniforms->ambient_intensity = FrameInfo::ambient_intensity;
}

static void draw_model(Model* model, framebuffer_t* framebuffer,bool isDrawShadowMap)
{
     Mesh* mesh = model->mesh;
     int num_faces = mesh->getNumFaces();
     const std::vector<Vertex>& vertices = mesh->getVertices();
     Program* program = model->program;

     uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();
     attribs_blinnphong* attribs;
     uniforms->isDrawShadowMap = isDrawShadowMap;
     for (int i = 0; i < num_faces; i++)
     {
         for (int j = 0; j < 3; j++) //遍历每个顶点
         {
             const Vertex& vertex = vertices[i * 3 + j];
             attribs = (attribs_blinnphong*)program_get_attribs(program, j);
         	 attribs->position = vertex.position;  //由于是指针，直接赋值即可
             attribs->normal = vertex.normal;
             attribs->texcoord = vertex.texcoord;
             attribs->tangent = vertex.tangent;
             attribs->joint = vertex.joint;
             attribs->weight = vertex.weight;
         }
         graphics_draw_triangle(framebuffer, program);  //一个一个三角形绘制
     }
     //rasterization_tri(mesh, program, framebuffer,isDrawShadowMap);
     
}


static vec4 common_vertex_shader(attribs_blinnphong* attribs,
    varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms)
{
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat3_t normal_matrix = get_normal_matrix(attribs, uniforms);
    mat4_t camera_vp_matrix = uniforms->camera_vp_matrix;//相机视图矩阵，将世界空间变换到相机空间
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;//光源视图矩阵，将世界空间变换到光源空间

    vec4 input_position = vec4_from_vec3(attribs->position, 1);
    vec4 world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4 clip_position = mat4_mul_vec4(camera_vp_matrix, world_position);//将世界空间变换到相机空间
    vec4 depth_position = mat4_mul_vec4(light_vp_matrix, world_position);//将世界空间变换到光源空间
    //std::cout<<attribs->tangent.x<<","<<attribs->tangent.y<<","<<attribs->tangent.z<<","<<attribs->tangent.w<<std::endl;
    vec3 input_normal = attribs->normal;
    vec3 world_normal = mat3_mul_vec3(normal_matrix, input_normal);
    if (uniforms->normal_map.height > 0)
    {
        mat3_t tangent_matrix = mat3_from_mat4(model_matrix);
        vec3 input_tangent = vec3_from_vec4(attribs->tangent);
        vec3 world_tangent = mat3_mul_vec3(tangent_matrix, input_tangent);
        vec3 world_bitangent;

        world_normal = world_normal.normalized();
        world_tangent = world_tangent.normalized();
        world_bitangent = cross(world_normal, world_tangent);
        world_bitangent = world_bitangent * attribs->tangent[3];

        varyings->normal = world_normal;
        varyings->world_tangent = world_tangent;
        varyings->world_bitangent = world_bitangent;
    }
    else
    {
        varyings->normal = world_normal.normalized();
    }

    varyings->world_position = vec3_from_vec4(world_position);
    varyings->depth_position = vec3_from_vec4(depth_position); //光照空间的depth
    varyings->texcoord = attribs->texcoord;
    
    return clip_position;
}


static vec4 shadow_vertex_shader(attribs_blinnphong* attribs,
    varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms) 
{
    //渲染shdowmap时，只需要计算顶点的在光源视角下的深度（z值）
    mat4_t model_matrix = get_model_matrix(attribs, uniforms);
    mat4_t light_vp_matrix = uniforms->light_vp_matrix;

    vec4 input_position = vec4_from_vec3(attribs->position, 1);
    vec4 world_position = mat4_mul_vec4(model_matrix, input_position);
    vec4 depth_position = mat4_mul_vec4(light_vp_matrix, world_position);

    varyings->texcoord = attribs->texcoord;
    return depth_position;
}

vec4 blinnphong_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    auto* attribs = static_cast<attribs_blinnphong*>(attribs_);
    auto* varyings = static_cast<varyings_blinnphong*>(varyings_);
    auto* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    if(uniforms->isDrawShadowMap==false)
	{
        return common_vertex_shader(attribs, varyings, uniforms);
	}
	else
	{
		return shadow_vertex_shader(attribs, varyings, uniforms);
	}

   
}

static vec3 get_normal_dir(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms, int backface)
{
    //添加法线贴图的相关逻辑
    vec3 normal_dir = varyings->normal;
    if (uniforms->normal_map.height > 0) //用这种方式判断有使用法线贴图
    {
        vec4 sample= sample2D(uniforms->normal_map, varyings->texcoord);
        vec3 tangent_normal = vec3{ sample[0] * 2 - 1,
            sample[1] * 2 - 1,
            sample[2] * 2 - 1 };
        float bump_intensity = 1;
        vec2 tangent_normal_xy = vec2{ tangent_normal[0], tangent_normal[1] };
        tangent_normal_xy = tangent_normal_xy * bump_intensity;
        float new_tangent_z = sqrt(1 - tangent_normal_xy[0] * tangent_normal_xy[0] - tangent_normal_xy[1] * tangent_normal_xy[1]);
        tangent_normal = vec3{ tangent_normal_xy[0], tangent_normal_xy[1], new_tangent_z };
        mat3_t tbn_matrix = mat3_from_cols(varyings->world_tangent,
            varyings->world_bitangent,
            varyings->normal);
        vec3 world_normal = mat3_mul_vec3(tbn_matrix, tangent_normal);
        normal_dir = world_normal.normalized();
    }
    else
    {
        normal_dir = varyings->normal.normalized();
	}
    //vec4_t sample1 = sample2D(uniforms->normal_map, varyings->texcoord);
    //return vec3_from_vec4(sample1);
    return backface ? -normal_dir : normal_dir;
}

static Material_BlinnPhong get_material(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms, int backface)
{
    vec2 texcoord = varyings->texcoord;
    vec3 diffuse, specular, normal, emission;
    float alpha, shininess;
	Material_BlinnPhong material;
    diffuse = vec3_from_vec4(uniforms->basecolor);
    alpha = uniforms->basecolor[3];
    
    vec4 albedo = sample2D(uniforms->diffuse_map, texcoord);
    diffuse = diffuse * vec3_from_vec4(albedo);
    alpha *= albedo[3];

    vec4 specular_map = sample2D(uniforms->specular_map, texcoord);
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

    vec4 emission_map = sample2D(uniforms->emission_map, texcoord);
    emission = vec3_from_vec4(emission_map);

    material.diffuse = diffuse;
    material.specular = specular;
    material.alpha = alpha;
    material.shininess = shininess;
    material.normal = normal;
    material.emission = emission;
    return material;
}

static vec3 get_view_dir(varyings_blinnphong* varyings, uniforms_blinnphong* uniforms)
{
	vec3 view_dir = uniforms->camera_pos - varyings->world_position;
    return view_dir.normalized();
}

static int is_zero_vector(vec3 v) {
    return v[0] == 0 && v[1] == 0 && v[2] == 0;
}

static vec3 get_specular(vec3 light_dir, vec3 view_dir, Material_BlinnPhong material)
{
    return material.specular;
    if (!is_zero_vector(material.specular))
    {
        vec3 half_dir = (light_dir +view_dir).normalized();
        float n_dot_h = dot(material.normal, half_dir);
        if (n_dot_h > 0)
        {
            float strength = (float)pow(n_dot_h, material.shininess);
            return material.specular * strength;
        }
        return vec3{ 0, 0, 0 };
    }
    else
    {
        return vec3{ 0, 0, 0 };
	}
   
}

static int is_in_shadow(varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms, 
    float n_dot_l) 
{
    if (uniforms->shadowmap != nullptr)
    {
        float sampleU = (varyings->depth_position[0] + 1) * 0.5f;
        float sampleV = (varyings->depth_position[1] + 1) * 0.5f;
        sampleV = 1.0 - sampleV; //sample函数会再做一次翻转，所以这里为了逻辑复用提前翻转一次，实际shadow map做的时候是不用翻转的

        //增加bias 避免阴影痤疮
        //float depth_bias = float_max(0.05f * (1 - n_dot_l), 0.005f);
        float depth_bias = float_max(0.01f * (1 - n_dot_l), 0.001f);
        float closest_depth = sample2D(uniforms->shadowmap, vec2{ sampleU, sampleV }, DEFAULT)[0];
        //uniforms->shadowmap->write_texture_to_file("shadowmap11111.tga");

        float current_depth = varyings->depth_position[2] * 0.5 + 0.5;  //depth是-1到1的范围，转换到0到1的范围
        current_depth = current_depth - depth_bias;
        //std::cout<<"current_depth:"<<current_depth<<",closest_depth:"<<closest_depth<< std::endl;

        return current_depth > closest_depth;
    }
    else
    {
        return 0;
    }
}

static vec4 common_fragment_shader(varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms,
    int* discard,
    int backface)  
{
    //sample texture
    Material_BlinnPhong material = get_material(varyings, uniforms, backface);
    vec3 color = material.emission;
    if (uniforms->ambient_intensity > 0)
    {
        vec3 ambient = material.diffuse;  //kd/ka
        float intensity = uniforms->ambient_intensity; //Ia
        color += (ambient * intensity);
    }
    //lambert and blinn_phong
    vec3 light_dir = uniforms->light_dir.normalized(); //注意Light方向要归一化
    
    float n_dot_l = dot(material.normal, light_dir);
    //half lambert
    //n_dot_l = n_dot_l * 0.5 + 0.5;
    if (n_dot_l < 0)
    {
		n_dot_l = 0;
	}
    if (!is_in_shadow(varyings, uniforms, n_dot_l))
    {
        vec3 view_dir = get_view_dir(varyings, uniforms);
        vec3 specular = get_specular(light_dir, view_dir, material);
        vec3 diffuse = material.diffuse * n_dot_l;
        color += (diffuse + specular);
    }
    //vec3_t test = vec3_new(n_dot_l, n_dot_l, n_dot_l);
    
    //diffuse = material.diffuse;
    //vec3_t ndotL = vec3_new(n_dot_l, n_dot_l, n_dot_l);
    return vec4_from_vec3(color , material.alpha);
}

static vec4 shadow_fragment_shader(varyings_blinnphong* varyings,
    uniforms_blinnphong* uniforms,
    int* discard,
    int backface)
{
    return vec4{ 0, 0, 0, 0 };
}

vec4 blinnphong_fragment_shader(void* varyings_, void* uniforms_, int* discard, int backface)
{
    varyings_blinnphong* varyings = static_cast<varyings_blinnphong*>(varyings_);
    uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(uniforms_);

    if (uniforms->isDrawShadowMap == false)
    {
        return common_fragment_shader(varyings, uniforms, discard, backface);
    }
    else 
    {
		return shadow_fragment_shader(varyings, uniforms, discard, backface);
	}
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

