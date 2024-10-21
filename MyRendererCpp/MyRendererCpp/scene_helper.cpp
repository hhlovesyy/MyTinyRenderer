#define _CRT_SECURE_NO_WARNINGS
#include "scene_helper.h"
#include "macro.h"
#include <assert.h>
#include <vector>

class Scene_Blinn_t
{
public:
	int index;
	vec4_t basecolor;
	char diffuse_map[LINE_SIZE];
};

class Scene_Transform_t
{
public:
	int index;
	mat4_t matrix;
};

class Scene_Model_t
{
public:
	int index;
	char mesh[LINE_SIZE];
	int material;
	int transform;
};

static int equals_to(const char* str1, const char* str2) 
{
	return strcmp(str1, str2) == 0;
}

static Scene_Blinn_t read_a_blinn_material(FILE* file)
{
	int items = 0;
	Scene_Blinn_t material;
	
	items = fscanf(file, " material %d:", &material.index);
	assert(items == 1);
	items = fscanf(file, " basecolor: %f %f %f %f", &material.basecolor.x, &material.basecolor.y, &material.basecolor.z, &material.basecolor.w);
	assert(items == 4);
	items = fscanf(file, " diffuse_map: %s", material.diffuse_map);
	assert(items == 1);
	UNUSED_VAR(items);  //消除未使用的变量的警告
	return material;
}

static void read_blinn_materials(FILE* file, std::vector<Scene_Blinn_t>& materials)
{
	int items = 0;
	int materials_cnt = 0;
	items = fscanf(file, " materials %d:", &materials_cnt);
	assert(items == 1); 
	for (int i = 0; i < materials_cnt; i++)
	{
		Scene_Blinn_t material = read_a_blinn_material(file);
		assert(material.index == i);
		materials.push_back(material);
	}
}

static void read_transforms(FILE* file, std::vector<Scene_Transform_t>& transforms)
{
	int items = 0;
	int transforms_cnt = 0;
	items = fscanf(file, " transforms %d:", &transforms_cnt);
	int item = 0;
	assert(items == 1);
	for (int i = 0; i < transforms_cnt; i++)
	{
		Scene_Transform_t transform;
		item = fscanf(file, " transform %d:", &transform.index);
		assert(item == 1);
		assert(transform.index == i);
		for (int j = 0; j < 4; j++)
		{
			item = fscanf(file, " %f %f %f %f",
				&transform.matrix.m[j][0],
				&transform.matrix.m[j][1],
				&transform.matrix.m[j][2],
				&transform.matrix.m[j][3]);
			assert(item == 4);
		}
		transforms.push_back(transform);
	}
	UNUSED_VAR(item);
	UNUSED_VAR(items);
}

static Scene_Model_t read_model(FILE* file)
{
	Scene_Model_t model;
	int items = 0;
	items = fscanf(file, " model %d:", &model.index);
	assert(items == 1);
	items = fscanf(file, " mesh: %s", model.mesh);
	assert(items == 1);
	items = fscanf(file, " material: %d", &model.material);
	assert(items == 1);
	items = fscanf(file, " transform: %d", &model.transform);
	assert(items == 1);
	return model;
}

static void read_models(FILE* file, std::vector<Scene_Model_t>& models)
{
	int items = 0;
	int models_cnt = 0;
	items = fscanf(file, " models %d:", &models_cnt);
	assert(items == 1);
	for (int i = 0; i < models_cnt; i++)
	{
		Scene_Model_t model = read_model(file);
		assert(model.index == i);
		models.push_back(model);
	}
}

static const char* wrap_path(const char* path)
{
	if (equals_to(path, "null")) {
		return NULL;
	}
	else {
		return path;
	}
}

static int wrap_knob(const char* knob)
{
	if (equals_to(knob, "on")) {
		return 1;
	}
	else {
		assert(equals_to(knob, "off"));
		return 0;
	}
}

static Scene* create_blinn_scene(std::vector<Scene_Blinn_t>& materials, std::vector<Scene_Transform_t>& transforms, std::vector<Scene_Model_t>& scene_models)
{
	int num_materials = materials.size();
	int num_transforms = transforms.size();
	int num_models = scene_models.size();
	std::vector<Model*> models;
	Scene* scene = new Scene();
	scene->background = vec4_new(0, 0, 0, 1);
	for (int i = 0; i < num_models; i++)
	{
		Scene_Model_t scene_model = scene_models[i];
		Scene_Transform_t scene_transform = transforms[scene_model.transform];
		Scene_Blinn_t scene_material = materials[scene_model.material];
		
		material_blinnphong material;
		
		assert(scene_model.transform < num_transforms);
		assert(scene_model.material < num_materials);
		
		const char* mesh = wrap_path(scene_model.mesh);
		material.basecolor = scene_material.basecolor;
		material.diffuse_map = wrap_path(scene_material.diffuse_map);

		Model* model = shader_BlinnPhong_create_model(mesh, scene_transform.matrix, material);
		models.push_back(model);
	}

	scene->models = models;
	return scene;
}

Scene scene_from_file(const char* filename)
{
	char scene_type[LINE_SIZE];
	int items = 0;

	FILE* file = fopen(filename, "rb");
	assert(file != NULL);
	items = fscanf(file, " type: %s", scene_type);
	assert(items == 1); //说明读取成功
	UNUSED_VAR(items);
	Scene scene;

	if (equals_to(scene_type, "blinn")) //目前只支持Blinn的相关场景
	{
		std::vector<Scene_Blinn_t> materials;
		std::vector<Scene_Transform_t> transforms;
		std::vector<Scene_Model_t> models;
		read_blinn_materials(file, materials);
		read_transforms(file, transforms);
		read_models(file, models);
		//scene = create_blinn_scene(materials, transforms, models);
		scene = *create_blinn_scene(materials, transforms, models);
	}
	else
	{
		assert(0);
		scene = Scene();
	}
	fclose(file);
	return scene;
}