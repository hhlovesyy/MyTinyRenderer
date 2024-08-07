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

    //����ģ��
    static Mesh* load(std::string  filename);
    //�ͷ�ģ��
    void release();

    //��ȡģ�͵�����
    int getNumFaces() const;
    //��ȡģ�͵Ķ�������
    const std::vector<Vertex>& getVertices() const;
    //��ȡģ�͵����ĵ�
    vec3_t getCenter() const;

    std::vector<TGAImage> textures;
    TGAImage load_texture(std::string filename);

private:
    Mesh() = default;
    int num_faces;
    std::vector<Vertex> vertices;
    vec3_t center;

    //���һ����Ԫ������ buildMesh ������Ϊ Mesh �����Ԫ�������������Ϳ��Է���˽�й��캯����
    friend Mesh* buildMesh
    (
        std::vector<vec3_t>& positions, std::vector<vec2_t>& texcoords, std::vector<vec3_t>& normals,
        std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices
    );

};

#endif