#include "rasterization.h"
#include "graphics.h"
#include <iostream>

//���ڱ����޳�
bool cull_back(vec3_t ndc_coords[3])
{
	vec3_t a = ndc_coords[0];
	vec3_t b = ndc_coords[1];
	vec3_t c = ndc_coords[2];

	vec3_t ab = vec3_sub(b, a);
	vec3_t bc = vec3_sub(c, b);

	float cross = ab.x * bc.y - ab.y * bc.x;
	return cross <= 0;
}
bool cull_back_efficient(vec3_t ndc_coords[3])
{
	vec3_t a = ndc_coords[0];
	vec3_t b = ndc_coords[1];
	vec3_t c = ndc_coords[2];

	float signed_area =
		a.x * b.y - a.y * b.x +
		b.x * c.y - b.y * c.x +
		c.x * a.y - c.y * a.x;
	return signed_area <= 0;
}

void graphics_draw_triangle(framebuffer_t* framebuffer, Program* program)
{
	int num_vertices;
	for (int i = 0; i < 3; i++)  //��һ�鶥����ɫ��
	{
		vec4_t clip_position = blinnphong_vertex_shader(program->shader_attribs_[i], program->in_varyings_[i], program->get_uniforms());
		program->in_coords[i] = clip_position;  //���涥����ɫ�������,ָ������������Ĳü��ռ�����
	}
	//���������Ȳ����ǲü�����Ϊ������ⲽ��һ����
	for (int i = 0; i < 3; i++) //���ü��޳�
	{
		program->out_coords[i] = program->in_coords[i];
		program->out_varyings_[i] = program->in_varyings_[i];
	}
	
	vec4_t clip_coords[3];
	void* varyings[3];
	for (int i = 0; i < 3; i++)  // ���������á���������դ��
	{
		clip_coords[i] = program->out_coords[i];
		varyings[i] = program->out_varyings_[i];
	}
	int is_culled;
	is_culled = rasterize_triangle(framebuffer, program,
		clip_coords, varyings);
	//if(is_culled) //����к����������Բ���һ��
}

static int rasterize_triangle(framebuffer_t* framebuffer, Program* program,
	vec4_t clip_coords[3], void* varyings[3])
{
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec3_t ndc_coords[3];
	vec2_t screen_coords[3];
	float screen_depths[3];
	float recip_w[3];
	int backface;
	bbox_t bbox;
	int i, x, y;
	/* perspective division */
	for (i = 0; i < 3; i++) 
	{
		vec3_t clip_coord = vec3_from_vec4(clip_coords[i]);
		ndc_coords[i] = vec3_div(clip_coord, clip_coords[i].w);
	}
	/* back-face culling */
	backface = cull_back_efficient(ndc_coords);
	if (backface && !program->double_sided) {
		return 1;
	}
	/* reciprocals of w */
	for (i = 0; i < 3; i++)
	{
		recip_w[i] = 1 / clip_coords[i].w;
	}

	/* viewport mapping */
	for (i = 0; i < 3; i++)
	{
		vec3_t window_coord = viewport_transform(width, height, ndc_coords[i]);
		screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
		screen_depths[i] = window_coord.z;
	}

	/* perform rasterization */
	bbox = find_bounding_box(screen_coords, width, height);
	for (x = bbox.min_x; x <= bbox.max_x; x++)
	{
		for (y = bbox.min_y; y <= bbox.max_y; y++)
		{
			vec2_t point = vec2_new((float)x + 0.5f, (float)y + 0.5f);
			vec3_t weights = calculate_weights(screen_coords, point);
			bool weight0_okay = weights.x > -EPSILON;
			bool weight1_okay = weights.y > -EPSILON;
			bool weight2_okay = weights.z > -EPSILON;
			if (weight0_okay && weight1_okay && weight2_okay)
			{
				int index = y * width + x;
				float depth = interpolate_depth(screen_depths, weights);
				/* early depth testing */
				if (depth <= framebuffer->depth_buffer[index])
				{
					interpolate_varyings(varyings, program->shader_varyings_,
						program->sizeof_varyings_,
						weights, recip_w);
					draw_fragment_new(framebuffer, program, backface, index, depth);
				}
			}
		}
	}

}

void rasterization_tri(Mesh* mesh,Program* program, framebuffer_t* framebuffer,bool isDrawShadowMap)
{
	std::vector<Vertex> vertices = mesh->getVertices();
	int num_faces = mesh->getNumFaces();
	
	for (int index = 0; index < num_faces; index++)
	{
		vec3_t abc_3d[3] =
		{
			vec3_new(vertices[index * 3].position.x, vertices[index * 3].position.y, vertices[index * 3].position.z),
			vec3_new(vertices[index * 3 + 1].position.x, vertices[index * 3 + 1].position.y, vertices[index * 3 + 1].position.z),
			vec3_new(vertices[index * 3 + 2].position.x, vertices[index * 3 + 2].position.y, vertices[index * 3 + 2].position.z)
		};
		//��������
		vec2_t uv[3] =
		{
			vec2_new(vertices[index * 3].texcoord.x, vertices[index * 3].texcoord.y),
			vec2_new(vertices[index * 3 + 1].texcoord.x, vertices[index * 3 + 1].texcoord.y),
			vec2_new(vertices[index * 3 + 2].texcoord.x, vertices[index * 3 + 2].texcoord.y)
		};
		vec3_t normal[3] =
		{
			vec3_new(vertices[index * 3].normal.x, vertices[index * 3].normal.y, vertices[index * 3].normal.z),
			vec3_new(vertices[index * 3 + 1].normal.x, vertices[index * 3 + 1].normal.y, vertices[index * 3 + 1].normal.z),
			vec3_new(vertices[index * 3 + 2].normal.x, vertices[index * 3 + 2].normal.y, vertices[index * 3 + 2].normal.z)
		};

		vec4_t clip_abc[3];
		vec3_t ndc_coords[3];

		vec2_t screen_coords[3];
		float screen_depths[3];
		vec3_t light_depth[3];

		float recip_w[3];

		varyings_blinnphong varyings;
		for (int i = 0; i < 3; i++)
		{
			//--
			attribs_blinnphong attribs;
			attribs.position = abc_3d[i];
			attribs.texcoord = uv[i];
			attribs.normal = normal[i];
			attribs.joint = vertices[index * 3 + i].joint;
			attribs.weight = vertices[index * 3 + i].weight;
			attribs.tangent = vertices[index * 3 + i].tangent;
			
			uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(program->get_uniforms());
			uniforms->isDrawShadowMap = isDrawShadowMap;
			vec4_t clip_position = blinnphong_vertex_shader(&attribs, &varyings, program->get_uniforms());
			recip_w[i] = 1 / clip_position.w;

			vec3_t clip_coord = vec3_from_vec4(clip_position);
			ndc_coords[i] = vec3_div(clip_coord, clip_position.w);  //��һ������͸�ӳ���
			vec3_t window_coord = viewport_transform(WINDOW_WIDTH, WINDOW_HEIGHT, ndc_coords[i]); //��һ�������ӿڱ任
			screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
			screen_depths[i] = window_coord.z;
			light_depth[i] = varyings.depth_position;
		}
		//�����޳�
		if(cull_back_efficient(ndc_coords))
		{
			continue;
		}

		bbox_t bbox = find_bounding_box(screen_coords, WINDOW_WIDTH, WINDOW_HEIGHT);


		for (int i = bbox.min_x; i <= bbox.max_x; i++)
		{
			for (int j = bbox.min_y; j <= bbox.max_y; j++)
			{
				vec2_t p{ (float)(i + 0.5), (float)(j + 0.5) };
				vec3_t result = calculate_weights(screen_coords, p);
				if (!(result.x > 0 && result.y > 0 && result.z > 0)) continue;
				float depth = interpolate_depth(screen_depths, result);
				int screen_index = j * WINDOW_WIDTH + i;
				//Zbuffer test ��Ȳ���
				//std::cout<< screen_index << std::endl;
				/*if(screen_index>= WINDOW_WIDTH* WINDOW_HEIGHT)
				{
					std::cout << "error" << std::endl;
					return;
				}
				std::cout << screen_index << std::endl;*/
				if (depth < framebuffer->depth_buffer[screen_index])
				{
					framebuffer->depth_buffer[screen_index] = depth;
					////����ǲ�͸�����壬�������д�룻�����͸�����壬���������д��
					if(program->alpha_blend == 0)
					{
						//���д��
						framebuffer->depth_buffer[screen_index] = depth;
					}					
				}
				else
				{
					continue;
				}

				vec3_t new_weights = interpolate_varyings_weights(result, recip_w);

				//��UV�����Ĳ�ֵ�������Ƕ���ɫ�����Ĳ�ֵ
				vec2_t uv_p = vec2_add(vec2_add(vec2_mul(uv[0], new_weights.x), vec2_mul(uv[1], new_weights.y)), vec2_mul(uv[2], new_weights.z));

				//--
				// �Է��������Ĳ�ֵ
				vec3_t normal_p = vec3_add(vec3_add(vec3_mul(normal[0], new_weights.x), vec3_mul(normal[1], new_weights.y)), vec3_mul(normal[2], new_weights.z));

				//�����߽������Ĳ�ֵ
				vec4_t tangent_p = vec4_add(vec4_add(vec4_mul(vertices[index * 3].tangent, new_weights.x), vec4_mul(vertices[index * 3 + 1].tangent, new_weights.y)), vec4_mul(vertices[index * 3 + 2].tangent, new_weights.z));

				//�Ը����߽������Ĳ�ֵ
				vec3_t bitangent_p = vec3_cross(normal_p, vec3_from_vec4(tangent_p));

				// ��λ�������Ĳ�ֵ
				vec3_t position_p = vec3_add(vec3_add(vec3_mul(abc_3d[0], new_weights.x), vec3_mul(abc_3d[1], new_weights.y)), vec3_mul(abc_3d[2], new_weights.z));

				vec3_t current_light_depth=	vec3_add(vec3_add(vec3_mul(light_depth[0], new_weights.x), vec3_mul(light_depth[1], new_weights.y)), vec3_mul(light_depth[2], new_weights.z));

				
				// ���ݲ�ֵ������Ը�ƬԪ��ɫ��
				varyings_blinnphong varyings;
				varyings.texcoord = uv_p;
				varyings.normal = normal_p;
				varyings.world_position = position_p;
				varyings.world_tangent = vec3_from_vec4(tangent_p);
				varyings.world_bitangent = bitangent_p;
				varyings.depth_position = current_light_depth;


				//--
				uniforms_blinnphong* uniforms = static_cast<uniforms_blinnphong*>(program->get_uniforms());
				uniforms->isDrawShadowMap = isDrawShadowMap;
				vec4_t finalColor = blinnphong_fragment_shader(&varyings, program->get_uniforms(), nullptr, 0);
				//draw_fragment(framebuffer, j * width + i, finalColor1);
				draw_fragment(framebuffer, j * WINDOW_WIDTH + i, finalColor, program);

			}
		}
	}
}