#include "rasterization.h"
#include "graphics.h"
#include <iostream>

typedef enum {
	POSITIVE_W,
	POSITIVE_X,
	NEGATIVE_X,
	POSITIVE_Y,
	NEGATIVE_Y,
	POSITIVE_Z,
	NEGATIVE_Z
} plane_t;

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

static int is_inside_plane(vec4_t coord, plane_t plane) {
	switch (plane) {
	case POSITIVE_W:
		return coord.w >= EPSILON;
	case POSITIVE_X:
		return coord.x <= +coord.w;
	case NEGATIVE_X:
		return coord.x >= -coord.w;
	case POSITIVE_Y:
		return coord.y <= +coord.w;
	case NEGATIVE_Y:
		return coord.y >= -coord.w;
	case POSITIVE_Z:
		return coord.z <= +coord.w;
	case NEGATIVE_Z:
		return coord.z >= -coord.w;
	default:
		assert(0);
		return 0;
	}
}

static float get_intersect_ratio(vec4_t prev, vec4_t curr, plane_t plane) {
	switch (plane) {
	case POSITIVE_W:
		return (prev.w - EPSILON) / (prev.w - curr.w);
	case POSITIVE_X:
		return (prev.w - prev.x) / ((prev.w - prev.x) - (curr.w - curr.x));
	case NEGATIVE_X:
		return (prev.w + prev.x) / ((prev.w + prev.x) - (curr.w + curr.x));
	case POSITIVE_Y:
		return (prev.w - prev.y) / ((prev.w - prev.y) - (curr.w - curr.y));
	case NEGATIVE_Y:
		return (prev.w + prev.y) / ((prev.w + prev.y) - (curr.w + curr.y));
	case POSITIVE_Z:
		return (prev.w - prev.z) / ((prev.w - prev.z) - (curr.w - curr.z));
	case NEGATIVE_Z:
		return (prev.w + prev.z) / ((prev.w + prev.z) - (curr.w + curr.z));
	default:
		assert(0);
		return 0;
	}
}

static int clip_against_plane(
	plane_t plane, int in_num_vertices, int varying_num_floats,
	vec4_t in_coords[MAX_VARYINGS], void* in_varyings[MAX_VARYINGS],
	vec4_t out_coords[MAX_VARYINGS], void* out_varyings[MAX_VARYINGS]) {
	int out_num_vertices = 0;
	int i, j;

	assert(in_num_vertices >= 3 && in_num_vertices <= MAX_VARYINGS);
	for (i = 0; i < in_num_vertices; i++) {
		int prev_index = (i - 1 + in_num_vertices) % in_num_vertices;
		int curr_index = i;
		vec4_t prev_coord = in_coords[prev_index];
		vec4_t curr_coord = in_coords[curr_index];
		float* prev_varyings = (float*)in_varyings[prev_index];
		float* curr_varyings = (float*)in_varyings[curr_index];
		int prev_inside = is_inside_plane(prev_coord, plane);
		int curr_inside = is_inside_plane(curr_coord, plane);

		if (prev_inside != curr_inside) {
			vec4_t* dest_coord = &out_coords[out_num_vertices];
			float* dest_varyings = (float*)out_varyings[out_num_vertices];
			float ratio = get_intersect_ratio(prev_coord, curr_coord, plane);

			*dest_coord = vec4_lerp(prev_coord, curr_coord, ratio);
			/*
			 * since this computation is performed in clip space before
			 * division by w, clipped varying values are perspective-correct
			 */
			for (j = 0; j < varying_num_floats; j++) {
				dest_varyings[j] = float_lerp(prev_varyings[j],
					curr_varyings[j],
					ratio);
			}
			out_num_vertices += 1;
		}

		if (curr_inside) {
			vec4_t* dest_coord = &out_coords[out_num_vertices];
			float* dest_varyings = (float*)out_varyings[out_num_vertices];
			int sizeof_varyings = varying_num_floats * sizeof(float);

			*dest_coord = curr_coord;
			memcpy(dest_varyings, curr_varyings, sizeof_varyings);
			out_num_vertices += 1;
		}
	}
	assert(out_num_vertices <= MAX_VARYINGS);
	return out_num_vertices;
}

#define CLIP_IN2OUT(plane)                                                  \
    do {                                                                    \
        num_vertices = clip_against_plane(                                  \
            plane, num_vertices, varying_num_floats,                        \
            in_coords, in_varyings, out_coords, out_varyings);              \
        if (num_vertices < 3) {                                             \
            return 0;                                                       \
        }                                                                   \
    } while (0)

#define CLIP_OUT2IN(plane)                                                  \
    do {                                                                    \
        num_vertices = clip_against_plane(                                  \
            plane, num_vertices, varying_num_floats,                        \
            out_coords, out_varyings, in_coords, in_varyings);              \
        if (num_vertices < 3) {                                             \
            return 0;                                                       \
        }                                                                   \
    } while (0)


static int is_vertex_visible(vec4_t v) {
	return fabs(v.x) <= v.w && fabs(v.y) <= v.w && fabs(v.z) <= v.w;
}

static int clip_triangle(
	int sizeof_varyings,
	vec4_t in_coords[MAX_VARYINGS], void* in_varyings[MAX_VARYINGS],
	vec4_t out_coords[MAX_VARYINGS], void* out_varyings[MAX_VARYINGS]) {
	int v0_visible = is_vertex_visible(in_coords[0]);
	int v1_visible = is_vertex_visible(in_coords[1]);
	int v2_visible = is_vertex_visible(in_coords[2]);
	if (v0_visible && v1_visible && v2_visible) {
		out_coords[0] = in_coords[0];
		out_coords[1] = in_coords[1];
		out_coords[2] = in_coords[2];
		memcpy(out_varyings[0], in_varyings[0], sizeof_varyings);
		memcpy(out_varyings[1], in_varyings[1], sizeof_varyings);
		memcpy(out_varyings[2], in_varyings[2], sizeof_varyings);
		return 3;
	}
	else {
		int varying_num_floats = sizeof_varyings / sizeof(float);
		int num_vertices = 3;
		CLIP_IN2OUT(POSITIVE_W);
		CLIP_OUT2IN(POSITIVE_X);
		CLIP_IN2OUT(NEGATIVE_X);
		CLIP_OUT2IN(POSITIVE_Y);
		CLIP_IN2OUT(NEGATIVE_Y);
		CLIP_OUT2IN(POSITIVE_Z);
		CLIP_IN2OUT(NEGATIVE_Z);
		return num_vertices;
	}
}

void graphics_draw_triangle(framebuffer_t* framebuffer, Program* program)
{
	int num_vertices;
	for (int i = 0; i < 3; i++)  //走一遍顶点着色器
	{
		vec4_t clip_position = program->vertex_shader_(program->shader_attribs_[i], program->in_varyings_[i], program->get_uniforms());
		program->in_coords[i] = clip_position;  //保存顶点着色器的输出,指的是三个顶点的裁剪空间坐标
	}
	/* triangle clipping */
	num_vertices = clip_triangle(program->sizeof_varyings_,
		program->in_coords, program->in_varyings_,
		program->out_coords, program->out_varyings_);
	
	/* triangle assembly */
	for (int i = 0; i < num_vertices - 2; i++) {
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;
		vec4_t clip_coords[3];
		void* varyings[3];
		int is_culled = 0;

		clip_coords[0] = program->out_coords[index0];
		clip_coords[1] = program->out_coords[index1];
		clip_coords[2] = program->out_coords[index2];
		varyings[0] = program->out_varyings_[index0];
		varyings[1] = program->out_varyings_[index1];
		varyings[2] = program->out_varyings_[index2];

		is_culled = rasterize_triangle(framebuffer, program,
			clip_coords, varyings);
		if (is_culled) {
			break;
		}
	}
}

int rasterize_triangle(framebuffer_t* framebuffer, Program* program,
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
	return 0;  //bug！！！之前没写return 0，问题很严重，而且不会报错，太美了C++
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