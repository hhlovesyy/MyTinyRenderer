//将这段代码复制到shadertoy网站上即可运行

#define PI 			3.1415926535  // 圆周率常数
#define MAXFLOAT	99999.99      // 最大浮动值

// 可以更改这些参数以提高质量（但会降低帧率 :-)
#define MAXDEPTH 	500         // 最大递归深度
#define NUMSAMPLES 	30         // 每像素采样数
#define ROTATION	false      // 是否启用旋转

// 光线结构体
struct Ray
{
    vec3 origin;     // 光线起点
    vec3 direction;  // 光线方向
};

// 相交信息结构体
struct IntersectInfo
{
    // 表面属性
    float t;          // 相交点的距离
    vec3  p;          // 相交点位置
    vec3  normal;     // 法向量
	
    // 材质属性
    int   materialType;     // 材质类型
    vec3  albedo;           // 反射率
    float fuzz;             // 模糊度
    float refractionIndex;  // 折射率
};
    
// 球体结构体
struct Sphere
{
    // 球体属性
    vec3 center;     // 球心
    float radius;    // 半径

    // 材质属性
    int   materialType;     // 材质类型
    vec3  albedo;           // 反射率
    float fuzz;             // 模糊度
    float refractionIndex;  // 折射率
};

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



// The 'scene'
Sphere sceneList[] = Sphere[](
Sphere(//地面
vec3( 0.000000, -1000.000000, 0.000000), 1000.000000, 0, 
vec3( 151.0/255.0, 181.0/255.0, 155.0/255.0), 1.000000, 1.000000),

//Sphere(//右边的球
//vec3( 4.000000, 1.000000, 0.000000), 1.000000, 2,
//vec3( 0.800000, 0.800000, 0.800000), 0.200000, 1.000000),

Sphere(//右边的大球
vec3( 4.000000, 1.000000, 0.000000), 1.000000, 2,
vec3( 0.800000, 0.800000, 0.800000), 0.200000, 1.50000),

//Sphere(vec3( 4.500000, 0.500000, 2.200000), 0.500000, 0, vec3( 156.0/255.0, 157.0/255.0, 45.0/255.0), 0.200000, 1.000000)
//Sphere(vec3( 4.500000, 0.500000, 1.900000), 0.500000, 0, vec3( 156.0/255.0, 157.0/255.0, 45.0/255.0), 0.200000, 0.600000),
//Sphere(vec3( 4.000000, 1.000000, 1.200000), 1.200000, 2, vec3( 0.800000, 0.800000, 0.800000), 1.000000, 0.900000),


Sphere(vec3( 0.000000, 0.600000, 0.400000), 0.600000, 0, vec3( 156.0/255.0, 157.0/255.0, 45.0/255.0), 0.200000, 1.300000),
//青色球
Sphere(vec3( 1.300000, 0.300000, 2.000000), 0.600000, 1, vec3( 45.0/255.0, 157.0/255.0, 145.0/255.0), 0.500000, 1.500000),
//青色球
Sphere(vec3( 1.300000, 1.100000, 1.200000), 0.500000, 1, vec3( 245.0/255.0, 237.0/255.0, 145.0/255.0), 0.00000, 1.500000),

//青色球
Sphere(vec3( 5.900000, 0.400000, 0.100000), 0.400000, 1, vec3( 45.0/255.0, 157.0/255.0, 145.0/255.0), 0.200000, 1.500000)
);

// Schlick's approximation for approximating the contribution of the Fresnel factor
// in the specular reflection of light from a non-conducting surface between two media
//
// Theta 是入射光方向和两种介质界面法线之间的角度
float schlick(float cos_theta, float n2)
{
    const float n1 = 1.0f;  // 空气的折射率

    float r0s = (n1 - n2) / (n1 + n2);  // 计算反射系数 r0
    float r0 = r0s * r0s;  // r0 的平方

    // 使用 Schlick 近似公式来计算反射光的贡献
    return r0 + (1.0f - r0) * pow((1.0f - cos_theta), 5.0f);
}


// 折射向量的计算
bool refractVec(vec3 v, vec3 n, float ni_over_nt, out vec3 refracted)
{
    vec3 uv = normalize(v);  // 单位化入射光方向

    float dt = dot(uv, n);  // 计算入射光与法线的点积

    float discriminant = 1.0 - ni_over_nt * ni_over_nt * (1.0f - dt * dt);  // 判别式

    // 如果判别式大于零，说明折射光存在
    if (discriminant > 0.0f)
    {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);  // 计算折射光

        return true;
    }
    else
        return false;  // 否则没有折射光
}


// 计算反射光向量
vec3 reflectVec(vec3 v, vec3 n)
{
     return v - 2.0f * dot(v, n) * n;  // 使用反射公式
}


// 随机数生成器
vec2 randState;

float hash(const float n) 
{
     return fract(sin(n) * 43758.54554213);  // 使用正弦函数生成伪随机数
}


// 生成 2D 随机数
float rand2D()
{
    randState.x = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);  // 生成 x 分量
    randState.y = fract(sin(dot(randState.xy, vec2(12.9898, 78.233))) * 43758.5453);  // 生成 y 分量
    
    return randState.x;
}


// 生成单位球内的随机方向（用于 Lambert BRDF）
vec3 random_in_unit_sphere()
{
    float phi = 2.0 * PI * rand2D();  // 随机角度
    float cosTheta = 2.0 * rand2D() - 1.0;  // 随机余弦值
    float u = rand2D();  // 随机数

    float theta = acos(cosTheta);  // 计算极角
    float r = pow(u, 1.0 / 3.0);  // 随机半径

    float x = r * sin(theta) * cos(phi);  // 计算x分量
    float y = r * sin(theta) * sin(phi);  // 计算y分量
    float z = r * cos(theta);  // 计算z分量

    return vec3(x, y, z);  // 返回随机向量
}


// 生成单位圆盘内的随机点（用于景深相机效果）
vec3 random_in_unit_disk()
{
    float spx = 2.0 * rand2D() - 1.0;  // 随机 x 坐标
    float spy = 2.0 * rand2D() - 1.0;  // 随机 y 坐标

    float r, phi;

    // 确定 r 和 phi 的值来保证生成的点在单位圆盘内
    if (spx > -spy)
    {
        if (spx > spy)
        {
            r = spx;
            phi = spy / spx;
        }
        else
        {
            r = spy;
            phi = 2.0 - spx / spy;
        }
    }
    else
    {
        if (spx < spy)
        {
            r = -spx;
            phi = 4.0f + spy / spx;
        }
        else
        {
            r = -spy;
            if (spy != 0.0)
                phi = 6.0 - spx / spy;
            else
                phi = 0.0;
        }
    }

    phi *= PI / 4.0;  // 转换为弧度

    return vec3(r * cos(phi), r * sin(phi), 0.0f);  // 返回随机点
}


// 摄像机结构体定义
struct Camera
{
    vec3 origin;  // 摄像机原点
    vec3 lowerLeftCorner;  // 图像的左下角
    vec3 horizontal;  // 水平方向的向量
    vec3 vertical;  // 垂直方向的向量
    vec3 u, v, w;  // 摄像机坐标系的三个基向量
    float lensRadius;  // 镜头半径，用于景深
};
    
// 初始化摄像机
void Camera_init(out Camera camera, vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focusDist)
{
    camera.lensRadius = aperture / 2.0;  // 设置镜头半径
    
    float theta = vfov * PI / 180.0;  // 将竖直视场角转为弧度
    float halfHeight = tan(theta / 2.0);  // 计算视野的一半高度
    float halfWidth = aspect * halfHeight;  // 根据宽高比计算视野的一半宽度

    camera.origin = lookfrom;  // 摄像机位置

    camera.w = normalize(lookfrom - lookat);  // 计算视线方向
    camera.u = normalize(cross(vup, camera.w));  // 计算水平方向
    camera.v = cross(camera.w, camera.u);  // 计算垂直方向

    // 计算左下角位置和视野的水平、垂直向量
    camera.lowerLeftCorner = camera.origin - halfWidth * focusDist * camera.u
                                           - halfHeight * focusDist * camera.v
                                           - focusDist * camera.w;

    camera.horizontal = 2.0 * halfWidth * focusDist * camera.u;
    camera.vertical = 2.0 * halfHeight * focusDist * camera.v;
}

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

//镜面反射
vec3 m_reflect(vec3 v, vec3 n)
{
    return v - 2.0 * dot(v, n) * n;
}

// 定义材质类型
#define LAMBERT    0  // 漫反射材质
#define METAL      1  // 金属材质
#define DIELECTRIC 2  // 电介质材质

// 物体材质的color，根据材质类型（漫反射、金属、介电质）进行光的反射和折射计算
bool Material_color(IntersectInfo isectInfo, Ray wo, out Ray wi, out vec3 attenuation)
{
    int materialType = isectInfo.materialType;

    if(materialType == LAMBERT)  // 漫反射材质
    {
        // 计算一个随机散射方向
        vec3 target = isectInfo.p + isectInfo.normal + random_in_unit_sphere();

        wi.origin = isectInfo.p;  // 散射光线的起始位置为交点
        wi.direction = target - isectInfo.p;  // 散射光线的方向

        attenuation = isectInfo.albedo;  // 反射衰减（反射率）

        return true;
    }
    else if(materialType == METAL)  // 金属材质
    {
        float fuzz = isectInfo.fuzz;  // 金属表面的粗糙度（模糊程度）

        // 计算反射光线的方向
        vec3 reflected = m_reflect(normalize(wo.direction), isectInfo.normal);

        wi.origin = isectInfo.p;
        wi.direction = reflected + fuzz * random_in_unit_sphere();  // 加上模糊程度的随机扰动

        attenuation = isectInfo.albedo;

        return (dot(wi.direction, isectInfo.normal) > 0.0f);  // 确保反射光线朝向表面
    }
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

    return false;  // 如果没有匹配的材质类型，则返回false
}


// 场景中的物体是否与光线相交的检测函数
bool intersectScene(Ray ray, float t_min, float t_max, out IntersectInfo rec)
{
        IntersectInfo temp_rec;

        bool hit_anything = false;
        float closest_so_far = t_max;  // 记录最近的交点

        // 遍历场景中的所有物体进行相交测试
        for (int i = 0; i < sceneList.length(); i++)
        {
            Sphere sphere = sceneList[i];

            // 如果当前物体与光线相交
            if (Sphere_hit(sphere, ray, t_min, closest_so_far, temp_rec))
            {
                hit_anything   = true;
                closest_so_far = temp_rec.t;
                rec            = temp_rec;  // 更新交点信息
            }
        }

        return hit_anything;  // 如果有物体与光线相交，返回true
}

// 计算天空背景色（基于光线方向）
vec3 skyColor(Ray ray)
{
    vec3 unit_direction = normalize(ray.direction);
    float t = 0.5 * (unit_direction.y + 1.0);  // 计算光线方向的y分量

    // 根据t的值线性插值，返回渐变的天空颜色
    return (1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.8, 0.8, 1.0);
}


// 计算光线的辐射（颜色）
vec3 radiance(Ray ray)
{
    IntersectInfo rec;

    vec3 col = vec3(1.0, 1.0, 1.0);  // 初始颜色为白色

    for(int i = 0; i < MAXDEPTH; i++)  // 最大递归深度
    {
        if (intersectScene(ray, 0.001, MAXFLOAT, rec))  // 如果光线与场景相交
        {
            Ray wi;
            vec3 attenuation;

            bool wasScattered = Material_color(rec, ray, wi, attenuation);  // 根据材质计算散射

            ray.origin = wi.origin;
            ray.direction = wi.direction;

            if (wasScattered)  // 如果光线被散射
                col *= attenuation;  // 更新颜色
            else  // 如果光线没有散射
            {
                col *= vec3(0.0f, 0.0f, 0.0f);  // 颜色为黑色
                break;
            }
        }
        else  // 如果光线没有与任何物体相交
        {
            col *= skyColor(ray);  // 使用天空颜色
            break;
        }
    }

    return col;  // 返回最终颜色
}


// 计算并输出最终的像素颜色
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{   
    vec3 lookfrom = vec3(13.0, 2.0, 3.0);  // 相机位置
    const vec3 lookat = vec3(0.0, 0.0, 0.0);  // 目标位置
    float distToFocus = 10.0;  // 焦点距离
    float aperture = 0.1;  // 相机光圈

    // 如果启用了旋转效果，旋转相机
    if(ROTATION)
    {
        float angle = iTime / 2.0;  // 根据时间计算旋转角度
    	mat4 rotationMatrix = mat4(cos(angle), 0.0, sin(angle), 0.0,
                                          0.0, 1.0,        0.0, 0.0,
                                 -sin(angle),  0.0, cos(angle), 0.0,
                                         0.0,  0.0,        0.0, 1.0);
    
    	lookfrom = vec3(rotationMatrix * vec4(lookfrom, 1.0));  // 更新相机位置
    }
    
    Camera camera;
    Camera_init(camera, lookfrom, lookat, vec3(0.0f, 1.0f, 0.0f), 20.0f, float(iResolution.x) / float(iResolution.y), aperture, distToFocus);  // 初始化相机

    randState = fragCoord.xy / iResolution.xy;  // 计算随机种子
    
    vec3 col = vec3(0.0, 0.0, 0.0);  // 初始化颜色为黑色

    for (int s = 0; s < NUMSAMPLES; s++)  // 多次采样，减少噪点
    {
        float u = float(fragCoord.x + rand2D()) / float(iResolution.x);  // 计算屏幕坐标的u分量
        float v = float(fragCoord.y + rand2D()) / float(iResolution.y);  // 计算屏幕坐标的v分量

        Ray r = Camera_getRay(camera, u, v);  // 获取相机射出的光线

        col += radiance(r);  // 根据光线计算辐射（颜色）

    }

    col /= float(NUMSAMPLES);  // 计算平均颜色
    col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );  // 色彩伽马校正

    fragColor = vec4(col, 1.0);  // 输出最终的像素颜色
}