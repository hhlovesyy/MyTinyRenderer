#include "shader_skybox.h"

vec4_t skybox_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    auto* attribs = static_cast<attribs_skybox*>(attribs_);
    auto* varyings = static_cast<varyings_skybox*>(varyings_);
    auto* uniforms = static_cast<uniforms_skybox*>(uniforms_);
    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->vp_matrix, local_pos);  //转到裁剪空间
    clip_pos.z = clip_pos.w * (1 - EPSILON); //稍微有个小的bias，防止精度误差
    //std::cout<<clip_pos.x<<" "<<clip_pos.y<<" "<<clip_pos.z<<" "<<clip_pos.w<<std::endl;
    varyings->direction = attribs->position;
    return clip_pos;
}

vec4_t skybox_fragment_shader(void* varys, void* unis,
    int* discard, int backface)
{
    auto* varyings = static_cast<varyings_skybox*>(varys);
    auto* uniforms = static_cast<uniforms_skybox*>(unis);
    //return vec4_t{ 0,0,1,1 };
   
    //assert(uniforms->skybox != nullptr);
    return cubemap_sample(uniforms->skybox, varyings->direction);
}

static void update_skybox_model(std::shared_ptr<Model> model, Camera* perframe)
{
    mat4_t view_matrix = camera_get_view_matrix(*perframe);
    mat4_t proj_matrix = camera_get_proj_matrix(*perframe);
    uniforms_skybox* uniforms = (uniforms_skybox*)model->program->shader_uniforms_;

    //移除掉skybox的位移
    view_matrix.m[0][3] = 0;
    view_matrix.m[1][3] = 0;
    view_matrix.m[2][3] = 0;

    uniforms->vp_matrix = mat4_mul_mat4(proj_matrix, view_matrix);
}

static void draw_skybox_model(std::shared_ptr<Model> model, framebuffer_t* framebuffer,
    bool is_shadow_pass)
{
    if (!is_shadow_pass)  //对于天空盒来说，阴影是不需要的
    {
        Mesh* mesh = model->mesh;
        int num_faces = mesh->getNumFaces();
        std::vector<Vertex> vertices = mesh->getVertices();
        Program* program = model->program;
        attribs_skybox* attribs;
        for (int i = 0; i < num_faces; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                std::unique_ptr<Vertex> vertex = std::make_unique<Vertex>(vertices[i * 3 + j]);
                attribs = (attribs_skybox*)program_get_attribs(program, j);
                attribs->position = vertex->position;
            }
            graphics_draw_triangle(framebuffer, program);
        }
    }
}


std::shared_ptr<Model> skybox_create_model(std::string skybox_name, int blur_level)
{
    int sizeof_attribs = sizeof(attribs_skybox);
    int sizeof_varyings = sizeof(varyings_skybox);
    int sizeof_uniforms = sizeof(uniforms_skybox);
    uniforms_skybox* uniforms;
    Program* program = new Program(skybox_vertex_shader, skybox_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms, 0); 
    program->double_sided = 1; //记得双面渲染
    std::shared_ptr<Model> skybox = std::make_shared<Model>();
    skybox->mesh = Mesh::load("common/box.obj");
    skybox->program = program;
    skybox->transform = mat4_identity();
    skybox->skeleton = nullptr;
    skybox->transparent = 0;
    skybox->update_new = update_skybox_model;
    skybox->draw_new = draw_skybox_model;
    
    uniforms = (uniforms_skybox*)program->shader_uniforms_;
    uniforms->skybox = std::make_shared<CubeMap>(skybox_name, blur_level);
    return skybox;
}