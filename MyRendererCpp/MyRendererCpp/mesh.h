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
    

    //����ģ��
    static Mesh* load(std::string  filename);
    //�ͷ�ģ��
    void release();

    //��ȡģ�͵�����
    int getNumFaces() const;
    //��ȡģ�͵Ķ�������
    const std::vector<Vertex>& getVertices() const;
    //��ȡģ�͵����ĵ�
    vec3 getCenter() const;

    std::vector<Texture> textures;
    Texture load_texture(std::string filename);
    std::vector<Vertex> get_vertices();

private:
    Mesh() = default;
    int num_faces;
    std::vector<Vertex> vertices;
    vec3 center;

    //���һ����Ԫ������ buildMesh ������Ϊ Mesh �����Ԫ�������������Ϳ��Է���˽�й��캯����
    friend Mesh* buildMesh
    (
        std::vector<vec3>& positions, std::vector<vec2>& texcoords, std::vector<vec3>& normals, std::vector<vec4>& tangents,
        std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices,
        std::vector<vec4>& joints, std::vector<vec4>& weights
    );

};

#endif