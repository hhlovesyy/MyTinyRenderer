#define _CRT_SECURE_NO_WARNINGS
#include "scene_helper.h"
#include "shader_skybox.h"
#include "macro.h"
#include <assert.h>
#include <vector>

class Scene_Blinn_t
{
public:
	int index;
	vec4 basecolor;
	float shininess;
	char diffuse_map[LINE_SIZE];
	char specular_map[LINE_SIZE];
	char emission_map[LINE_SIZE];
	char normal_map[LINE_SIZE];
	int alpha_blend;
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
	char skeleton[LINE_SIZE];
	int material;
	int transform;
};

class Scene_Light_t
{
public:
	vec3 background;
	float ambient;
	char shadow[LINE_SIZE];//Off表示不产生阴影，On表示产生阴影，也可以定义shadowmap的大小
	char skybox[LINE_SIZE]; //环境贴图的种类
	char environment[LINE_SIZE]; //环境贴图
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
	items = fscanf(file, " basecolor: %f %f %f %f", &material.basecolor[0], &material.basecolor[1], &material.basecolor[2], &material.basecolor[3]);
	assert(items == 4);
	items = fscanf(file, " shininess: %f", &material.shininess);
	assert(items == 1);
	items = fscanf(file, " diffuse_map: %s", material.diffuse_map);
	assert(items == 1);
	items = fscanf(file, " specular_map: %s", material.specular_map);
	assert(items == 1);
	items = fscanf(file, " emission_map: %s", material.emission_map);
	assert(items == 1);
	//add normal map
	items = fscanf(file, " normal_map: %s", material.normal_map);
	assert(items == 1);
	items = fscanf(file, " alpha_blend: %d", &material.alpha_blend);
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
	items = fscanf(file, " skeleton: %s", model.skeleton);
	//assert(items == 1);
	items = fscanf(file, " material: %d", &model.material);
	assert(items == 1);
	items = fscanf(file, " transform: %d", &model.transform);
	assert(items == 1);
	return model;
}

static void read_light(FILE* file, Scene_Light_t& light)
{
	char header[LINE_SIZE];
	int items;

	items = fscanf(file, " %s", header);
	if (!equals_to(header, "lighting:")) 
	{
		return;
	}
	items = fscanf(file, " background: %f %f %f",
		&light.background[0],
		&light.background[1],
		&light.background[2]);
	assert(items == 3);
	items = fscanf(file, " ambient: %f", &light.ambient);
	assert(items == 1);
	items = fscanf(file, " shadow: %s", &light.shadow);
	assert(items == 1);
	items = fscanf(file, " environment: %s", light.environment);
	assert(items == 1);
	items = fscanf(file, " skybox: %s", &light.skybox);
	assert(items == 1);

	UNUSED_VAR(items);
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

static std::string string_wrap_path(const char* path)
{
	if (equals_to(path, "null")) {
		return std::string();
	}
	else {
		return std::string(path);
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

static void scene_create(vec3 background, std::shared_ptr<Model> skybox, Scene* scene, float ambient,
int shadowmap_width, int shadowmap_height)
{
	scene->background = vec4_from_vec3(background, 1);
	scene->ambient_intensity = ambient;
	scene->skybox = skybox;
	if (shadowmap_width > 0 && shadowmap_height > 0) 
	{
		scene->shadowmap_buffer = framebuffer_create(shadowmap_width, shadowmap_height);
	}
	else 
	{
		scene->shadowmap_buffer = NULL;
	}
	
}


static void create_scene(Scene_Light_t& light, Scene* scene)
{
	//增加对于skybox的判断,引入shared_ptr
	std::shared_ptr<Model> skybox;
	if (equals_to(light.skybox, "off"))
	{
		skybox = nullptr;
	}
	else
	{
		std::string skybox_path = string_wrap_path(light.environment);
		int blur_level = 0; //模糊系数，目前先支持不模糊的版本
		if (equals_to(light.skybox, "ambient")){
			blur_level=-1;
		}
		else if (equals_to(light.skybox, "blurred")){
			blur_level = 1;
		}
		else {
			assert(equals_to(light.skybox, "on"));
			blur_level = 0;
		}
		assert(skybox_path.c_str()!=nullptr);
		skybox = skybox_create_model(skybox_path, blur_level);
	}
	
	//定义shadow的有无，以及shadowmap的大小
	int shadowmap_width, shadowmap_height;
	if (equals_to(light.shadow, "off")) 
	{
		shadowmap_width = -1;
		shadowmap_height = -1;
	}
	else 
	{
		if (equals_to(light.shadow, "on"))
		{
			shadowmap_width = 512;
			shadowmap_height = 512;
		}
		else 
		{
			int items;
			items = sscanf(light.shadow, "%dx%d",
				&shadowmap_width, &shadowmap_height);
			assert(items == 2 && shadowmap_width > 0 && shadowmap_height > 0);
			UNUSED_VAR(items);
		}
	}

	scene_create(light.background, skybox, scene, light.ambient,
	shadowmap_width, shadowmap_height
	);
}

static Scene* create_blinn_scene(Scene_Light_t& light, std::vector<Scene_Blinn_t>& materials, std::vector<Scene_Transform_t>& transforms, std::vector<Scene_Model_t>& scene_models, mat4_t root_transform)
{
	int num_materials = materials.size();
	int num_transforms = transforms.size();
	int num_models = scene_models.size();
	std::vector<Model*> models;
	Scene* scene = new Scene();
	scene->background = vec4{ 0, 0, 0, 1 };
	for (int i = 0; i < num_models; i++)
	{
		Scene_Model_t scene_model = scene_models[i];
		Scene_Transform_t scene_transform = transforms[scene_model.transform];
		Scene_Blinn_t scene_material = materials[scene_model.material];
		
		material_blinnphong material;
		
		assert(scene_model.transform < num_transforms);
		assert(scene_model.material < num_materials);
		
		std::string mesh = string_wrap_path(scene_model.mesh);
		std::string skeleton = string_wrap_path(scene_model.skeleton);
		mat4_t transform = mat4_mul_mat4(root_transform, scene_transform.matrix);
		material.basecolor = scene_material.basecolor;
		material.diffuse_map = string_wrap_path(scene_material.diffuse_map);
		//bad C++, string_wrap_path返回nullptr或者NULL都会报错，这里直接返回空字符串，后面再处理了
		material.specular_map = string_wrap_path(scene_material.specular_map);
		material.emission_map = string_wrap_path(scene_material.emission_map);
		material.normal_map = string_wrap_path(scene_material.normal_map);
		material.shininess = scene_material.shininess;
		material.alpha_blend = scene_material.alpha_blend;

		Model* model = shader_BlinnPhong_create_model(mesh, skeleton, transform, material);
		models.push_back(model);
	}

	scene->models = models;
	create_scene(light, scene);
	return scene;
}

Scene scene_from_file(const char* filename, mat4_t root)
{
	char scene_type[LINE_SIZE];
	int items = 0;

	//打印一下filename的绝对路径

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
		Scene_Light_t light;
		read_light(file, light);
		read_blinn_materials(file, materials);
		read_transforms(file, transforms);
		read_models(file, models);
		scene = *create_blinn_scene(light, materials, transforms, models, root);
	}
	else
	{
		assert(0);
		scene = Scene();
	}
	fclose(file);
	return scene;
}