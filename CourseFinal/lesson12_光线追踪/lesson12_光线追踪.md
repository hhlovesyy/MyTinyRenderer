# lesson12_光线追踪



### 1. 引言

- 光线追踪与传统渲染技术的对比（如光栅化）+光线追踪的基本概念

​	不知道读者是否会觉得，我们之前实现的光栅化效果不够真实，并不那么像现实中的效果。例如很难实现如下图中物体A反射物体B的光。

<img src="lesson12_光线追踪.assets/image-20241205164248618.png" alt="image-20241205164248618" style="zoom: 67%;" />

​	本章我们将介绍光线追踪，光线追踪能够更真实地反映现实中的光影效果。在现实中，我们的眼睛看到的颜色都是因为接收到了光，现实中的光线从光源出发，在场景中不断弹射，如果最终进入到我们的眼睛，那么我们的眼睛就能看到这个色彩。

![image-20241205162438446](lesson12_光线追踪.assets/image-20241205162438446.png)

​	根据光路可逆性，我们同样可以从眼睛/相机出发，回溯这根光线在到达相之前是怎样的传播路径，打到了什么物体，经过了几次的反射，几次折射等。我们要做的就是追踪光线，这就是光线追踪的思想.对于每个像素，我们从相机开始发射一根（或者多根）光线穿过这个像素，追踪这根光线，如果这根光线与场景中的物体相交，我们就计算着色，并模拟光线的反射或者折射行为，例如此时光线打到了如图所示的光滑镜面材质上，那我们就进行镜面反射的计算，创建新的光线往镜面反射的方向出发。最后把这根光线这一路上的颜色相加，就是最终呈现在屏幕像素上的颜色。

![image-20241205163344639](lesson12_光线追踪.assets/image-20241205163344639.png)

​	光线追踪并不需要像光栅化那样，计算复杂的空间变换。

​	复习一下光栅化，需要遍历空间中的三角形，将三角形从模型坐标转换到世界空间，相机空间，裁剪空间，最终转换到屏幕空间等，然后再去遍历屏幕中的每个像素，看像素是否在三角形内，进而进行着色等操作。

```
for (T in triangles )
    for (P in pixels )
        检查像素P是否在三角形T中
```

​	（伪代码参考[1]]）

​	而光线追踪则是对于每个屏幕中的像素，我们发射光线，检查场景中的所有物体/图元（如三角形）是否会被射线打到，如果会，光线就会“染上”物体的颜色。

【只需要将物体，光源，相机，光线等转到同一个坐标系下即可，例如物体从模型空间转到世界空间，同时光源与相机也转换到世界空间统一计算【存疑】】

```
for (P in pixels )
    for (T in triangles )
        检查穿过像素P的射线是否打中三角形T
```



#### 1.1 实验环境

​	由于光线追踪与光栅化是完全不同的两个体系，而且光线追踪的计算量非常大，不再适合我们之前的用CPU渲染的C++光栅化渲染器，而应该采用GPU，且光追作为一个单独的体系，也希望之前没有跟着写渲染器的读者可以实现光线追踪，且由于希望读者无需配任何环境，我们选择使用在线网站Shadertoy（https://www.shadertoy.com/）作为光线追踪的实验环境。

​	读者可以打开Shadertoy（https://www.shadertoy.com/），使用GLSL语言直接进行实验。

### 2. 光线追踪的基础原理

#### 2.1光线的生成

##### 2.1.1定义一条光线

要实现光线追踪，我们首先要知道如何定义一条光线。

- 我们将光线定义为$Ray(t)=\mathbf{o}+t \mathbf{d}$

  - $Ray(t)$是一个三维坐标的点，沿着光线移动；$o(origin)$是起点，$d(direction)$则是光线的方向；$t(time)$是一个实数，可以理解为时刻

  - 不同的t值可以到达不同的地方，如果t只能取正数那么这就是一条射线：

  ![image-20241205172158791](lesson12_光线追踪.assets/image-20241205172158791.png)

```GLSL
// 光线结构体
struct Ray
{
    vec3 origin;     // 光线起点
    vec3 direction;  // 光线方向
};
```



```GLSL
// 相交信息结构体
struct IntersectInfo
{
    // 表面属性
    float t;          // 相交点的距离
    vec3  p;          // 相交点位置
    // 材质属性等
    ......
};
```



##### 2.1.2光线与屏幕坐标的关联

得到了光线的定义之后，我们要决定这条光线要往哪个方向去发射呢？$Ray(t)=\mathbf{o}+t \mathbf{d}$中的$\mathbf{o} 和\mathbf{d}$是多少呢？（已知t可以是不同的值，让光线到达不同的地方）

光线应当从相机出发，往屏幕上的每一个像素发射。因而起点$o(origin)$是相机，那么方向$d(direction)$呢？

我们从相机$o(origin)$点发射一条光线Ray（红色线），穿过像素A的中心。这些光线被称作者相机光线（camera ray) 或者眼睛光线（eye ray）。

（u,v）是屏幕上像素点A的坐标，

![image-20241206115911828](lesson12_光线追踪.assets/image-20241206115911828.png)





- 在本项目当中，为了防止混淆导致不好Debug，我们选择不同长宽的图像

- 暂时来说，认为视角的aspect ratio就是渲染画面的宽高比。定义viewport的高度是2.0，同时定义被投影点与投影平面的距离是焦距，为1.0。我们将相机放置在o点，同时认为**X轴向右，Y轴向上，Z轴正方向指向屏幕外面**（也就是说，这是一个**右手坐标系**）（补充一下，**Unity的模型坐标系和世界坐标系都是左手坐标系**，Z轴正方向是指向屏幕内侧的）。从左上角开始遍历屏幕空间。

- 此时，如何表示从视角打向成像平面的像素光线的方向？

  - 认为屏幕左下角为`lower_left_corner`,例如可以是坐标（-2，-1，-1）；
  - 认为水平方向为`horizontal`,例如可以是(4.0,0.0,0.0),而竖直方向为`vertical`,例如可以是(0.0,2.0,0.0)，后面将会用比例系数来表示点在屏幕中的位置；

  ![image-20241206120520390](lesson12_光线追踪.assets/image-20241206120520390.png)

  - 在上图当中，$\vec{direction}=\vec{b}+\vec{a}$
    - $\vec{b}=lowerleftcorner-origin$
    - $\vec{a}=u*\vec{horizontal}+v*\vec{vertical}$ 

  - 代入$\vec{a}和\vec{b}$,得到结果为$\vec{direction}=lowerleftcorner-origin+u*\vec{horizontal}+v*\vec{vertical}$



```GLSL
// 获取摄像机的光线
Ray Camera_getRay(Camera camera, float u, float v)
{
    vec3 rd = camera.lensRadius * random_in_unit_disk();  // 随机偏移
    vec3 offset = camera.u * rd.x + camera.v * rd.y;  // 计算偏移量

    Ray ray;

    ray.origin = camera.origin + offset;  // 光线的起始位置
    ray.direction = camera.lowerLeftCorner - camera.origin + u * camera.horizontal + v * camera.vertical  - offset;  // 光线的方向

    return ray;
}
```

至此，我们就可以完成穿过所有像素的我们想要的所有光线的生成。那么接下来，就要让光线不断往前走，直到与场景中的物体相交（也可能最后都没有与任何物体相交），计算光线与物体的交点，然后计算光线的颜色以及光线接下来往那个方向发射（比如反射或者折射）。



#### 2.2 光线与物体求交

我们将光线从相机中发射出来之后，

##### 2.2.1 光线与球面相交

(1)光线方程

$Ray(t)=\mathbf{o}+t \mathbf{d}(0≤t<∞)\quad$

(2)球面方程



- 半径为R的球体的方程：

  - $x^2+y^2+z^2=R^2$

  - 也就是说，如果一个坐标点$(x，y，z)$满足上面的方程，我们就认为这个点在球面上，否则不在球面上；

- 根据上式推广，如果球心在 $C  =(cx，cy，cz)$，则球的方程为：

  - $(x-cx)^2+(y-cy)^2+(z-cz)^2=R^2$

- 再次推广，此时对于球面上的点p，有：

  - $(p(t)-C)^2-R^2=0\quad$,其中c为中心点的坐标

- 也就是说，对于光线上的点$p(t)=\mathbf{o}+t \mathbf{d}$，有$(p(t)-C)^2-R^2=0\quad$

  也就是$(\mathbf{o}+t \mathbf{d}-C)^2=R^2$ 

**将上式中的点乘展开，进行多项式展开，可以得到化简之后的式子：**

$ t^2·\vec{d}·\vec{d}+2·t·\vec{d}·\vec{(o-C)}+\vec{(o-C)}·\vec{(o-C)}-R^2=0$

可以看到，通过这个式子求解二次方程,**就可以求解出t的值**，通过t的值来判定是否相交以及相交的交点数

对于求根公式来说，可以令$b_o=2b$，则此时求根公式可以简化如下：
$$
\begin{aligned}
& \frac{-b_o \pm \sqrt{b_o^{2}-4 a c}}{2 a} \\
= & \frac{-2 b \pm \sqrt{(2 b)^{2}-4 a c}}{2 a} \\
= & \frac{-2 b \pm 2 \sqrt{b^{2}-a c}}{2 a} \\
= & \frac{-b \pm \sqrt{b^{2}-a c}}{a}
\end{aligned}
$$

我们让：
$$
a=\vec{d}·\vec{d}\\
b=\vec{d}·\vec{(o-C)}\\
c=\vec{(o-C)}·\vec{(o-C)}-R^2
$$
因此原来的式子变为:(再次注意和我们学的二次方程稍有不同，t的参数是2b而不是b)
$$
a·t^2+2·b·t+c=0
$$

得到:
$$
t = \frac{-b \pm \sqrt{b^2-ac}}{a}
$$



如果判别式$discriminant=\sqrt{b^2-ac} >0 $,说明有2个交点，$discriminant=\sqrt{b^2-ac} <0 $说明无交点。如下图：

![image-20241210113610405](lesson12_光线追踪.assets/image-20241210113610405.png)

```GLSL
// 球体与光线相交检测函数
bool Sphere_hit(Sphere sphere, Ray ray, float t_min, float t_max, out IntersectInfo rec)
{
    vec3 oc = ray.origin - sphere.center;  // 计算光线起点与球心的差向量
    float a = dot(ray.direction, ray.direction);  // 计算光线方向的平方
    float b = dot(oc, ray.direction);  // 计算光线起点到球心的差向量与光线方向的点积
    float c = dot(oc, oc) - sphere.radius * sphere.radius;  // 计算光线起点到球心的差向量的平方减去球的半径平方

    float discriminant = b * b - a * c;  // 判别式

    if (discriminant > 0.0f)  // 如果判别式大于零，说明有交点
    {
        float temp = (-b - sqrt(discriminant)) / a;  // 计算第一个交点

        if (temp < t_max && temp > t_min)  // 判断交点是否在有效范围内
        {
            rec.t                = temp;  // 记录交点距离
            rec.p                = ray.origin + rec.t * ray.direction;  // 计算交点位置
            rec.normal           = (rec.p - sphere.center) / sphere.radius;  // 计算法向量
            rec.materialType     = sphere.materialType;  // 记录材质类型
            rec.albedo           = sphere.albedo;  // 记录反射率
            rec.fuzz             = sphere.fuzz;  // 记录模糊度
            rec.refractionIndex  = sphere.refractionIndex;  // 记录折射率

            return true;  // 返回交点
        }

        temp = (-b + sqrt(discriminant)) / a;  // 计算第二个交点

        if (temp < t_max && temp > t_min)  // 判断交点是否在有效范围内
        {
            rec.t                = temp;  // 记录交点距离
            rec.p                = ray.origin + rec.t * ray.direction;  // 计算交点位置
            rec.normal           = (rec.p - sphere.center) / sphere.radius;  // 计算法向量
            rec.materialType     = sphere.materialType;  // 记录材质类型
            rec.albedo           = sphere.albedo;  // 记录反射率
            rec.fuzz             = sphere.fuzz;  // 记录模糊度
            rec.refractionIndex  = sphere.refractionIndex;  // 记录折射率

            return true;  // 返回交点
        }
    }

    return false;  // 如果没有交点，返回false
}
```




##### 2.2.2 光线与**隐式平面求交**

联立方程组:
$$
\begin{cases}
  r(t)=p=o+td(0≤t<∞)\quad① \\ 
  p:f(p)=0\quad ②
\end{cases}.
$$
也就是说,解方程$f(o+td)=0$即可.



##### 2.2.3 光线与三角形求交

###### (1)使用三角形的重心坐标判断

判断光线是否与三角形相交,可以按照下面的步骤(**注:该算法需要进行优化**):

- 1.判断光线是否与三角形所在平面相交;
- 2.判断交点是否在三角形的内部(**使用重心**)

**1.判断光线是否与三角形所在平面相交;**

已知平面方程如下:

$ax+by+cz+d=0$

则对于平面上的点$p$,有:

- $p:(p-p')·N=0$  ($p'$是平面上一点,$N$是平面的法线方向,$\vec{N}=(a,b,c)$)



**此时我们联立光线方程与平面方程,求解二者的交点$p$**
$$
\begin{cases}  r(t)=p=o+td(0≤t<∞)\quad① \\   p:(p-p')·N=0\quad ②\end{cases}.
$$
将①代入②,有$(o+td-p')·N=0$,通过点乘的分配律求解出:$\large t=\frac{(p'-o)·N}{d·N}$(需要验证是否满足$0≤t<∞$)

接下来,只需要判断$o+td$这个点是否在三角形内部即可(使用重心坐标来判断)



**2.判断交点是否在三角形的内部(使用重心)**

我们复习一下第一节光栅化提到的重心坐标判断点是否在三角形内部的内容：

重心坐标$(w,u,v)$表示点P在三角形ABC内的位置。

（以下以二维情况为例）

对于三角形ABC内任意一点P, 存在三个数u,v,w，满足：

$u+v+w=1$

$P=w*A+u*B+v*C$

则$(w,u,v)$称为此三角形上P点的重心坐标



在三角形情形中，重心坐标也叫**面积坐标**，因为*P*点关于三角形*ABC*的重心坐标和三角形PBC,PCA及PAB的（有向）面积成比例，

$w:u:v = S(PBC):S(PCA):S(PAB)$



<img src="lesson12_光线追踪.assets/image-20240919173325490.png" alt="image-20240919173325490" style="zoom:67%;" />



（2）判断点在三角形内

如果点P在三角形内部，则它必然会对三角形的每一边形成一个小三角形，这些小三角形的面积都是正的（即w,u,v都大于零）。

如果其中任一重心坐标（比如u）为零或负数，意味着点P恰好位于某一边上或者在边外，这样就不再是“严格”位于三角形内部。

因此只有当三角形的三个重心坐标都>0时才认为是在三角形内（这里暂时不过多考虑边界情况，比如=0之类的）。

因此,我们只需要计算出点P的重心坐标,就可以判断这一点是否在三角形内,从而决定是否为其着色。



 （3） 重心坐标计算公式

使$w=1-u-v$

则$P=w*A+u*B+v*C = (1-u-v)*A+u*B+v*C$

展开

$P = A-u*A-v*A+u*B+v*C $

$P = A+u*(B-A)+v*(C-A)$

$P = A+u*\overrightarrow{AB}+v*\overrightarrow{AC}$

$u*\overrightarrow{AB}+v*\overrightarrow{AC}+\overrightarrow{PA} = \overrightarrow{0}$

由此，可以推出以下两个方程：




$$
\begin{bmatrix} u \\ v \\ 1 \end{bmatrix} \begin{bmatrix} \overrightarrow{AB}_x & \overrightarrow{AC}_x & \overrightarrow{PA}_x \end{bmatrix} = 0
$$


$$
\begin{bmatrix} u \\ v \\ 1 \end{bmatrix} \begin{bmatrix} \overrightarrow{AB}_y & \overrightarrow{AC}_y & \overrightarrow{PA}_y \end{bmatrix} = 0
$$

因此$\begin{bmatrix} u & v & 1 \end{bmatrix} $与$\begin{bmatrix} \overrightarrow{AB}_x & \overrightarrow{AC}_x & \overrightarrow{PA}_x \end{bmatrix} $和$\begin{bmatrix} \overrightarrow{AB}_y & \overrightarrow{AC}_y & \overrightarrow{PA}_y \end{bmatrix} $都垂直，

>在三维几何中，向量a和向量b的叉乘结果是一个向量，该向量垂直于a和b向量构成的平面。
>
>$\overrightarrow{a} \times \overrightarrow{b} =  \overrightarrow{n}$
>
>则 $ \overrightarrow{n}$与 $ \overrightarrow{a}$垂直， $ \overrightarrow{n}$与 $ \overrightarrow{b}$垂直，
>
>反之亦然，如果 $ \overrightarrow{n}$与 $ \overrightarrow{a}$垂直， $ \overrightarrow{n}$与 $ \overrightarrow{b}$垂直，
>
>则$\overrightarrow{a} \times \overrightarrow{b} =  \overrightarrow{n}$

不难看出，实际上重心坐标就等于$(\overrightarrow{AB}_x, \overrightarrow{AC}_x, \overrightarrow{PA}_x)$和$(\overrightarrow{AB}_y, \overrightarrow{AC}_y, \overrightarrow{PA}_y)$的叉乘结果（当然要归一化z到1）。
$$
\begin{bmatrix} u & v & 1 \end{bmatrix} =(\overrightarrow{AB}_x, \overrightarrow{AC}_x, \overrightarrow{PA}_x) \times (\overrightarrow{AB}_y, \overrightarrow{AC}_y, \overrightarrow{PA}_y)
$$

$$
(\overrightarrow{AB}_x, \overrightarrow{AC}_x, \overrightarrow{PA}_x) \times (\overrightarrow{AB}_y, \overrightarrow{AC}_y, \overrightarrow{PA}_y) = \\
\begin{vmatrix}i&j&k\\\overrightarrow{AB}_x&\overrightarrow{AC}_x&\overrightarrow{PA}_x\\\overrightarrow{AB}_y&\overrightarrow{AC}_y&\overrightarrow{PA}_y
\end{vmatrix} \\
$$


$$
[(\overrightarrow{AC}_x\cdot \overrightarrow{PA}_y)-(\overrightarrow{AC}_y\cdot \overrightarrow{PA}_x)]\overrightarrow{i} \\
-[(\overrightarrow{AB}_x\cdot \overrightarrow{PA}_y)-(\overrightarrow{AB}_y\cdot \overrightarrow{PA}_x)]\overrightarrow{j} \\
+[(\overrightarrow{AB}_x\cdot \overrightarrow{AC}_y)-(\overrightarrow{AB}_y\cdot \overrightarrow{AC}_x)]\overrightarrow{k}
$$



至此,我们发现,当我们只知道A,B,C和P的坐标,就顺利算出P的重心坐标$\begin{bmatrix} 1-u-v,u , v \end{bmatrix} $,通过判断u,v是否>0,我们就可以判断P是否在三角形内.



###### (2)补充:`Möller Trumbore Algorithm`

- 这是一种简化算法,可以立刻直接求解判断**光线与三角形**求交，而不需要先求光线与三角形交点然后再使用重心坐标判断是否在三角形内部.

该算法的推导可以参考下列博客:

[(8条消息) Möller-Trumbore算法-射线三角形相交算法_zhanxi1992的博客-CSDN博客](https://blog.csdn.net/zhanxi1992/article/details/109903792)

推导过程如下:

假设$P_0、P_1、P_2$是三角形的三个顶点：

> 利用重心坐标一步求解交点是否在三角形内,我们需要求解如下方程:
>
> $\vec{O}+t\vec{D}=(1-b1-b2)\vec{P_0}+b_1\vec{P_1}+b_2\vec{P_2}$
>
> (以下证明为了方便,不再标注向量符号)
>
> 将括号展开,移项可得:
>
> $O-P_0=(P_1-P_0)b_1+(P_2-P_0)b_2-tD\quad ①$
>
> 通过观察可知,$O-P_0,(P_1-P_0),(P_2-P_0)$都是已知量,因此可以将其用变量来表示:
> $$
> E_1=P_1-P_0 \\
> E_2=P_2-P_0 \\
> S=O-P_0
> $$
>
> 此时①式可化为:
>
> $S=E_1b_1+E_2b_2-tD$
>
> 将其转为矩阵乘法的表示形式,有:
> $$
> \begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}
> \begin{bmatrix}
> t\\b_1\\b_2
> \end{bmatrix}=S
> $$
> 这个方程形如$Ax=c$,因此可以用**克拉默法则**求解出$t$的值(注意到此时的)$$\begin{bmatrix}-D&E_1&E_2\end{bmatrix}$$其实是一个$3×3$的矩阵(因为每一项是一个列向量),这里我们考虑$det\begin{bmatrix}-D&E_1&E_2\end{bmatrix}≠0$的情况.所以有:
>
> ​																						$$ \large{t=det\frac{\begin{bmatrix}S&E_1&E_2\end{bmatrix}}{\begin{bmatrix}-					D&E_1&E_2\end{bmatrix}}} \quad ②$$
>
> 关于克拉默法则的更进一步介绍,可以参考[克拉默法则 - 知乎 (zhihu.com)](https://zhuanlan.zhihu.com/p/161429987)
>
> 注意到:
> $$
> det\begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}=det
> \begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}^T
> $$
> **此时由向量混合积可以得出:**
>
> 分母部分:
> $$
> det\begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}=det
> \begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}^T=-D·(E_1×E_2)=E_1·(D×E_2)
> $$
> 令$S_2=D×E_2$
>
> 所以就可以推导出:
> $$
> det\begin{bmatrix}
> -D&E_1&E_2
> \end{bmatrix}=E_1·S_1 \quad ③
> $$
> 对于分子部分也是同理,令$S_2=S×E$
>
> 可以用类似的推导方式推导出:
> $$
> det\begin{bmatrix}
> S&E_1&E_2
> \end{bmatrix}=S_2·E_2 \quad ④
> $$
> 将③④代入②,解得:
>
> ​																					$t=\LARGE\frac{S_2·E_2}{E_1·S_1}$
>
> 同样,也是可以解出$b_1,b_2$的:
>
> ​																					$b_1=\LARGE\frac{S_1·S}{E_1·S_1}$
>
> ​																					$b_2=\LARGE\frac{S_2·D}{E_1·S_1}$
>
> 所以,我们只需要判断是否满足$t≥0,b_1≥0,b_2≥0,(1-b_1-b_2)≥0$即可判断光线是否与三角形相交.





##### 2.2.4 场景物体众多

回顾一下，我们此时学会了从相机出发，对于每个像素发射一条（多条）光线，然后追踪这些光线，看它们是否与场景中的物体相交。最简单的做法是，从像素点打出一根光线后，然后遍历场景中的所有物体，计算他们是否与光线相交。如下图，如果与多个物体相交，我们可以找到最近的交点。



![image-20241206160250572](lesson12_光线追踪.assets/image-20241206160250572.png)

(图来自[2]Wang, Zijin. "The Development of Ray Tracing and Its Future." (2022).)

但是如果如下图场景中有非常多的物体，每个物体都要和每个光线判断是否相交，这个性能上会有很大的问题。因此，我们可以使用**空间加速**结构，例如八叉树，BVH或者k-d树。（后续会更新空间加速章节）

<img src="lesson12_光线追踪.assets/image-20241206160836711.png" alt="image-20241206160836711" style="zoom: 25%;" />



#### 2.3 像素着色

那么接下来我们就知道相交的物体与相交的点，我们应该做些什么，直接计算返回交点的颜色？还是继续反射/折射光线？实际上，这取决于光线相交的物体的材质等各种因素。
##### 2.3.2.镜面反射-金属材质
假设相交的物体材质是金属类型的，表面很光滑，那么光线大概率会沿着反射方向反射出去。

<img src="lesson12_光线追踪.assets/image-20241206162620524.png" alt="image-20241206162620524" style="zoom:50%;" />



对于光滑的金属材质来说, 光线是不会像漫反射那样随机散射的, 而是产生类似镜面反射。我们用向量来具体推导出这种反射方向：



<img src="lesson12_光线追踪.assets/image-20241206170417439.png" alt="image-20241206170417439" style="zoom:50%;" />

我们想要求出红色的向量（ $\vec{r}$）的公式：

$\vec{r}=\vec{v}+2*\vec{b}$

其中，已知法线方向$\vec{N}$是一个单位向量，所以我们假设向量$\vec{v}$与水平方向的夹角为θ，则$\left|\vec{b}\right |=\left|\vec{v}\right|*sin(\theta)=-\left|\vec{v}\right|*cos(90°+\theta)$

所以$\left|\vec{b}\right |=-\vec{v}·\vec{N}$,又因为向量$\vec{b}$ 的方向与$\vec{N}$保持一致，所以$\vec{b}=(-\vec{v}·\vec{N})*\vec{N}$

可以推导出：

$\vec{r}=\vec{v}+2*\vec{b}=\vec{v}-2*(\vec{v}·\vec{N})*\vec{N}$

代码如下：

```GLSL
//镜面反射
vec3 m_reflect(vec3 v, vec3 n)
{
    return v - 2.0 * dot(v, n) * n;
}
```

实际上shaertoy中也自带refrect函数，我们使用shadertoy中的refrect函数也一样。

![image-20241210120115375](lesson12_光线追踪.assets/image-20241210120115375.png)

###### fuzzy效果(磨砂效果)

在现实中,有很多金属并不是完美的镜面反射,而是磨砂材质的,如下图: 

![image-20241210120416650](lesson12_光线追踪.assets/image-20241210120416650.png)

我们还可以给反射方向**加入一点点随机性, 只要在算出反射向量后**, 在其终点为球心的球内随机选取一个点作为最终的终点，如下图所示：



<img src="lesson12_光线追踪.assets/image-20241210153710107.png" alt="image-20241210153710107" style="zoom:67%;" />

<img src="lesson12_光线追踪.assets/image-20241210153720584.png" alt="image-20241210153720584" style="zoom:67%;" />



当然上图这个灰色空心的球越大, 金属看上去就更加模糊(fuzzy, 或者说粗糙)。所以我们这里引入一个变量来表示模糊的程度(fuzziness)(所以当fuzz=0时不会产生模糊)。如果fuzz, 也就是随机球的半径很大, 光线可能会散射到物体内部去。这时候我们可以认为物体吸收了光线。



```GLSL
else if(materialType == METAL)  // 金属材质
    {
        float fuzz = isectInfo.fuzz;  // 金属表面的粗糙度（模糊程度）

        // 计算反射光线的方向
        vec3 reflected = m_reflect(normalize(wo.direction), isectInfo.normal);

        wi.origin = isectInfo.p;
        wi.direction = reflected + fuzz * random_in_unit_sphere();  ////增加对模糊情况的判断，通过对反射光实现一定范围的偏移得到最终的反射光线

        attenuation = isectInfo.albedo;

        return (dot(wi.direction, isectInfo.normal) > 0.0f);  // 确保反射光线朝向表面
    }
```



##### 2.3.3 漫反射材质

如果是漫反射材质，比如很多绝缘体（塑料，橡皮,  木头桌子）等等粗糙材质，光线打到表面后，反射光线会向四面八方发射，而不是集中反射到某个区域被我们捕捉到.

<img src="lesson12_光线追踪.assets/image-20241206164547209.png" alt="image-20241206164547209" style="zoom:50%;" />

>回忆与对比Phong模型或者Blinn-Phong模型中的漫反射处理
>
>在之前的"基础光照"章节中,我们使用Phong模型或者Blinn-Phong模型来模拟漫反射,我们复习一下其公式$L_d = k_d(I/r^2)max(0, \vec{n} · \vec{l})$,其中$\vec{l}$为指向光源的方向.实际上,这模拟的是如果光“直接照射”到的表面会更亮一些，而非直接照射的区域会更暗，背光侧最暗。
>
>在真实情况或光线追踪中:
>
>从光源角度出发, 光线并不容易到达某物体的背光面, 更容易到达表面"直接面向"光源的地方; 
>
>从像素出发, 如从上图交点处的"逆"光线往四面八方发射出去, "直接面向"光源的地方其反射的光线会更多的到达光源, 最终呈现在像素上的颜色会更亮. 而背光面的地方, 往四面八方发射出去光线, 也可能到达不了光源. 
>
>因此, Phong模型或者Blinn-Phong模型中的漫反射处理实际上是对真实的漫反射的一种模拟, 在光追中, 我们可以更真实地实现它.

​	我们可以采用在与漫反射材质的交点处,发射非常多的光线并追踪光线; 当然发射的光线越多, 计算量越大, 速度也就越慢.  如果想要加快速度, 我们也可以牺牲一定的质量做简化, 例如随机采样一个或者几个方向作为反射方向.

​	如何随机采样呢?实际上, 这也是有说法的, 我们应该选择在**单位球面**选取一个随机点, 长度固定为单位向量长度, 来作为随机发射的方向.

​	要得到这个反射想象，我们可以从交点出发，向法线方向走单位距离，然后随机取一个单位长度的方向，到达目标点，类似于：

<img src="lesson12_光线追踪.assets/image-20241210140532355.png" alt="image-20241210140532355" style="zoom:50%;" />

然后交点向目标点连线，得到的这个方向就是我们要进行反射的方向。

<img src="lesson12_光线追踪.assets/image-20241210140549561.png" alt="image-20241210140549561" style="zoom:50%;" />

代码如下：

```GLSL
if(materialType == LAMBERT)  // 漫反射材质
    {
        // 计算一个随机散射方向
        vec3 target = isectInfo.p + isectInfo.normal + random_in_unit_sphere();

        wi.origin = isectInfo.p;  // 散射光线的起始位置为交点
        wi.direction = target - isectInfo.p;  // 散射光线的方向

        attenuation = isectInfo.albedo;  // 反射衰减（反射率）

        return true;
    }
```

效果如下：

![image-20241210140138509](lesson12_光线追踪.assets/image-20241210140138509.png)

##### 2.3.4 折射

如果材质是类似玻璃,钻石和水等会发生折射的材质，那么光线不仅会发生反射，还会发生折射。

<img src="lesson12_光线追踪.assets/image-20241206165256910.png" alt="image-20241206165256910" style="zoom:50%;" />

​	还记得透明物体的章节我们提到过折射吗,此时我们再复习一下:从光学原理来看，光线入射到半透明物体的表面，一部分光线被反射出去，一部分光线被折射进入表面。光线在不同介质中的传播速度不同，回顾一下初中物理，我们将介质的折射率$\eta_i$定义为$\frac{c}{v}$，其中$c$是真空中光速，$v$是介质中的光速。

​	如下图所示，入射角与折射角之间的关系由Snell定律给出：
$$
\frac{\sin\theta_r}{\sin\theta_i}=\frac{\eta_1}{\eta_2}
$$

<img src="lesson12_光线追踪.assets/image-20241209161901633.png" alt="image-20241209161901633" style="zoom: 67%;" />

​	其中，$\theta_i$是入射角，$\theta_r$是折射角，$\eta_1$是入射介质的折射率，$\eta_2$是折射介质的折射率。

​	透明物体的章节中我们没有考虑折射,而是直接假设所有介质的折射率都是一样的，这样折射角总是与入射角相等。对于很薄的表面来说，效果其实是可以接受的.但我们既然实现了光线追踪,就可以做更真实的效果.

​      我们在前面的镜面反射部分已经求得反射方向,我们现在的目的是求出折射方向,推导过程如下

(参考[3]De Greve, Bram. "Reflections and refractions in ray tracing." *Retrived Oct* 16 (2006): 2014.及https://www.zhihu.com/people/cjt-83-73/posts ):

(假定以下**向量都归一化**,且以下向量省略上标箭头)

<img src="lesson12_光线追踪.assets/v2-28597a488a157e3c8b8f3114db9f2bb4_1440w.jpg" alt="img" style="zoom:50%;" />

根据[斯涅尔定律](https://link.zhihu.com/?target=https%3A//zh.wikipedia.org/zh/%E6%96%AF%E6%B6%85%E5%B0%94%E5%AE%9A%E5%BE%8B)（Snell's law），介质折射率和正弦的乘积一定相等。
$$
\frac{\sin\theta_r}{\sin\theta_i}=\frac{\eta_1}{\eta_2}
$$

因$r_{\parallel}$和$i_{\parallel}$平行且指向相同方向

$$
r_{\parallel}=
\frac{|r_{\parallel}|}{|i_{\parallel}|}i_{\parallel}=
\frac{sin\theta_{r}}{sin\theta_{i}}i_{\parallel}=
\frac{\eta_{1}}{\eta_{2}}i_{\parallel}=
\frac{\eta_{1}}{\eta_{2}}(cos\theta_{i}n \cdot |i|+i) 
\stackrel{|i|=1}{=} \frac{\eta_{1}}{\eta_{2}}(cos\theta_{i}n+i)
\tag {1}
$$

根据勾股定理

$$
\left|r_\perp\right|=\sqrt{\left|r\right|^2-\left|r_\parallel\right|^2}=\sqrt{1-sin^2\theta_r}
$$

所以

$$
r_\perp=-|r_\perp|n=-\sqrt{1-sin^2\theta_r}n
$$

再根据斯涅尔定律

$$
sin^{2}\theta_{r}=(\frac{\eta_{1}}{\eta_{2}})^{2}sin^{2}\theta_{i}=(\frac{\eta_{1}}{\eta_{2}})^{2}(1-cos^{2}\theta_{i})
$$

代入得

$$
r_\perp=-\sqrt{1-(\frac{\eta_1}{\eta_2})^2(1-cos^2\theta_i)}n
\tag{2}
$$



将式子(1)和式子(2)代入

$$
\begin{aligned}\text{r}&=r_{\parallel}+r_{\perp}\\&=\frac{\eta_{1}}{\eta_{2}}(i+cos\theta_{i}n)-\sqrt{1-(\frac{\eta_{1}}{\eta_{2}})^{2}(1-cos^{2}\theta_{i})}n\\&=\frac{\eta_{1}}{\eta_{2}}i+(\frac{\eta_{1}}{\eta_{2}}cos\theta_{i}-\sqrt{1-(\frac{\eta_{1}}{\eta_{2}})^{2}(1-cos^{2}\theta_{i})})n\end{aligned}
$$

最后因为

$$cos\theta_i=-i\cdot n$$

得到

$$
r=\frac{\eta_1}{\eta_2}i-(\frac{\eta_1}{\eta_2}(i\cdot n)+\sqrt{1-(\frac{\eta_1}{\eta_2})^2(1-(i\cdot n)^2)})n
$$

需要注意的是，上述式子在根号内的值大于等于0时才成立。

当$\eta_1\leq\eta_2$ 时，根号内始终大于等于0。

当$\eta_1>\eta_2$ 时，则在特定入射角度(超过临界角)会发生全内反射的现象。此时折射分量为0.

$$
r=\begin{cases}\:\frac{\eta_1}{\eta_2}i-(\frac{\eta_1}{\eta_2}(i\cdot n)+\sqrt{1-(\frac{\eta_1}{\eta_2})^2(1-(i\cdot n)^2)})n&\text{if}\:1-(\frac{\eta_1}{\eta_2})^2(1-(i\cdot n)^2)\geq0\\\:0&\text{if}\:1-(\frac{\eta_1}{\eta_2})^2(1-(i\cdot n)^2)<0\end{cases}
$$

<img src="lesson12_光线追踪.assets/image-20241210145305786.png" alt="image-20241210145305786" style="zoom: 67%;" />

由此,我们就可以求出不出现全反射情况下的折射方向r.

但是，如图上蓝色线所示，当$\eta_1>\eta_2$ 时，在特定入射角度(超过临界角)会发生全内反射的现象。此时折射分量为0，那么我们也需要表现出这个全反射的情况。

###### 全反射情况

注意到如果$\frac{\eta_1}{\eta_2}$比较大的话，那么$sinθ_r=sin\theta_i\frac{\eta_1}{\eta_2}$有可能会出现大于1的情况（当光线从高折射律介质射入低折射率介质时, 比如从钻石类的材质射入到空气中，此时上述的Snell方程可能没有实解（因为看前面推导的公式里根号下面可能出现<0的情况）），此时就无法求解出正确的折射光线了。通过上面的公式发现本质上无解的情况应该是$sin\theta_r>1$，也就是$sin\theta_i\frac{\eta_1}{\eta_2}>1$

**解决方案：当求解出光线无法完成折射的任务时，使其反射**

**什么情况下光线会从高折射律介质射入低折射率介质？**

常常在实心物体的内部发生, 所以我们称这种情况被称为"全内反射"。比如在潜水的时候, 你发现水与空气的交界处看上去像一面镜子的原因。

可以用三角函数解出$\sin\theta$

$$
\sin \theta=\sqrt{1-\cos ^{2} \theta}
$$
其中的$cos\theta$为$$cos\theta=dot(-i,n)$$,其中i是入射光线，n是法线。



##### Schlick近似方法

现实世界中的玻璃, **发生反射的概率会随着入射角而改变**——从一个很狭窄的角度去看玻璃窗, 它会变成一面镜子。如果要完全精确地描述这件事公式是十分痛苦的（参考学习链接：https://zhuanlan.zhihu.com/p/372110183），不过有一个数学上近似的等式, 它是由Christophe Schlick提出的。Schlick 反射率函数如下：
$$
F_{\text {Schlick }}\left(n, v, F_{0}\right)=F_{0}+\left(1-F_{0}\right)(1-(n \cdot v))^{5} 
$$
其中的F0与介质的折射率有关，公式计算如下：
$$
F_{0}=\left(\frac{\eta_{1}-\eta_{2}}{\eta_{1}+\eta_{2}}\right)^{2}=\left(\frac{\eta-1}{\eta+1}\right)^{2}
$$
下面展示了用使用schlick近似方法来实现折射：



```GLSL
else if(materialType == DIELECTRIC)  // 介电质（玻璃等）材质
    {
        vec3 outward_normal;
        vec3 reflected = reflect(wo.direction, isectInfo.normal);  // 计算反射光线

        float ni_over_nt;  // 折射率

        attenuation = vec3(1.0f, 1.0f, 1.0f);  // 介电质材质的衰减一般为1
        vec3 refracted;  // 折射光线
        float reflect_prob;  // 反射概率
        float cosine;

        float rafractionIndex = isectInfo.refractionIndex;  // 介电质的折射率

        if (dot(wo.direction, isectInfo.normal) > 0.0f)  // 如果光线从物体内部出射
        {
            outward_normal = -isectInfo.normal;  // 法线方向取反
            ni_over_nt = rafractionIndex;
           
            cosine = dot(wo.direction, isectInfo.normal) / length(wo.direction);
            cosine = sqrt(1.0f - rafractionIndex * rafractionIndex * (1.0f - cosine * cosine));  // 计算折射角度
        }
        else  // 如果光线从物体外部入射
        {
            outward_normal = isectInfo.normal;
            ni_over_nt = 1.0f / rafractionIndex;
            cosine = -dot(wo.direction, isectInfo.normal) / length(wo.direction);
        }
        
        if (refractVec(wo.direction, outward_normal, ni_over_nt, refracted))  // 尝试计算折射光线
            reflect_prob = schlick(cosine, rafractionIndex);  // 计算反射概率（使用Schlick近似）
        else
            reflect_prob = 1.0f;  // 如果折射失败，反射概率为1
        
        if (rand2D() < reflect_prob)  // 根据反射概率决定是否反射
        {
            wi.origin = isectInfo.p;
            wi.direction = reflected;
        }
        else
        {
            wi.origin = isectInfo.p;
            wi.direction = refracted;
        }

        return true;
    }
```



![image-20241210152231270](lesson12_光线追踪.assets/image-20241210152231270.png)

也许大家会觉得奇怪，但是如果现实中的玻璃珠的全反射实际上也是这个效果

<img src="lesson12_光线追踪.assets/image-20241210152404499.png" alt="image-20241210152404499" style="zoom:50%;" />

​		图片来自https://pixabay.com/zh/photos/glass-sphere-glass-bullet-round-1547291/

##### 2.3.5 阴影

如果我们此时往光源（暂定一个光源）去发射一根射线，我们可以知道在这个交点往光源的方向是否会被遮挡，如果被遮挡，那么这点就在阴影中。因此这条射线也被称为shadow ray。

<img src="lesson12_光线追踪.assets/image-20241206164721524.png" alt="image-20241206164721524" style="zoom:50%;" />

​															被遮挡，交点处是阴影

<img src="lesson12_光线追踪.assets/image-20241206164741818.png" alt="image-20241206164741818" style="zoom:50%;" />

​																未被遮挡



![image-20241210154440207](lesson12_光线追踪.assets/image-20241210154440207.png)

参考文章：

[1] Real-Time Rendering 4th

[2]Wang, Zijin. "The Development of Ray Tracing and Its Future." (2022).https://ceur-ws.org/Vol-3150/short3.pdf

[3]De Greve, Bram. "Reflections and refractions in ray tracing." *Retrived Oct* 16 (2006): 2014. https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf

[4]https://www.shadertoy.com/view/lssBD7