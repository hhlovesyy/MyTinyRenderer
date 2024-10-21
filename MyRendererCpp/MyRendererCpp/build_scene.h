#pragma once
// build_scene.h
#ifndef BUILD_SCENE_H
#define BUILD_SCENE_H
class SceneBuilder {
public:
    void test_draw_scene(Scene scene, framebuffer_t* framebuffer, Camera* camera);
    void sort_models(std::vector<Model*>& models, const mat4_t& view_matrix);
    // 未来可以在这里添加更多成员函数
};



#endif // BUILD_SCENE_H