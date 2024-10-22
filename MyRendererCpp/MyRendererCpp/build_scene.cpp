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
    
    //����͸�����͸��ģ��
    vector<Model*> TransModels ;//͸������
    vector<Model*> OpaqueModels;//��͸������
    for (int index = 0; index < models.size(); index++)
    {  // ͳ�Ʒ�͸��ģ�͵�����
        Model* model = models[index];
        
        if (model->transparent==0)
        {
            OpaqueModels.push_back(model);
        }
        else
        {
            TransModels.push_back(model);
            //break;  // ͸��ģ�Ͳ�����
        }
    }
    //����Ⱦ��͸�����壬����Ⱦ͸������
    //���Ʋ�͸��ģ�� ����Ҫ��ע˳��
    sort_models(OpaqueModels, camera_get_view_matrix(*camera));
    for (int index = 0; index < OpaqueModels.size(); index++)
	{
		Model* model = OpaqueModels[index];
        Mesh* mesh = OpaqueModels[index]->mesh;

        Program* program = OpaqueModels[index]->program;
        uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();
        uniforms->light_dir = vec3_new(0.5f, 0.8f, 0.9f);
        uniforms->camera_pos = camera->position;

        uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*camera), camera_get_view_matrix(*camera));

        rasterization_tri(mesh, program, framebuffer);
	}
    //����͸��ģ��
    //����͸������ʱ��Ӧ�ò�����Զ�����˳��
    sort_models(TransModels, camera_get_view_matrix(*camera),false);
    for (int index = 0; index < TransModels.size(); index++)
	{
		Model* model = TransModels[index];
		Mesh* mesh = TransModels[index]->mesh;

		Program* program = TransModels[index]->program;
		uniforms_blinnphong* uniforms = (uniforms_blinnphong*)program->get_uniforms();
		uniforms->light_dir = vec3_new(0.5f, 0.8f, 0.9f);
		uniforms->camera_pos = camera->position;

		uniforms->camera_vp_matrix = mat4_mul_mat4(camera_get_proj_matrix(*camera), camera_get_view_matrix(*camera));

		rasterization_tri(mesh, program, framebuffer);
	}
}
void Qsort(std::vector<Model*>& models, bool isAscending)
{
    if(isAscending)
	{
		std::sort(models.begin(), models.end(), [](Model* a, Model* b) {
			return a->distance < b->distance; // ��������
		});
	}
	else
	{
		std::sort(models.begin(), models.end(), [](Model* a, Model* b) {
			return a->distance > b->distance; // ��������
		});
	}
  
}
void SceneBuilder::sort_models(std::vector<Model*>& models, const mat4_t& view_matrix,bool isAscending)//�Ƿ�������

{
    int num_models = models.size();  // ��ȡģ������
    if (num_models > 1)
    {
        for (int i = 0; i < num_models; i++)
        {  // ��������ģ��
            Model* model = models[i];
            vec3_t center = model->mesh->getCenter();  // ��ȡģ������
            vec4_t local_pos = vec4_from_vec3(center, 1.0f);  // ת��Ϊ4D�������
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);  // �任����������

            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;  // �������
        }
        Qsort(models,isAscending);  
    }
}