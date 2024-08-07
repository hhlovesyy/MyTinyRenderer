#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "macro.h"
#include "mesh.h"
#include "private.h"


//buildMesh�������Ǹ��ݶ������ݹ���һ��Mesh����
static Mesh* buildMesh
(
    std::vector<vec3_t>& positions, std::vector<vec2_t>& texcoords, std::vector<vec3_t>& normals,
    std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices
) 
{
    //bbox_min��bbox_max�ֱ��ʾģ�͵İ�Χ�е���С����󶥵����ꡣ
    vec3_t bbox_min = vec3_new(+1e6, +1e6, +1e6);
    vec3_t bbox_max = vec3_new(-1e6, -1e6, -1e6);
    int num_indices = position_indices.size();
    int num_faces = num_indices / 3;
    std::vector<Mesh::Vertex> vertices(num_indices);
    Mesh* mesh = new Mesh();//buildMesh��Mesh �����Ԫ�������������Ϳ��Է���˽�й��캯��

    // ���Լ��
    assert(num_faces > 0 && num_faces * 3 == num_indices);
    assert(position_indices.size() == num_indices);
    assert(texcoord_indices.size() == num_indices);
    assert(normal_indices.size() == num_indices);

    // �������е��棬������������
    for (int i = 0; i < num_indices; i++) 
    {
        int position_index = position_indices[i];
        int texcoord_index = texcoord_indices[i];
        int normal_index = normal_indices[i];
        assert(position_index >= 0 && position_index < positions.size());
        assert(texcoord_index >= 0 && texcoord_index < texcoords.size());
        assert(normal_index >= 0 && normal_index < normals.size());
        vertices[i].position = positions[position_index];
        vertices[i].texcoord = texcoords[texcoord_index];
        vertices[i].normal = normals[normal_index];

        bbox_min = vec3_min(bbox_min, vertices[i].position);
        bbox_max = vec3_max(bbox_max, vertices[i].position);
    }

    // ���� Mesh ���������
    mesh->num_faces = num_faces;
    mesh->vertices = std::move(vertices);
    mesh->center = vec3_div(vec3_add(bbox_min, bbox_max), 2);

    return mesh;
}

static Mesh* loadObj(std::string filename) 
{
    std::vector<vec3_t> positions;
    std::vector<vec2_t> texcoords;
    std::vector<vec3_t> normals;
    std::vector<int> position_indices;
    std::vector<int> texcoord_indices;
    std::vector<int> normal_indices;
    char line[LINE_SIZE];
    Mesh* mesh;
    FILE* file;

    file = fopen(filename.c_str(), "r");
    std::cout << "filename: " << filename << std::endl;
    assert(file != NULL);
    while (1) 
    {
        int items;
        if (fgets(line, LINE_SIZE, file) == NULL) 
        {
            break;
        }
        //strncmp�Ƚ������ַ�����ǰn���ַ��Ƿ���ȡ�
        //strncmp(line, "v ", 2) == 0���ڼ���ַ���line��ǰ�����ַ��Ƿ����ַ���"v "��ȡ������ȣ���ʾ���������ַ�"v "��ͷ�ġ�
        else if (strncmp(line, "v ", 2) == 0) 
        {              
            vec3_t position;
            items = sscanf(line, "v %f %f %f",
                &position.x, &position.y, &position.z);
            assert(items == 3);
            positions.push_back(position);
        }
        else if (strncmp(line, "vt ", 3) == 0) 
        {             
            vec2_t texcoord;
            items = sscanf(line, "vt %f %f",
                &texcoord.x, &texcoord.y);
            assert(items == 2);
            texcoords.push_back(texcoord);
        }
        else if (strncmp(line, "vn ", 3) == 0)
        {             
            vec3_t normal;
            items = sscanf(line, "vn %f %f %f",
                &normal.x, &normal.y, &normal.z);
            assert(items == 3);
            normals.push_back(normal);
        }
        else if (strncmp(line, "f ", 2) == 0) 
        {               
            int i;
            int pos_indices[3], uv_indices[3], n_indices[3];
            items = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &pos_indices[0], &uv_indices[0], &n_indices[0],
                &pos_indices[1], &uv_indices[1], &n_indices[1],
                &pos_indices[2], &uv_indices[2], &n_indices[2]);
            assert(items == 9);
            for (i = 0; i < 3; i++) 
            {
                //position_indices��texcoord_indices��normal_indices�ֱ�洢�˶����λ�á���������ͷ��ߵ�������
                position_indices.push_back(pos_indices[i] - 1);
                texcoord_indices.push_back(uv_indices[i] - 1);
                normal_indices.push_back(n_indices[i] - 1);
            }
        }
        
        //���߱�����������������������һ������items���������ǲ�û���ں����Ĵ�����ʹ��������ֹ��������Ҫ�ľ�����Ϣ��
        UNUSED_VAR(items);
    }
    fclose(file);

    mesh = buildMesh(positions, texcoords, normals, 
        position_indices, texcoord_indices, normal_indices);

    return mesh;
}

//load�������ڼ���ģ���ļ�������һ��Mesh����
Mesh* Mesh::load(std::string filename) 
{
    std::string extension_str = private_get_extension(filename);
    const char* extension = extension_str.c_str();

    if (strcmp(extension, "obj") == 0) 
    {
        return loadObj(filename);
    }
    else 
    {
        assert(0);
        return nullptr;
    }
}

void Mesh::release() 
{
    delete this;
}

/* vertex retrieving */

int Mesh::getNumFaces() const 
{
    return num_faces;
}

const std::vector<Mesh::Vertex>& Mesh::getVertices() const 
{
    return vertices;
}

vec3_t Mesh::getCenter() const 
{
    return center;
}

//��������

TGAImage Mesh::load_texture(std::string filename) 
{
    TGAImage tmp;
    std::string texfile = filename;
    //std::string texfile = "combinePamu_diffuse.tga";
    if (tmp.read_tga_file(texfile.c_str()))
    {
        std::cout << "load texture file " << texfile << " success" << std::endl;
        textures.push_back(tmp);
    }
    else
    {
        std::cerr << "load texture file " << texfile << " failed" << std::endl;
    }
    return tmp;
}