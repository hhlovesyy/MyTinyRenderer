#include "scene.h"
#include "build_scene.h"



void SceneBuilder::test_draw_scene(Scene* scene, framebuffer_t* framebuffer)
{
    /*
    std::vector<Model*>& models = scene->models;  // ��ȡ����ģ��
    int num_models = models.size();  // ��ȡģ������
    int i;

    for (i = 0; i < num_models; i++) 
    {  // ��������ģ��
        Model* model = models[i];
        model->update(perframe);  // ���ø��º���
    }
    


    sort_models(models, perframe->camera_view_matrix);  // ���������ͼ��������ģ��
    framebuffer->clear_color(scene->background);  // �����ɫ����
    framebuffer->clear_depth(1);  // �����Ȼ���
    if (skybox == nullptr || perframe->layer_view >= 0)
    {  // ��������ģ��
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
        {  // �����͸��ģ������
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
        {  // �������з�͸��ģ��
            Model* model = models[i];
            model->draw(framebuffer, 0);
        }
        skybox->draw(framebuffer, 0);  // ������պ�
        for (i = num_opaques; i < num_models; i++) 
        {  // ����͸��ģ��
            Model* model = models[i];
            model->draw(framebuffer, 0);
        }
    }
    */
}

//static void sort_models(std::vector<Model*>& models, const mat4& view_matrix)
//{
//    int num_models = models.size();  // ��ȡģ������
//    if (num_models > 1)
//    {
//        for (int i = 0; i < num_models; i++)
//        {  // ��������ģ��
//            Model* model = models[i];
//            vec3 center = model->mesh->get_center();  // ��ȡģ������
//            vec4 local_pos = vec4(center, 1.0f);  // ת��Ϊ4D����
//            vec4 world_pos = model->transform * local_pos;  // �任����������
//            vec4 view_pos = view_matrix * world_pos;  // �任����ͼ����
//            model->distance = -view_pos.z;  // �������
//        }
//        std::sort(models.begin(), models.end(), compare_models);  // ʹ��std::sort
//    }
//}