#include "rasterization.h"
#include <iostream>

//用于背面剔除
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

void rasterization_tri(Mesh* mesh,Program* program, framebuffer_t* framebuffer,bool isDrawShadowMap)
{
	std::vector<Mesh::Vertex> vertices = mesh->getVertices();
	int num_faces = mesh->getNumFaces();
	
	for (int index = 0; index < num_faces; index++)
	{
		vec3_t abc_3d[3] =
		{
			vec3_new(vertices[index * 3].position.x, vertices[index * 3].position.y, vertices[index * 3].position.z),
			vec3_new(vertices[index * 3 + 1].position.x, vertices[index * 3 + 1].position.y, vertices[index * 3 + 1].position.z),
			vec3_new(vertices[index * 3 + 2].position.x, vertices[index * 3 + 2].position.y, vertices[index * 3 + 2].position.z)
		};
		//纹理坐标
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
			ndc_coords[i] = vec3_div(clip_coord, clip_position.w);  //这一步是做透视除法
			vec3_t window_coord = viewport_transform(WINDOW_WIDTH, WINDOW_HEIGHT, ndc_coords[i]); //这一步是做视口变换
			screen_coords[i] = vec2_new(window_coord.x, window_coord.y);
			screen_depths[i] = window_coord.z;
			light_depth[i] = varyings.depth_position;
		}
		//背面剔除
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
				//Zbuffer test 深度测试
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
					////如果是不透明物体，进行深度写入；如果是透明物体，不进行深度写入
					if(program->alpha_blend == 0)
					{
						//深度写入
						framebuffer->depth_buffer[screen_index] = depth;
					}					
				}
				else
				{
					continue;
				}

				vec3_t new_weights = interpolate_varyings_weights(result, recip_w);

				//对UV做重心插值，而不是对颜色做重心插值
				vec2_t uv_p = vec2_add(vec2_add(vec2_mul(uv[0], new_weights.x), vec2_mul(uv[1], new_weights.y)), vec2_mul(uv[2], new_weights.z));

				//--
				// 对法线做重心插值
				vec3_t normal_p = vec3_add(vec3_add(vec3_mul(normal[0], new_weights.x), vec3_mul(normal[1], new_weights.y)), vec3_mul(normal[2], new_weights.z));

				//对切线进行重心插值
				vec4_t tangent_p = vec4_add(vec4_add(vec4_mul(vertices[index * 3].tangent, new_weights.x), vec4_mul(vertices[index * 3 + 1].tangent, new_weights.y)), vec4_mul(vertices[index * 3 + 2].tangent, new_weights.z));

				//对副切线进行重心插值
				vec3_t bitangent_p = vec3_cross(normal_p, vec3_from_vec4(tangent_p));

				// 对位置做重心插值
				vec3_t position_p = vec3_add(vec3_add(vec3_mul(abc_3d[0], new_weights.x), vec3_mul(abc_3d[1], new_weights.y)), vec3_mul(abc_3d[2], new_weights.z));

				vec3_t current_light_depth=	vec3_add(vec3_add(vec3_mul(light_depth[0], new_weights.x), vec3_mul(light_depth[1], new_weights.y)), vec3_mul(light_depth[2], new_weights.z));

				
				// 传递插值后的属性给片元着色器
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