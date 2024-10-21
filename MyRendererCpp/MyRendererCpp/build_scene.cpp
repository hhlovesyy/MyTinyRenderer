#include "scene.h"
#include "build_scene.h"



void SceneBuilder::test_draw_scene(Scene* scene, framebuffer_t* framebuffer)
{
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

//static void sort_models(std::vector<Model*>& models, const mat4& view_matrix)
//{
//    int num_models = models.size();  // 获取模型数量
//    if (num_models > 1)
//    {
//        for (int i = 0; i < num_models; i++)
//        {  // 遍历所有模型
//            Model* model = models[i];
//            vec3 center = model->mesh->get_center();  // 获取模型中心
//            vec4 local_pos = vec4(center, 1.0f);  // 转换为4D向量
//            vec4 world_pos = model->transform * local_pos;  // 变换到世界坐标
//            vec4 view_pos = view_matrix * world_pos;  // 变换到视图坐标
//            model->distance = -view_pos.z;  // 计算距离
//        }
//        std::sort(models.begin(), models.end(), compare_models);  // 使用std::sort
//    }
//}