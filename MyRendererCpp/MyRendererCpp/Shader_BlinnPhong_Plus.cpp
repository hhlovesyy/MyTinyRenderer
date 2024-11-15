#include "maths.h"
#include "shader_BlinnPhong.h"
#include "shader_BlinnPhong.cpp"


class Shader_BlinnPhong_Plus
{
    public:
    //暂时存储渲染shadowmap所需要的顶点/片元着色器

    vec4_t shadowmap_vertex_shader(attribs_blinnphong* attribs,
        varyings_blinnphong* varyings,
        uniforms_blinnphong* uniforms)
    {
        //渲染shdowmap时，只需要计算顶点的在光源视角下的深度（z值）
        mat4_t model_matrix = get_model_matrix(attribs, uniforms);
        mat4_t light_vp_matrix = uniforms->light_vp_matrix;

        vec4_t input_position = vec4_from_vec3(attribs->position, 1);
        vec4_t world_position = mat4_mul_vec4(model_matrix, input_position);
        vec4_t depth_position = mat4_mul_vec4(light_vp_matrix, world_position);

        varyings->texcoord = attribs->texcoord;
        return depth_position;
    }


    vec4_t shadowmap_fragment_shader(varyings_blinnphong* varyings,
        uniforms_blinnphong* uniforms,
        int* discard,
        int backface)
    {
        //sample texture
        Material_BlinnPhong material = get_material(varyings, uniforms, backface);
        vec3_t color = material.emission;
        if (uniforms->ambient_intensity > 0)
        {
            vec3_t ambient = material.diffuse;  //kd/ka
            float intensity = uniforms->ambient_intensity; //Ia
            color = vec3_add(color, vec3_mul(ambient, intensity));
        }
        //lambert and blinn_phong
        vec3_t light_dir = vec3_normalize(uniforms->light_dir); //注意Light方向要归一化

        float n_dot_l = vec3_dot(material.normal, light_dir);
        //half lambert
        //n_dot_l = n_dot_l * 0.5 + 0.5;
        if (n_dot_l < 0)
        {
            n_dot_l = 0;
        }
        //vec3_t test = vec3_new(n_dot_l, n_dot_l, n_dot_l);
        vec3_t view_dir = get_view_dir(varyings, uniforms);
        vec3_t specular = get_specular(light_dir, view_dir, material);
        vec3_t diffuse = vec3_mul(material.diffuse, n_dot_l);
        color = vec3_add(color, vec3_add(diffuse, specular));
        //diffuse = material.diffuse;
        vec3_t ndotL = vec3_new(n_dot_l, n_dot_l, n_dot_l);
        return vec4_from_vec3(color, material.alpha);
    }
};