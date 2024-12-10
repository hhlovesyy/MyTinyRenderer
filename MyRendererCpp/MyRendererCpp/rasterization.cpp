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
bool cull_back(vec3 ndc_coords[3])
{
	vec3 a = ndc_coords[0];
	vec3 b = ndc_coords[1];
	vec3 c = ndc_coords[2];

	vec3 ab = b - a;
	vec3 bc = c - b;

	float cross = ab[0] * bc[1] - ab[1] * bc[0];
	return cross <= 0;
}
bool cull_back_efficient(vec3 ndc_coords[3])
{
	vec3 a = ndc_coords[0];
	vec3 b = ndc_coords[1];
	vec3 c = ndc_coords[2];

	float signed_area =
		a[0] * b[1] - a[1] * b[0] +
		b[0] * c[1] - b[1] * c[0] +
		c[0] * a[1] - c[1] * a[0];
	return signed_area <= 0;
}

static int is_inside_plane(vec4 coord, plane_t plane) {
	switch (plane) {
	case POSITIVE_W:
		return coord[3] >= EPSILON;
	case POSITIVE_X:
		return coord[0] <= +coord[3];
	case NEGATIVE_X:
		return coord[0] >= -coord[3];
	case POSITIVE_Y:
		return coord[1] <= +coord[3];
	case NEGATIVE_Y:
		return coord[1] >= -coord[3];
	case POSITIVE_Z:
		return coord[2] <= +coord[3];
	case NEGATIVE_Z:
		return coord[2] >= -coord[3];
	default:
		assert(0);
		return 0;
	}
}

static float get_intersect_ratio(vec4 prev, vec4 curr, plane_t plane) {
	switch (plane) {
	case POSITIVE_W:
		return (prev[3] - EPSILON) / (prev[3] - curr[3]);
	case POSITIVE_X:
		return (prev[3] - prev[0]) / ((prev[3] - prev[0]) - (curr[3] - curr[0]));
	case NEGATIVE_X:
		return (prev[3] + prev[0]) / ((prev[3] + prev[0]) - (curr[3] + curr[0]));
	case POSITIVE_Y:
		return (prev[3] - prev[1]) / ((prev[3] - prev[1]) - (curr[3] - curr[1]));
	case NEGATIVE_Y:
		return (prev[3] + prev[1]) / ((prev[3] + prev[1]) - (curr[3] + curr[1]));
	case POSITIVE_Z:
		return (prev[3] - prev[2]) / ((prev[3] - prev[2]) - (curr[3] - curr[2]));
	case NEGATIVE_Z:
		return (prev[3] + prev[2]) / ((prev[3] + prev[2]) - (curr[3] + curr[2]));
	default:
		assert(0);
		return 0;
	}
}

static int clip_against_plane(
	plane_t plane, int in_num_vertices, int varying_num_floats,
	vec4 in_coords[MAX_VARYINGS], void* in_varyings[MAX_VARYINGS],
	vec4 out_coords[MAX_VARYINGS], void* out_varyings[MAX_VARYINGS]) {
	int out_num_vertices = 0;
	int i, j;

	assert(in_num_vertices >= 3 && in_num_vertices <= MAX_VARYINGS);
	for (i = 0; i < in_num_vertices; i++) {
		int prev_index = (i - 1 + in_num_vertices) % in_num_vertices;
		int curr_index = i;
		vec4 prev_coord = in_coords[prev_index];
		vec4 curr_coord = in_coords[curr_index];
		float* prev_varyings = (float*)in_varyings[prev_index];
		float* curr_varyings = (float*)in_varyings[curr_index];
		int prev_inside = is_inside_plane(prev_coord, plane);
		int curr_inside = is_inside_plane(curr_coord, plane);

		if (prev_inside != curr_inside) {
			vec4* dest_coord = &out_coords[out_num_vertices];
			float* dest_varyings = (float*)out_varyings[out_num_vertices];
			float ratio = get_intersect_ratio(prev_coord, curr_coord, plane);

			*dest_coord = lerp(prev_coord, curr_coord, ratio);
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
			vec4* dest_coord = &out_coords[out_num_vertices];
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


static int is_vertex_visible(vec4 v) {
	return fabs(v[0]) <= v[3] && fabs(v[1]) <= v[3] && fabs(v[2]) <= v[3];
}

static int clip_triangle(
	int sizeof_varyings,
	vec4 in_coords[MAX_VARYINGS], void* in_varyings[MAX_VARYINGS],
	vec4 out_coords[MAX_VARYINGS], void* out_varyings[MAX_VARYINGS]) {
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
		vec4 clip_position = program->vertex_shader_(program->shader_attribs_[i], program->in_varyings_[i], program->get_uniforms());
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
		vec4 clip_coords[3];
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
	vec4 clip_coords[3], void* varyings[3])
{
	int width = framebuffer->width;
	int height = framebuffer->height;
	vec3 ndc_coords[3];
	vec2 screen_coords[3];
	float screen_depths[3];
	float recip_w[3];
	int backface;
	bbox_t bbox;
	int i, x, y;
	/* perspective division */
	for (i = 0; i < 3; i++) 
	{
		vec3 clip_coord = vec3_from_vec4(clip_coords[i]);
		ndc_coords[i] = clip_coord /clip_coords[i][3];
	}
	/* back-face culling */
	backface = cull_back_efficient(ndc_coords);
	if (backface && !program->double_sided) {
		return 1;
	}
	/* reciprocals of w */
	for (i = 0; i < 3; i++)
	{
		recip_w[i] = 1 / clip_coords[i][3];
	}

	/* viewport mapping */
	for (i = 0; i < 3; i++)
	{
		vec3 window_coord = viewport_transform(width, height, ndc_coords[i]);
		screen_coords[i] = vec2{ window_coord[0], window_coord[1]};
		screen_depths[i] = window_coord[2];
	}

	/* perform rasterization */
	bbox = find_bounding_box(screen_coords, width, height);
	for (x = bbox.min_x; x <= bbox.max_x; x++)
	{
		for (y = bbox.min_y; y <= bbox.max_y; y++)
		{
			vec2 point = vec2{ (float)x + 0.5f, (float)y + 0.5f };
			vec3 weights = calculate_weights(screen_coords, point);
			bool weight0_okay = weights[0] > -EPSILON;
			bool weight1_okay = weights[1] > -EPSILON;
			bool weight2_okay = weights[2] > -EPSILON;
			if (weight0_okay && weight1_okay && weight2_okay)
			{
				int index = y * width + x;
				float depth = interpolate_depth(screen_depths, weights);
				/* early depth testing */
				if (depth <= framebuffer->depth_buffer[index])
				{
					//插值
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
