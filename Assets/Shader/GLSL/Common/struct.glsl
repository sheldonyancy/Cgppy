/**
 * MIT License
 *
 * Copyright (c) 2024 Sheldon Yancy
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if defined(__VERSION__)
    #define GLSL_STRUCT(name) struct name
#else
    #define GLSL_STRUCT(name) name
#endif



struct GLSL_Triangle {
    vec3 p0;
    float _padding1;
    vec3 p1;
    float _padding2;
    vec3 p2;
    float _padding3;
    vec3 normal;
    float _padding4;
};

struct GLSL_Quad {
    vec3 p0;
    float _padding1;
    vec3 p1;
    float _padding2;
    vec3 p2;
    float _padding3;
    vec3 p3;
    float _padding4;
    vec3 normal;
    float _padding5;
};

struct GLSL_AABB {
    vec3 min;
    float _padding1;
    vec3 max;
    float _padding2;
};

struct GLSL_BVHNode {
    GLSL_STRUCT(GLSL_AABB) aabb;
    int left_node_index;
    int right_node_index;
    int vertex_index;
    int vertex_count;
};

struct GLSL_RasterizationCamera {
    vec4 position;
    mat4 view_matrix;
    mat4 projection_matrix;
};

struct GLSL_PhysicallyBasedCamera {
    vec3 position;
    float _padding1;
    vec3 target;
    float _padding2;
    vec3 forward;
    float _padding3;
    vec3 right;
    float _padding4;
    vec3 up;
    float _padding5;
    vec2 resolution;
    float focal_length;
    float image_sensor_width;
    float image_sensor_height;
    float _padding6;
};

struct GLSL_Material {
    vec4 albedo;
    int brdf_type;
    float _padding1[3];
    vec3 kd;
    float _padding2;
    vec3 le;
    float _padding3;
};

struct GLSL_Ray {
    vec3 origin;
    vec3 direction;
};

struct GLSL_HitInfo {
    float t;
    int index;
};

struct GLSL_IntersectInfo {
    bool hit;
    float t;
    vec3 hit_pos;
    vec3 hit_normal;
    vec3 dpdu;
    vec3 dpdv;
    int material_id;
    int entity_id;
};

struct GLSL_Light {
    vec3 center;
    float _padding1;
    vec3 p0;
    float _padding2;
    vec3 p1;
    float _padding3;
    vec3 p2;
    float _padding4;
    vec3 p3;
    float _padding5;
    mat4 space_matrix;
    vec3 le;
    float _padding6;
    int entity_id;
    float _padding7[3];
};

struct GLSL_GlobalUniformObject {
    int rendering_model;
    int accumulate_image_index;
    int samples;
    GLSL_STRUCT(GLSL_RasterizationCamera) rasterization_camera;
    GLSL_STRUCT(GLSL_PhysicallyBasedCamera) physically_based_camera;
    GLSL_STRUCT(GLSL_Light) light;
};

struct GLSL_PushConstantsObject {
    mat4 model_matrix;
};

struct GLSL_GlobalSceneBlock{
    int bvh_node_count;
    int material_count;
    int vertex_count;
    float _padding1;
    GLSL_STRUCT(GLSL_BVHNode) bvh_node[64];
    GLSL_STRUCT(GLSL_Material) materials[32];
    vec4 vertex_position[256];
    vec4 vertex_normal[256];
    int vertex_material_id[256];
    int vertex_entity_id[256];
};