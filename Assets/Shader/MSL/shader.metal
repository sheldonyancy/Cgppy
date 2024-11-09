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

#include <metal_stdlib>
using namespace metal;

struct YsVertexPosition {
    float3 position [[attribute(0)]];
};
struct YsVertexNormal {
    float3 normal [[attribute(0)]];
};

struct YsUniformModel {
    float4x4 model_matrix;
};

struct YsUniformCamera {
    float4x4 view_matrix;
    float4x4 projection_matrix;
};

struct YsVertexOut {
    float4 position [[position]];
    float3 normal;
};

//
vertex YsVertexOut vertex_main(uint vertex_index [[vertex_id]],
                               constant YsVertexPosition* vertex_postition [[buffer(0)]],
                               constant YsVertexNormal* vertex_normal [[buffer(1)]],
                               constant YsUniformModel* u_model [[buffer(2)]],
                               constant YsUniformCamera* u_camera [[buffer(3)]]) {
    YsVertexOut out;
    out.position = u_camera->projection_matrix * u_model->model_matrix * vector_float4(vertex_postition[vertex_index].position, 1.0);
    out.normal = vertex_normal[vertex_index].normal;

    return out;
}

fragment float4 fragment_main(YsVertexOut interpolated [[stage_in]]) {
    float3 material_diffuse_color = float3(1.0, 0.0, 0.0);

    float3 camera_pos = float3(0.0, 0.0, 100.0);
    float3 light_pos = camera_pos;
    float3 light_color = float3(1.0, 1.0, 1.0);
    float3 ambient_color = float3(0.1, 0.1, 0.1) * material_diffuse_color;
    float specular_strength = 0.5;
    float shininess = 32.0;

    float3 light_dir = normalize(light_pos - interpolated.position.xyz);
    float3 view_dir = normalize(camera_pos - interpolated.position.xyz);

    float diff = max(dot(interpolated.normal, light_dir), 0.0);
    float3 diffuse = diff * light_color * material_diffuse_color;

    float3 halfway_dir = normalize(light_dir + view_dir);
    float spec = pow(max(dot(interpolated.normal, halfway_dir), 0.0), shininess);
    float3 specular = spec * specular_strength * light_color;

    float3 result_color = ambient_color + diffuse + specular;
    return float4(result_color, 1.0);
}