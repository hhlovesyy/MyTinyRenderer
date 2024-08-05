# 课时4——模型解析与导入

**这节课主要来做obj模型的解析和导入，同学们也能够为导入自己喜欢的模型进行后续的学习。**



# 一、3D模型的表示--以obj为例

这节课我们会首先讲讲单网格（mesh）的obj模型的简单解析。后续会逐步进行更多的解析。



我们拿一个obj模型文件打开来进行查看

`v(x,y,z)` :以v开头的表示的是顶点，后面三个值表示xyz。例如：

```
# vertex positions
v -0.222180 0.021757 0.006937
v -0.219632 0.009923 0.006365
v -0.218038 0.011877 0.016683
v -0.219499 0.022392 0.018293
```



![image-20240805191642018](lesson4_模型解析与导入.assets/image-20240805191642018.png)



`vt(u,v)` :以vt开头的表示的是uv纹理坐标，后面两个值表示u和v。通常在0到1之间。如果大于1，意味着对纹理进行镜像填充、翻转填充等操作。例如：

```
# UV coordinates
vt 0.298623 0.589949
vt 0.283123 0.584219
vt 0.289757 0.573796
vt 0.303445 0.576128
vt 0.288927 0.604331
vt 0.275961 0.597488
```

`vn(x,y,z)` :以vn开头的表示的是顶点法线，后面三个值表示xyz,指示法向量的方向。例如：

```
# vertex normals
vn -0.990921 -0.088540 0.101176
vn -0.951096 -0.292274 0.099964
vn -0.924206 -0.259723 0.279977
vn -0.944558 -0.075414 0.319566
vn -0.990841 -0.088444 -0.102042
vn -0.951107 -0.292062 -0.100478
vn -0.989059 0.098723 0.109618
```

`f(v,vt,vn)` :以f开头的表示的是面，后面三个值表示上面的顶点、纹理和法线的索引。例如`f 4/4/4 1/1/1 2/2/2`其中，这个三角形面的第一个顶点的顶点索引是4，对应上面顶点坐标部分的`v -0.219499 0.022392 0.018293`。例如：

```
# Mesh
f 4/4/4 1/1/1 2/2/2
f 2/2/2 3/3/3 4/4/4
f 1/1/1 5/5/5 6/6/6
f 6/6/6 2/2/2 1/1/1
f 8/8/8 7/7/7 1/1/1
f 1/1/1 4/4/4 8/8/8
f 7/10/7 9/9/9 5/5/5
f 5/5/5 1/1/1 7/10/7
```



了解了obj文件格式如何进行解析之后，我们来动手实现一下对于obj格式的解析：

mesh.cpp中的loadObj函数

```C++
static Mesh* loadObj(const char* filename) 
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

    file = fopen(filename, "r");
    std::cout << "filename: " << filename << std::endl;
    assert(file != NULL);
    while (1) 
    {
        int items;
        if (fgets(line, LINE_SIZE, file) == NULL) 
        {
            break;
        }
        //strncmp比较两个字符串的前n个字符是否相等。
        //strncmp(line, "v ", 2) == 0用于检查字符串line的前两个字符是否与字符串"v "相等。如果相等，表示该行是以字符"v "开头的。
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
                //position_indices、texcoord_indices、normal_indices分别存储了顶点的位置、纹理坐标和法线的索引。
                position_indices.push_back(pos_indices[i] - 1);
                texcoord_indices.push_back(uv_indices[i] - 1);
                normal_indices.push_back(n_indices[i] - 1);
            }
        }
        
        //告诉编译器，我们在这里声明了一个变量items，但是我们并没有在后续的代码中使用它，防止产生不必要的警告信息。
        UNUSED_VAR(items);
    }
    fclose(file);
    mesh=后续解析
    return mesh;
}

```



>strncmp:
>
>strncmp比较两个字符串的前n个字符是否相等。
>strncmp(line, "v ", 2) == 0用于检查字符串line的前两个字符是否与字符串"v "相等。如果相等，表示该行是以字符"v "开头的。





