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
    
    //区分透明与非透明模型
    vector<Model*> TransModels ;//透明物体
    vector<Model*> OpaqueModels;//非透明物体
    for (int index = 0; index < models.size(); index++)
    {  // 统计非透明模型的数量
        Model* model = models[index];
        
        if (model->transparent==0)
        {
            OpaqueModels.push_back(model);
        }
        else
        {
            TransModels.push_back(model);
            //break;  // 透明模型不计入
        }
    }
    //先渲染不透明物体，再渲染透明物体
    //绘制不透明模型 不需要关注顺序
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
    //绘制透明模型
    //绘制透明物体时，应该采用先远后近的顺序
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