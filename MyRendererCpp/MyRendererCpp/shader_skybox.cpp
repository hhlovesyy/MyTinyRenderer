#include "shader_skybox.h"

vec4_t skybox_vertex_shader(void* attribs_, void* varyings_, void* uniforms_)
{
    return vec4_new(0, 0, 0, 0);
}

vec4_t skybox_fragment_shader(void* varyings, void* uniforms,
    int* discard, int backface)
{
    return vec4_new(0, 0, 0, 0);
}

static void update_skybox_model(Model* model, Camera* perframe)
{
    
}

static void draw_skybox_model(Model* model, framebuffer_t* framebuffer,
    bool is_shadow_pass)
{

}


std::shared_ptr<Model> skybox_create_model(std::string skybox_name, int blur_level)
{
    int sizeof_attribs = sizeof(attribs_skybox);
    int sizeof_varyings = sizeof(varyings_skybox);
    int sizeof_uniforms = sizeof(uniforms_skybox);
    uniforms_skybox* uniforms;
    Program* program = new Program(skybox_vertex_shader, skybox_fragment_shader,
        sizeof_attribs, sizeof_varyings, sizeof_uniforms, 0);
    std::shared_ptr<Model> skybox = std::make_shared<Model>();
    skybox->mesh = Mesh::load("common/box.obj");
    skybox->program = program;
    skybox->transform = mat4_identity();
    skybox->skeleton = nullptr;
    skybox->transparent = 0;
    skybox->update = update_skybox_model;
    skybox->draw = draw_skybox_model;
    
    uniforms = (uniforms_skybox*)program->shader_uniforms_;
    uniforms->skybox = std::make_shared<CubeMap>(skybox_name, blur_level);
    return skybox;
}