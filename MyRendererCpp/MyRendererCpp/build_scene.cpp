#include "scene.h"
#include "scene_helper.h"
#include <vector>
#include "build_scene.h"
#include "rasterization.h"
#include <algorithm> // for std::sort
using namespace std;

void SceneBuilder::test_draw_scene(Scene& scene, framebuffer_t* framebuffer, Camera* camera)
{
    vector<Model*> models = scene.models;
    // ���������³���������mesh
    for (int index = 0; index < models.size(); index++)
    {
		Model* model = models[index];
        if (model->update)
        {
			model->update(model, camera);
		}
	}

    std::shared_ptr<Model> skybox = scene.skybox;
    if (skybox != nullptr)
    {
        skybox->update_new(skybox, camera);
	}

    //����͸�����͸��ģ��
    vector<Model*> TransModels;//͸������
    vector<Model*> OpaqueModels;//��͸������
    for (int index = 0; index < models.size(); index++)
    {  // ͳ�Ʒ�͸��ģ�͵�����
        Model* model = models[index];

        if (model->transparent == 0)
        {
            OpaqueModels.push_back(model);
        }
        else
        {
            TransModels.push_back(model);  //renderQueue = transparent + 1 
            //break;  // ͸��ģ�Ͳ�����
        }
    }

    //��Ⱦshadowmap
    if (scene.shadowmap_buffer)
    {  // �����������Ӱ����������Ӱ��ͼ
        sort_models(models, FrameInfo::get_light_view_matrix());// ���չ�Դ���ӽǾ����ģ�ͽ�������(Ϊ���Ż����� ���Բ����Ƚ���Զ��˳��)
        framebuffer_clear_depth(scene.shadowmap_buffer, 1);  // �����Ӱ�������������Ϣ
        for (int index = 0; index < OpaqueModels.size(); index++)
        {  // ��������ģ��
            Model* model = models[index];
            model->draw(model, scene.shadowmap_buffer,true);  // ����ģ�͵���Ӱ������
            //draw�Ǻ���ָ�룬�����ָ��draw_model(Model* model, framebuffer_t* framebuffer,bool isDrawShadowMap)
        }
        if(scene.shadowmap == nullptr)
            scene.shadowmap = new Texture();
        scene.shadowmap->set_texture_from_depth_buffer(scene.shadowmap_buffer);
        //����������԰���Ӱ��ͼ���浽�ļ�������debug��ע��Ҫ��һ��gammaУ��
        /*std::unique_ptr<Texture> dump_shadowmap = std::make_unique<Texture>(*scene.shadowmap);
        dump_shadowmap->linear_to_srgb();
        dump_shadowmap->write_texture_to_file("shadowmap_srgb.tga");*/
        // ����Ӱ�������������Ϣת��Ϊ��Ӱ��ͼ

        for(int index =0;index<OpaqueModels.size();index++) //todo:�����Ż��������ǰ��ܵ�shadowmap��ֵ��ÿ��model��uniform��ֵ�����������
		{
            Model* model = models[index];
            uniforms_blinnphong* uniforms = (uniforms_blinnphong*)model->program->get_uniforms();
            uniforms->shadowmap = scene.shadowmap;
            
		}
        
        //texture_from_depthbuffer(scene->shadow_map, scene->shadow_buffer);  // ����Ӱ�������������Ϣת��Ϊ��Ӱ��ͼ
    }

    //��Ⱦ˳��

    //����Ⱦ��͸�����壬����Ⱦ͸������
    //���Ʋ�͸��ģ�� Ϊ���Ż����� ���Բ����Ƚ���Զ��˳��
    sort_models(OpaqueModels, camera_get_view_matrix(*camera));
    for (int index = 0; index < OpaqueModels.size(); index++)
	{
		Model* model = OpaqueModels[index];
        if (model->draw)
        {
            model->draw(model, framebuffer, false);
        }
	}
    //������պ�
    if (skybox != nullptr)
    {
        skybox->draw_new(skybox, framebuffer, false);
    }
    
    
    //����͸��ģ��
    //����͸������ʱ��Ӧ�ò�����Զ�����˳��
    sort_models(TransModels, camera_get_view_matrix(*camera),false);
    for (int index = 0; index < TransModels.size(); index++)
	{
		Model* model = TransModels[index];
        if (model->draw)
        {
			model->draw(model, framebuffer, false);
		}
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