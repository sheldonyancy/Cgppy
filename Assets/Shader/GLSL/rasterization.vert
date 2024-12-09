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

#version 460

#extension GL_ARB_separate_shader_objects : enable

#include "define.glsl"
#include "struct.glsl"
#include "uniform_buffer_object.glsl"
#include "stroage_buffer_object.glsl"
#include "push_constant_object.glsl"

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec4 in_normal;
layout(location = 2) in int in_material_id;

layout(location = 0) out struct dto {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec4 shadow_coord;
} out_dto;

const mat4 bias_mat = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 1.0, 0.0,
0.5, 0.5, 0.0, 1.0);

void main() {
    out_dto.position = in_position.xyz;
    out_dto.normal = in_normal.xyz;
    out_dto.color = ssbo.materials[in_material_id].albedo;
    out_dto.shadow_coord = bias_mat
                          *ubo.light.space_matrix
                          *ubo.model_matrix * vec4(in_position.xyz, 1.0);

    gl_Position = ubo.rasterization_camera.projection_matrix
                 *ubo.rasterization_camera.view_matrix
                 *ubo.model_matrix * vec4(in_position.xyz, 1.0);
}
