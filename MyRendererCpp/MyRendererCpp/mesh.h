#pragma once
#ifndef MESH_H
#define MESH_H

#include "maths.h"
#include <vector>

class Mesh {
public:
    struct Vertex {
        vec3_t position;
        vec2_t texcoord;
        vec3_t normal;
        vec4_t tangent;
    };

    /* mesh loading/releasing */
    static Mesh* load(const char* filename);
    void release();

    /* vertex retrieving */
    int getNumFaces() const;
    const std::vector<Vertex>& getVertices() const;
    vec3_t getCenter() const;

private:
    Mesh() = default;
    int num_faces;
    std::vector<Vertex> vertices;
    vec3_t center;

    //添加一个友元函数或将 buildMesh 函数作为 Mesh 类的友元函数，这样它就可以访问私有构造函数。
    friend Mesh* buildMesh(
        std::vector<vec3_t>& positions, std::vector<vec2_t>& texcoords, std::vector<vec3_t>& normals,
        std::vector<vec4_t>& tangents,
        std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices);

};

#endif