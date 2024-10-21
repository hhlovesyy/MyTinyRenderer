#include "scene.h"
#include "scene_helper.h"
#include <vector>
#include "build_scene.h"
#include "rasterization.h"
#include <algorithm> // for std::sort
using namespace std;

void SceneBuilder::test_draw_scene(Scene scene, framebuffer_t* framebuffer, Camera* camera)
{
    vector<Model*> models = scene.models;
    //渲染顺序
    //模型排序
    int num_models = models.size();
    sort_models(models, camera_get_view_matrix(*camera));

    for (int index = 0; index < models.size(); index++)
    {
        Mesh* mesh = models[index]->mesh;

        Program* program = models[index]->program;
        uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();
        uniforms->light_dir = vec3_new(0.5f, 0.8f, 0.9f);
        uniforms->camera_pos = camera->position;

        uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*camera), camera_get_view_matrix(*camera));

        rasterization_tri(mesh, program, framebuffer);
    }
    /*
    std::vector<Model*>& models = scene->models;  // 获取所有模型
    int num_models = models.size();  // 获取模型数量
    int i;

    for (i = 0; i < num_models; i++) 
    {  // 更新所有模型
        Model* model = models[i];
        model->update(perframe);  // 调用更新函数
    }

    sort_models(models, perframe->camera_view_matrix);  // 按摄像机视图矩阵排序模型
    framebuffer->clear_color(scene->background);  // 清除颜色缓冲
    framebuffer->clear_depth(1);  // 清除深度缓冲
    if (skybox == nullptr || perframe->layer_view >= 0)
    {  // 绘制所有模型
        for (i = 0; i < num_models; i++)
        {
            Model* model = models[i];
            model->draw(framebuffer, 0);
        }
    }
    else
    {
        int num_opaques = 0;
        for (i = 0; i < num_models; i++) 
        {  // 计算非透明模型数量
            Model* model = models[i];
            if (model->opaque) 
            {
                num_opaques++;
            }
            else 
            {
                break;
            }
        }

        for (i = 0; i < num_opaques; i++) 
        {  // 绘制所有非透明模型
            Model* model = models[i];
            model->draw(framebuffer, 0);
        }
        skybox->draw(framebuffer, 0);  // 绘制天空盒
        for (i = num_opaques; i < num_models; i++) 
        {  // 绘制透明模型
            Model* model = models[i];
            model->draw(framebuffer, 0);
        }
    }
    */
}
void Qsort(std::vector<Model*>& models)
{
    std::sort(models.begin(), models.end(), [](Model* a, Model* b) {
        return a->distance < b->distance; // 升序排序
        });
}
void SceneBuilder::sort_models(std::vector<Model*>& models, const mat4_t& view_matrix)
{
    int num_models = models.size();  // 获取模型数量
    if (num_models > 1)
    {
        for (int i = 0; i < num_models; i++)
        {  // 遍历所有模型
            Model* model = models[i];
            vec3_t center = model->mesh->getCenter();  // 获取模型中心
            vec4_t local_pos = vec4_from_vec3(center, 1.0f);  // 转换为4D向量
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);  // 变换到世界坐标

            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;  // 计算距离
        }
        Qsort(models);  // 使用std::sort
    }
}
//Qsort函数
