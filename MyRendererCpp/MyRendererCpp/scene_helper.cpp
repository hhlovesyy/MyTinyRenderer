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
	std::vector<Scene_Blinn_t> materials;
	int materials_cnt = 0;
	items = fscanf(file, " materials: %d", &materials_cnt);
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
	items = fscanf(file, " transforms: %d", &transforms_cnt);
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

static void read_models(FILE* file, std::vector<Model>& models)
{
	int items = 0;
	int models_cnt = 0;
	items = fscanf(file, " models: %d", &models_cnt);
	assert(items == 1);
	for (int i = 0; i < models_cnt; i++)
	{
		
	}
}

Scene* scene_from_file(const char* filename)
{
	char scene_type[LINE_SIZE];
	int items = 0;

	FILE* file = fopen(filename, "rb");
	assert(file != NULL);
	items = fscanf(file, " type: %s", scene_type);
	assert(items == 1); //说明读取成功
	UNUSED_VAR(items);

	if (equals_to(scene_type, "blinn")) //目前只支持Blinn的相关场景
	{
		std::vector<Scene_Blinn_t> materials;
		std::vector<Scene_Transform_t> transforms;
		std::vector<Model> models;
		read_blinn_materials(file, materials);
		read_transforms(file, transforms);
		read_models(file, models);


	}

}