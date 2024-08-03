# 课时2——空间变换

**暂时先主要讲解实现，矩阵的推导放在真正做教程的时候再来做。**

现在我们在屏幕上绘制出了第一个三角形，但这个三角形是静止的（也许你完成了上一课时的作业，但三角形依然变换的比较无聊）。通过这个课时的学习和实践，我们希望能够把这个三角形变得“立体化”，像是一个三维的三角形，并绘制属于我们的第一个箱子模型（一个box，从obj格式的文件中读取）

> 注意：我们实现的是OpenGL标准，这就意味着都是矩阵乘向量这种形式，如果要对向量旋转，需要把旋转矩阵乘在左边。



# 一、平移、旋转、缩放

这里直接给出对应的矩阵链接，暂时不讲推导，等到真正需要教学的时候再推导。

## （1）平移矩阵

```c++
mat4_t mat4_translate(float tx, float ty, float tz)
{
    mat4_t m = mat4_identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}
```

不妨去上一个课时的代码里进行修改，查看平移矩阵带来的影响（不过z的影响应该是看不到的，我们来修改一下x和y的属性即可）：

```c++
//修改rasterize_triangle函数
//...
void matrix_translate(vec2_t* abc)
{
	mat4_t translate = mat4_translate(l2_x_delta_trans, l2_y_delta_trans, 0);
	vec4_t a = vec4_new(abc[0].x, abc[0].y, 0, 1);
	vec4_t b = vec4_new(abc[1].x, abc[1].y, 0, 1);
	vec4_t c = vec4_new(abc[2].x, abc[2].y, 0, 1);
	a = mat4_mul_vec4(translate, a);
	b = mat4_mul_vec4(translate, b);
	c = mat4_mul_vec4(translate, c);
	abc[0] = vec2_new(a.x, a.y);
	abc[1] = vec2_new(b.x, b.y);
	abc[2] = vec2_new(c.x, c.y);
}
vec2_t abc[3] = { vec2_new(100 , 300), vec2_new(200 , 600), vec2_new(300, 100) };
matrix_rotateZ(abc);
```

修改之后再运行，可以看到三角形在屏幕上看到三角形平移运动。

------



## （2）旋转矩阵

```c++
/*
 * angle: the angle of rotation, in radians
 *
 *  1  0  0  0
 *  0  c -s  0
 *  0  s  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_x(float angle) //传入的angle是弧度，意味着90度是PI/2.0
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c  0  s  0
 *  0  1  0  0
 * -s  0  c  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_y(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

/*
 * angle: the angle of rotation, in radians
 *
 *  c -s  0  0
 *  s  c  0  0
 *  0  0  1  0
 *  0  0  0  1
 *
 * see http://www.songho.ca/opengl/gl_anglestoaxes.html
 */
mat4_t mat4_rotate_z(float angle)
{
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    mat4_t m = mat4_identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return m;
}
```

在提供的Github代码中，有测试绕着Z轴旋转的相关的测试函数，读者可以进行测试，查看旋转矩阵的执行效果。

------



## （3）缩放矩阵

```c++
/*
 * sx, sy, sz: scale factors along the x, y, and z axes, respectively
 *
 * sx  0  0  0
 *  0 sy  0  0
 *  0  0 sz  0
 *  0  0  0  1
 *
 * see http://docs.gl/gl2/glScale
 */
mat4_t mat4_scale(float sx, float sy, float sz) 
{
    mat4_t m = mat4_identity();
    assert(sx != 0 && sy != 0 && sz != 0);
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}
```



------

