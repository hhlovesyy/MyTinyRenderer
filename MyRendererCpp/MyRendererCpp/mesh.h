#pragma once
#ifndef MESH_H
#define MESH_H

#include "maths.h"
#include "tgaimage.h"
#include <vector>

class Mesh 
{
public:
    struct Vertex 
    {
        vec3_t position;
        vec2_t texcoord;
        vec3_t normal;
    };

    //加载模型
    static Mesh* load(std::string  filename);
    //释放模型
    void release();

    //获取模型的面数
    int getNumFaces() const;
    //获取模型的顶点数据
    const std::vector<Vertex>& getVertices() const;
    //获取模型的中心点
    vec3_t getCenter() const;

    std::vector<TGAImage> textures;
    TGAImage load_texture(std::string filename);

private:
    Mesh() = default;
    int num_faces;
    std::vector<Vertex> vertices;
    vec3_t center;

    //添加一个友元函数或将 buildMesh 函数作为 Mesh 类的友元函数，这样它就可以访问私有构造函数。
    friend Mesh* buildMesh
    (
        std::vector<vec3_t>& positions, std::vector<vec2_t>& texcoords, std::vector<vec3_t>& normals,
        std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices
    );

};

#endif