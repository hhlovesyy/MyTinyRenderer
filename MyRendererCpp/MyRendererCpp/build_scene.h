#pragma once
// build_scene.h
#ifndef BUILD_SCENE_H
#define BUILD_SCENE_H
class SceneBuilder {
public:
    void test_draw_scene(Scene scene, framebuffer_t* framebuffer, Camera* camera);
    void sort_models(std::vector<Model*>& models, const mat4_t& view_matrix, bool isAscending = true);//�Ƿ�������
    // δ��������������Ӹ����Ա����
};



#endif // BUILD_SCENE_H