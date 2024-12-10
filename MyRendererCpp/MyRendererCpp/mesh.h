#pragma once
#ifndef MESH_H
#define MESH_H

#include "maths.h"
#include "tgaimage.h"
#include "texture2D.h"
#include <vector>

struct Vertex
{
    vec3 position;
    vec2 texcoord;
    vec3 normal;
    vec4 joint;
    vec4 weight;
    vec4 tangent;
};

class Mesh 
{
public:
    

    //加载模型
    static Mesh* load(std::string  filename);
    //释放模型
    void release();

    //获取模型的面数
    int getNumFaces() const;
    //获取模型的顶点数据
    const std::vector<Vertex>& getVertices() const;
    //获取模型的中心点
    vec3 getCenter() const;

    std::vector<Texture> textures;
    Texture load_texture(std::string filename);
    std::vector<Vertex> get_vertices();

private:
    Mesh() = default;
    int num_faces;
    std::vector<Vertex> vertices;
    vec3 center;

    //添加一个友元函数或将 buildMesh 函数作为 Mesh 类的友元函数，这样它就可以访问私有构造函数。
    friend Mesh* buildMesh
    (
        std::vector<vec3>& positions, std::vector<vec2>& texcoords, std::vector<vec3>& normals, std::vector<vec4>& tangents,
        std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices,
        std::vector<vec4>& joints, std::vector<vec4>& weights
    );

};

#endif