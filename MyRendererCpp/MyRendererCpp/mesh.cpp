#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "macro.h"
#include "mesh.h"
#include "private.h"
#include "config.h"


//buildMesh�������Ǹ��ݶ������ݹ���һ��Mesh����
static Mesh* buildMesh
(
    std::vector<vec3>& positions, std::vector<vec2>& texcoords, std::vector<vec3>& normals, std::vector<vec4>& tangents,
    std::vector<int>& position_indices, std::vector<int>& texcoord_indices, std::vector<int>& normal_indices,
    std::vector<vec4>& joints, std::vector<vec4>& weights
) 
{
    //bbox_min��bbox_max�ֱ��ʾģ�͵İ�Χ�е���С����󶥵����ꡣ
    vec3 bbox_min{ +1e6, +1e6, +1e6 };
    vec3 bbox_max{ -1e6, -1e6, -1e6 };
    int num_indices = position_indices.size();
    int num_faces = num_indices / 3;
    std::vector<Vertex> vertices(num_indices);
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

        if (tangents.size() > 0)
        {
            int tangent_index = position_index;
            assert(tangent_index >= 0 && tangent_index < tangents.size());
			vertices[i].tangent = tangents[tangent_index];
		}
        else
        {
            vertices[i].tangent = vec4{ 1, 0, 0, 1 };
		}

        if (joints.size() > 0 && weights.size() > 0)
        {
			vertices[i].joint = joints[position_index];
			vertices[i].weight = weights[position_index];
		}
        else
        {
            vertices[i].joint = vec4{ 0, 0, 0, 0 };
            vertices[i].weight = { 0, 0, 0, 0 };
        }

        bbox_min = vec_min(bbox_min, vertices[i].position);
        bbox_max = vec_max(bbox_max, vertices[i].position);
    }

    // ���� Mesh ���������
    mesh->num_faces = num_faces;
    mesh->vertices = std::move(vertices);
    mesh->center = (bbox_min + bbox_max) / 2.0;

    return mesh;
}

static Mesh* loadObj(std::string filename) 
{
    std::vector<vec3> positions;
    std::vector<vec2> texcoords;
    std::vector<vec3> normals;
    std::vector<int> position_indices;
    std::vector<int> texcoord_indices;
    std::vector<int> normal_indices;
    //add joints and weights
    std::vector<vec4> joints;
    std::vector<vec4> weights;
    std::vector<vec4> tangents;
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
            vec3 position;
            items = sscanf(line, "v %f %f %f",
                &position[0], &position[1], &position[2]);
            assert(items == 3);
            positions.push_back(position);
        }
        else if (strncmp(line, "vt ", 3) == 0) 
        {             
            vec2 texcoord;
            items = sscanf(line, "vt %f %f",
                &texcoord[0], &texcoord[1]);
            assert(items == 2);
            texcoords.push_back(texcoord);
        }
        else if (strncmp(line, "vn ", 3) == 0)
        {             
            vec3 normal;
            items = sscanf(line, "vn %f %f %f",
                &normal[0], &normal[1], &normal[2]);
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
        else if (strncmp(line, "# ext.tangent ", 14) == 0)
        {
            vec4 tangent;
            items = sscanf(line, "# ext.tangent %f %f %f %f",
                &tangent[0], &tangent[1], &tangent[2], &tangent[3]);
            assert(items == 4);
            tangents.push_back(tangent);
        }
        else if (strncmp(line, "# ext.joint ", 12) == 0) {    /* joint */
            vec4 joint;
            items = sscanf(line, "# ext.joint %f %f %f %f",
                &joint[0], &joint[1], &joint[2], &joint[3]);
            assert(items == 4);
            joints.push_back(joint);
        }
        else if (strncmp(line, "# ext.weight ", 13) == 0) {   /* weight */
            vec4 weight;
            items = sscanf(line, "# ext.weight %f %f %f %f",
                &weight[0], &weight[1], &weight[2], &weight[3]);
            assert(items == 4);
            weights.push_back(weight);
        }
        
        //���߱�����������������������һ������items���������ǲ�û���ں����Ĵ�����ʹ��������ֹ��������Ҫ�ľ�����Ϣ��
        UNUSED_VAR(items);
    }
    fclose(file);

    mesh = buildMesh(positions, texcoords, normals,tangents, 
        position_indices, texcoord_indices, normal_indices, joints, weights);

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

const std::vector<Vertex>& Mesh::getVertices() const 
{
    return vertices;
}

vec3 Mesh::getCenter() const 
{
    return center;
}


//��������

Texture Mesh::load_texture(std::string filename) 
{
    TGAImage tmp;
    Texture res;
    std::string texfile = filename;
    //std::string texfile = "combinePamu_diffuse.tga";
    if (tmp.read_tga_file(texfile.c_str()))
    {
        std::cout << "load texture file " << texfile << " success" << std::endl;
        
        if (Global_Config::should_flip_uv_y)
        {
            TGAImage* tmp1 = &tmp;
            tmp1->flip_vertically();
        }
        res.set_texture(tmp);
        textures.push_back(res);
    }
    else
    {
        std::string log_file_name = texfile.size() > 0 ? texfile : "null";
        std::cerr << "load texture file " << log_file_name << " failed" << std::endl;
    }
    return res;
}

