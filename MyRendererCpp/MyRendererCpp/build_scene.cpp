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
    //��Ⱦ˳��
    //ģ������
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
void Qsort(std::vector<Model*>& models)
{
    std::sort(models.begin(), models.end(), [](Model* a, Model* b) {
        return a->distance < b->distance; // ��������
        });
}
void SceneBuilder::sort_models(std::vector<Model*>& models, const mat4_t& view_matrix)
{
    int num_models = models.size();  // ��ȡģ������
    if (num_models > 1)
    {
        for (int i = 0; i < num_models; i++)
        {  // ��������ģ��
            Model* model = models[i];
            vec3_t center = model->mesh->getCenter();  // ��ȡģ������
            vec4_t local_pos = vec4_from_vec3(center, 1.0f);  // ת��Ϊ4D����
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);  // �任����������

            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;  // �������
        }
        Qsort(models);  // ʹ��std::sort
    }
}
//Qsort����
