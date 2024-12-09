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

struct GLSL_Triangle {
    vec3 p0;
    vec3 p1;
    vec3 p2;
    vec3 normal;
};

struct GLSL_Quad {
    vec3 p0;
    vec3 p1;
    vec3 p2;
    vec3 p3;
    vec3 normal;
};

struct GLSL_AABB {
    vec3 min;
    vec3 max;
};

struct GLSL_BVHNode {
    GLSL_AABB aabb;
    int left_node_index;
    int right_node_index;
    int vertex_index;
    int vertex_count;
};

struct GLSL_RasterizationCamera {
    vec3 position;
    mat4 view_matrix;
    mat4 projection_matrix;
};

struct GLSL_PhysicallyBasedCamera {
    vec3 position;
    vec3 target;
    vec3 forward;
    vec3 right;
    vec3 up;
    vec2 resolution;
    float focal_length;
    float image_sensor_width;
    float image_sensor_height;
};

struct GLSL_Material {
    vec4 albedo;
    int brdf_type;
    vec3 kd;
    vec3 le;
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
    vec3 p0;
    vec3 p1;
    vec3 p2;
    vec3 p3;
    mat4 space_matrix;
    vec3 le;
    int entity_id;
};