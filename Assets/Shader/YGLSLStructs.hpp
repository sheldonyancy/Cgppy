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


#ifndef CGPPY_YGLSLSTRUCTS_HPP
#define CGPPY_YGLSLSTRUCTS_HPP


#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct alignas(16) GLSL_AABB {
    glm::fvec3 min;
    alignas(16) glm::fvec3 max;
};

struct alignas(16) GLSL_BVHNode {
    GLSL_AABB aabb;
    int left_node_index;
    int right_node_index;
    int vertex_index;
    int vertex_count;
};

struct alignas(16) GLSL_Material {
    glm::fvec4 albedo;
    int brdf_type;
    alignas(16) glm::fvec3 kd;
    alignas(16) glm::fvec3 le;
};

struct alignas(16) GLSL_RasterizationCamera {
    glm::fvec3 position;
    alignas(16) glm::fmat4x4 view_matrix;
    glm::fmat4x4 projection_matrix;
};

struct alignas(16) GLSL_PhysicallyBasedCamera {
    glm::fvec3 position;
    alignas(16) glm::fvec3 target;
    alignas(16) glm::fvec3 forward;
    alignas(16) glm::fvec3 right;
    alignas(16) glm::fvec3 up;
    alignas(16) glm::fvec2 resolution;
    float focal_length;
    float image_sensor_width;
    float image_sensor_height;
};

struct alignas(16) GLSL_Light {
    glm::fvec3 center;
    alignas(16) glm::fvec3 p0;
    alignas(16) glm::fvec3 p1;
    alignas(16) glm::fvec3 p2;
    alignas(16) glm::fvec3 p3;
    alignas(16) glm::fmat4x4 space_matrix;
    glm::fvec3 le;
    int entity_id;
};

struct alignas(16) GLSL_SSBO {
    int bvh_node_count;
    int material_count;
    int vertex_count;
    GLSL_BVHNode bvh_node[64];
    GLSL_Material materials[32];
    glm::fvec4 vertex_position[256];
    glm::fvec4 vertex_normal[256];
    int vertex_material_id[256];
    int vertex_entity_id[256];
};

struct alignas(16) GLSL_UBO{
    int rendering_model;
    int path_tracing_spp;
    int path_tracing_max_depth;
    alignas(16) glm::fmat4x4 model_matrix;
    GLSL_RasterizationCamera rasterization_camera;
    GLSL_PhysicallyBasedCamera physically_based_camera;
    GLSL_Light light;
};

struct alignas(16) GLSL_PushConstantObject {
    int current_present_image_index;
    int current_frame;
};


#endif 