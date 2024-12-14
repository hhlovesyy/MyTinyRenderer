#define MAX_BOUNCES 50
#define pi 3.1415926536

float gSeed = 0.0;

uint baseHash(uvec2 p)
{
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y>>3U));
    return h32 ^ (h32 >> 16);
}

float hash1(inout float seed) {
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1,seed += 0.1)));
    return float(n) / float(0xffffffffU);
}

vec2 hash2(inout float seed) {
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1,seed += 0.1)));
    uvec2 rz = uvec2(n, n * 48271U);
    return vec2(rz.xy & uvec2(0x7fffffffU)) / float(0x7fffffff);
}

vec3 hash3(inout float seed)
{
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1, seed += 0.1)));
    uvec3 rz = uvec3(n, n * 16807U, n * 48271U);
    return vec3(rz & uvec3(0x7fffffffU)) / float(0x7fffffff);
}


struct Ray 
{
    vec3 o;     // origin
    vec3 d;     // direction - always set with normalized vector
    float t;    // time
};

Ray createRay(vec3 o, vec3 d, float t)
{
    Ray r;
    r.o = o;
    r.d = d;
    r.t = t;
    return r;
}

Ray createRay(vec3 o, vec3 d)
{
    return createRay(o, d, 0.0);
}

struct Camera
{
    vec3 origin;
    vec3 lowerLeftCorner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u, v, w;
    float lensRadius;
    float time0, time1;
};


// MT_ material type
#define MT_DIFFUSE 0
#define MT_METAL 1
#define MT_DIALECTRIC 2

struct Material
{
    int type;
    vec3 albedo;
    float roughness; // controls roughness for metals
    float refIdx; // index of refraction for dialectric
};


struct HitRecord
{
    vec3 pos;
    vec3 normal;
    float t;            // ray parameter
    Material material;
};

struct Sphere
{
    vec3 center;
    float radius;
};

Material createDiffuseMaterial(vec3 albedo)
{
    Material m;
    m.type = MT_DIFFUSE;
    m.albedo = albedo;
    return m;
}

Material createMetalMaterial(vec3 albedo, float roughness)
{
    Material m;
    m.type = MT_METAL;
    m.albedo = albedo;
    m.roughness = roughness;
    return m;
}

Material createDialectricMaterial(vec3 albedo, float refIdx)
{
    Material m;
    m.type = MT_DIALECTRIC;
    m.albedo = albedo;
    m.refIdx = refIdx;
    return m;
}


Sphere createSphere(vec3 center, float radius)
{
    Sphere s;
    s.center = center;
    s.radius = radius;
    return s;
}


Camera createCamera(
    vec3 lookFrom,
    vec3 lookAt,
    vec3 worldUp,
    float fovy,
    float aspect,
    float aperture,
    float focusDist,
    float time0,
    float time1)
{
    float theta = fovy * pi / 180.0;
    float halfHeight = tan(theta * 0.5);
    float halfWidth = aspect * halfHeight;

    Camera cam;
    cam.lensRadius = aperture * 0.5;
    cam.origin = lookFrom;
    cam.w = normalize(lookFrom - lookAt);
    cam.u = normalize(cross(worldUp, cam.w));
    cam.v = cross(cam.w, cam.u);
    
    cam.lowerLeftCorner = cam.origin - halfWidth * focusDist * cam.u - halfHeight * focusDist * cam.v - cam.w * focusDist;
    cam.horizontal = 2.0 * halfWidth * focusDist * cam.u;
    cam.vertical = 2.0 * halfHeight * focusDist * cam.v;
    cam.time0 = time0;
    cam.time1 = time1;
    return cam;
}

vec3 pointOnRay(Ray r, float t)
{
    return r.o + r.d * t;
}

bool hit_sphere(Sphere s, Ray r, float tmin, float tmax, inout HitRecord rec)
{
    vec3 oc = r.o - s.center;
    float a = dot(r.d, r.d);
    float b = dot(oc, r.d);
    float c = dot(oc, oc) - s.radius * s.radius;
    float discriminant = b * b - a * c;
    if(discriminant > 0.0)
    {
        float sqrtDiscriminant = sqrt(discriminant);
        float temp = (-b - sqrtDiscriminant) / a;
        if(temp < tmax && temp > tmin)
        {
            rec.t = temp;
            rec.pos = pointOnRay(r, rec.t);
            rec.normal = (rec.pos - s.center) / s.radius;
            return true;
        }
        temp = (-b + sqrtDiscriminant) / a;
        if(temp < tmax && temp > tmin)
        {
            rec.t = temp;
            rec.pos = pointOnRay(r, rec.t);
            rec.normal = (rec.pos - s.center) / s.radius;
            return true;
        }
    }
    return false;
}

vec2 randomInUnitDisk(inout float seed)
{
    vec2 h = hash2(seed) * vec2(1.0, 6.28318530718);
    float phi = h.y;
    float r = sqrt(h.x);
	return r * vec2(sin(phi), cos(phi));
}

vec3 randomInUnitSphere(inout float seed)
{
    vec3 h = hash3(seed) * vec3(2.0, 6.28318530718, 1.0) - vec3(1.0, 0.0, 0.0);
    float phi = h.y;
    float r = pow(h.z, 1.0/3.0);
	return r * vec3(sqrt(1.0 - h.x * h.x) * vec2(sin(phi), cos(phi)), h.x);
}

bool customRefract(vec3 v, vec3 n, float niOverNt, out vec3 refracted)
{
    vec3 uv = normalize(v);
    float dt = dot(uv, n);
    float discriminant = 1.0 - niOverNt * niOverNt * (1.0 - dt * dt);
    if(discriminant > 0.0)
    {
        refracted = niOverNt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    }
    return false;
}

float schlick(float cosine, float refIdx)
{
    float r0 = (1.0 - refIdx) / (1.0 + refIdx);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow((1.0 - cosine), 5.0);
}


bool scatter(Ray rIn, HitRecord rec, out vec3 atten, out Ray rScattered)
{
    if(rec.material.type == MT_DIFFUSE)
    {
        vec3 target = rec.pos + rec.normal + randomInUnitSphere(gSeed);
        rScattered = createRay(rec.pos, normalize(target - rec.pos), rIn.t);
        atten = rec.material.albedo;
        return true;
    }
    if(rec.material.type == MT_METAL)
    {
        vec3 refl = reflect(rIn.d, rec.normal);
        rScattered = createRay(rec.pos, normalize(refl + rec.material.roughness * randomInUnitSphere(gSeed)), rIn.t);
        atten = rec.material.albedo;
        return dot(rScattered.d, rec.normal) > 0.0;
    }
    if(rec.material.type == MT_DIALECTRIC)
    {
        atten = vec3(1.0);
        vec3 outwardNormal;
        float niOverNt;
        float cosine;
        if(dot(rIn.d, rec.normal) > 0.0)  //说明光线从内部打出去
        {
            outwardNormal = -rec.normal;
            niOverNt = rec.material.refIdx; //默认空气的折射率为1
            cosine = dot(rIn.d, rec.normal) / length(rIn.d); 
            cosine = sqrt(1.0f - rec.material.refIdx * rec.material.refIdx * (1.0f - cosine * cosine));  // 计算折射角度
        }
        else
        {
            outwardNormal = rec.normal;
            niOverNt = 1.0 / rec.material.refIdx;
            cosine = -dot(rIn.d, rec.normal) / length(rIn.d); 
        }
        vec3 refracted;
        float reflectProb;
        if(customRefract(rIn.d, outwardNormal, niOverNt, refracted))
        {
            reflectProb = schlick(cosine, rec.material.refIdx);
        }
        else //全内反射的情况，一定走反射逻辑
        {
            reflectProb = 1.0;
        }

        if(hash1(gSeed) < reflectProb) //走反射逻辑
        {
            vec3 reflected = reflect(rIn.d, rec.normal);
            rScattered = createRay(rec.pos, normalize(reflected), rIn.t);
        }
        else  //走折射逻辑
        {
            rScattered = createRay(rec.pos, normalize(refracted), rIn.t);
        }

        return true;
    }

}


Ray getRay(Camera cam, vec2 uv)
{
    vec2 rd = cam.lensRadius * randomInUnitDisk(gSeed);
    vec3 offset = cam.u * rd.x + cam.v * rd.y;  //模拟景深/模糊效果，光线起始点随机偏移一点
    float time = cam.time0 + hash1(gSeed) * (cam.time1 - cam.time0);
    return createRay(
        cam.origin + offset,
        normalize(cam.lowerLeftCorner + uv.x * cam.horizontal + uv.y * cam.vertical - cam.origin - offset),
        time); 
}

bool hit_world(Ray r, float tmin, float tmax, inout HitRecord rec)
{
    bool hit = false;
    rec.t = tmax;

    if(hit_sphere(
        createSphere(vec3(0.0, -1000.0, -1.0), 1000.0),
        r,
        tmin,
        rec.t,
        rec))
    {
        hit = true;
        rec.material = createDiffuseMaterial(vec3(0.5));
    }

    if(hit_sphere(
        createSphere(vec3(-4.0, 1.0, 0.0), 1.0),
        r,
        tmin,
        rec.t,
        rec))
    {
        hit = true;
        rec.material = createDiffuseMaterial(vec3(0.4, 0.2, 0.1));
    }

    if(hit_sphere(
        createSphere(vec3(4.0, 1.0, 0.0), 1.0),
        r,
        tmin,
        rec.t,
        rec))
    {
        hit = true;
        rec.material = createMetalMaterial(vec3(0.7, 0.6, 0.5), 0.0);
    }

    if(hit_sphere(
        createSphere(vec3(0.0, 1.0, 0.0), 1.0),
        r,
        tmin,
        rec.t,
        rec))
    {
        hit = true;
        rec.material.type = MT_DIALECTRIC;
        rec.material = createDialectricMaterial(vec3(0.8, 0.8, 0.8), 1.5);
    }

    if(hit_sphere(
        createSphere(vec3(0.0, 1.0, 0.0), -0.95),
        r,
        tmin,
        rec.t,
        rec))
    {
        hit = true;
        rec.material.type = MT_DIALECTRIC;
        rec.material = createDialectricMaterial(vec3(0.8, 0.8, 0.8), 1.5);
    }

    int numxy = 5;
    for(int x = -numxy; x < numxy; ++x) //随机生成很多球
    {
        for(int y = -numxy; y < numxy; ++y)
        {
            float fx = float(x);
            float fy = float(y);
            float seed = fx + fy / 1000.0;
            vec3 rand1 = hash3(seed);
            vec3 center = vec3(fx + 0.9 * rand1.x, 0.2, fy + 0.9 * rand1.y);
            float chooseMaterial = rand1.z;
            if(distance(center, vec3(4.0, 0.2, 0.0)) > 0.9)
            {
                if(chooseMaterial < 0.4)
                {
                    vec3 center1 = center + vec3(0.0, hash1(gSeed) * 0.5, 0.0);
                    // diffuse
                    if(hit_sphere(
                        createSphere(center, 0.2),
                        r,
                        tmin,
                        rec.t,
                        rec))
                    {
                        hit = true;
                        rec.material = createDiffuseMaterial(hash3(seed) * hash3(seed));
                    }
                }
                else if(chooseMaterial < 0.6)
                {
                    // diffuse
                    if(hit_sphere(
                        createSphere(center, 0.2),
                        r,
                        tmin,
                        rec.t,
                        rec))
                    {
                        hit = true;
                        rec.material = createDiffuseMaterial(hash3(seed) * hash3(seed));
                    }
                }
                else if(chooseMaterial < 0.9)
                {
                    // metal
                    if(hit_sphere(
                        createSphere(center, 0.2),
                        r,
                        tmin,
                        rec.t,
                        rec))
                    {
                        hit = true;
                        rec.material.type = MT_METAL;
                        rec.material = createMetalMaterial((hash3(seed) + 1.0) * 0.5, hash1(seed));
                    }
                }
                else
                {
                    // glass (dialectric)
                    if(hit_sphere(
                        createSphere(center, 0.2),
                        r,
                        tmin,
                        rec.t,
                        rec))
                    {
                        hit = true;
                        rec.material.type = MT_DIALECTRIC;
                        rec.material = createDialectricMaterial(vec3(0.0), 1.5);
                    }
                }
            }
        }
    }

    return hit;
}

vec3 color(Ray r)
{
    HitRecord rec;
    vec3 col = vec3(1.0);
    for(int i = 0; i < MAX_BOUNCES; ++i)
    {
        if(hit_world(r, 0.001, 10000.0, rec))
        {
            Ray scatterRay;
            vec3 atten;
            if(scatter(r, rec, atten, scatterRay))
            {
                col *= atten;
                r = scatterRay;
            }
            else
            {
                return vec3(0.0);
            }
        }
        else  //只是返回背景色
        {
            float t = 0.5 * (r.d.y + 1.0);
            col *= mix(vec3(1.0), vec3(0.5, 0.7, 1.0), t);
            return col;
        }
    }
    return col;
}

vec3 toLinear(vec3 c)
{
    return pow(c, vec3(2.2));
}

vec3 toGamma(vec3 c)
{
    return pow(c, vec3(1.0 / 2.2));
}


#define MAX_SAMPLES 5000.0
void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    gSeed = float(baseHash(floatBitsToUint(gl_FragCoord.xy))) / float(0xffffffffU) + iTime; //随机一个seed，暂不需关心细节
    vec2 mouse = iMouse.xy / iResolution.xy;
    mouse.x = mouse.x * 2.0 - 1.0;

    vec3 camPos = vec3(mouse.x * 10.0, mouse.y * 5.0, 8.0);
    vec3 camTarget = vec3(0.0, 0.0, -1.0);
    float fovy = 20.0;
    float aperture = 0.0;
    float distToFocus = length(camTarget - camPos);
    float time0 = 0.0;
    float time1 = 0.0; //暂时先设定time1=time0=0.0
    Camera cam = createCamera(
        camPos,
        camTarget,
        vec3(0.0, 1.0, 0.0),    // world up vector
        fovy,
        iResolution.x / iResolution.y,
        aperture,
        distToFocus,
        time0,
        time1);
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec4 prev = texture(iChannel0, uv);
    prev.xyz *= prev.w;
    
    uv = (gl_FragCoord.xy + hash2(gSeed)) / iResolution.xy; //在当前像素的基础上随机加一点扰动
    
    vec3 col = color(getRay(cam, uv));
    vec4 mousedata = texelFetch(iChannel1, ivec2(fragCoord), 0);
    if(mousedata.x - mousedata.z != 0.0 || mousedata.y - mousedata.w != 0.0)
    {
        fragColor = vec4(col, 1.0);
        return;
    }
    if(prev.w > MAX_SAMPLES)
    {
        fragColor = prev;
        return;
    }
    col += prev.xyz;
    float w = prev.w + 1.0;
    col /= w;
    fragColor = vec4(col, w);
}

