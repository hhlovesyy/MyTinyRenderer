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
    // 新增：更新场景的所有mesh
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

    //区分透明与非透明模型
    vector<Model*> TransModels;//透明物体
    vector<Model*> OpaqueModels;//非透明物体
    for (int index = 0; index < models.size(); index++)
    {  // 统计非透明模型的数量
        Model* model = models[index];

        if (model->transparent == 0)
        {
            OpaqueModels.push_back(model);
        }
        else
        {
            TransModels.push_back(model);  //renderQueue = transparent + 1 
            //break;  // 透明模型不计入
        }
    }

    //渲染shadowmap
    if (scene.shadowmap_buffer)
    {  // 如果场景有阴影缓冲区和阴影贴图
        sort_models(models, FrameInfo::get_light_view_matrix());// 按照光源的视角矩阵对模型进行排序(为了优化性能 可以采用先近后远的顺序)
        framebuffer_clear_depth(scene.shadowmap_buffer, 1);  // 清除阴影缓冲区的深度信息
        for (int index = 0; index < OpaqueModels.size(); index++)
        {  // 遍历所有模型
            Model* model = models[index];
            model->draw(model, scene.shadowmap_buffer,true);  // 绘制模型到阴影缓冲区
            //draw是函数指针，如可以指向draw_model(Model* model, framebuffer_t* framebuffer,bool isDrawShadowMap)
        }
        if(scene.shadowmap == nullptr)
            scene.shadowmap = new Texture();
        scene.shadowmap->set_texture_from_depth_buffer(scene.shadowmap_buffer);
        //以下三句可以把阴影贴图保存到文件，方便debug，注意要做一步gamma校正
        /*std::unique_ptr<Texture> dump_shadowmap = std::make_unique<Texture>(*scene.shadowmap);
        dump_shadowmap->linear_to_srgb();
        dump_shadowmap->write_texture_to_file("shadowmap_srgb.tga");*/
        // 将阴影缓冲区的深度信息转换为阴影贴图

        for(int index =0;index<OpaqueModels.size();index++) //todo:可以优化，现在是把总的shadowmap赋值给每个model的uniform的值，方便后面用
		{
            Model* model = models[index];
            uniforms_blinnphong* uniforms = (uniforms_blinnphong*)model->program->get_uniforms();
            uniforms->shadowmap = scene.shadowmap;
            
		}
        
        //texture_from_depthbuffer(scene->shadow_map, scene->shadow_buffer);  // 将阴影缓冲区的深度信息转换为阴影贴图
    }

    //渲染顺序

    //先渲染不透明物体，再渲染透明物体
    //绘制不透明模型 为了优化性能 可以采用先近后远的顺序
    sort_models(OpaqueModels, camera_get_view_matrix(*camera));
    for (int index = 0; index < OpaqueModels.size(); index++)
	{
		Model* model = OpaqueModels[index];
        if (model->draw)
        {
            model->draw(model, framebuffer, false);
        }
	}
    //绘制天空盒
    if (skybox != nullptr)
    {
        skybox->draw_new(skybox, framebuffer, false);
    }
    
    
    //绘制透明模型
    //绘制透明物体时，应该采用先远后近的顺序
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
			return a->distance < b->distance; // 升序排序
		});
	}
	else
	{
		std::sort(models.begin(), models.end(), [](Model* a, Model* b) {
			return a->distance > b->distance; // 降序排序
		});
	}
  
}
void SceneBuilder::sort_models(std::vector<Model*>& models, const mat4_t& view_matrix,bool isAscending)//是否是升序

{
    int num_models = models.size();  // 获取模型数量
    if (num_models > 1)
    {
        for (int i = 0; i < num_models; i++)
        {  // 遍历所有模型
            Model* model = models[i];
            vec3_t center = model->mesh->getCenter();  // 获取模型中心
            vec4_t local_pos = vec4_from_vec3(center, 1.0f);  // 转换为4D齐次坐标
            vec4_t world_pos = mat4_mul_vec4(model->transform, local_pos);  // 变换到世界坐标

            vec4_t view_pos = mat4_mul_vec4(view_matrix, world_pos);
            model->distance = -view_pos.z;  // 计算距离
        }
        Qsort(models,isAscending);  
    }
}