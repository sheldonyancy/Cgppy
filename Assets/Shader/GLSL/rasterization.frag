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
#include "global_ubo.glsl"

layout(set = 2, binding = 1) uniform sampler2D shadow_mapping;

layout(location = 0) in struct dto {
    vec3 position;
    vec3 normal;
    vec4 color;
    vec4 shadow_coord;
} in_dto;

layout(location = 0) out vec4 out_colour;

vec4 illuminationBlinnPhong() {
    vec4 result_color = vec4(0.0, 0.0, 0.0, 1.0);

    if(0.0 != in_dto.color.w) {
        vec3 light_color = vec3(1.0);
        vec3 light_dir = normalize(global_ubo.object.light.center - in_dto.position);
        vec3 view_dir = normalize(global_ubo.object.rasterization_camera.position.xyz - in_dto.position);

        vec3 material_diffuse_color = in_dto.color.xyz;

        //
        vec3 halfway_dir = normalize(light_dir + view_dir);
        float spec = pow(max(dot(in_dto.normal, halfway_dir), 0.0), SHININESS);
        vec3 specular = spec * SPECULAR_STRENGTH * light_color;

        //
        vec3 diffuse = max(dot(in_dto.normal, light_dir), 0.0) * light_color * material_diffuse_color;

        //
        vec3 ambient = AMBIENT_STRENGTH * material_diffuse_color;

        //
        float shadow = 1.0;
        vec3 shadow_coord = in_dto.shadow_coord.xyz / in_dto.shadow_coord.w;
        if ( shadow_coord.z > -1.0 && shadow_coord.z < 1.0 ){
            float dist = texture(shadow_mapping, shadow_coord.st).r;
            if (dist + 0.001 < shadow_coord.z) {
                shadow = AMBIENT_STRENGTH;
            }
        }

        //
        result_color = vec4(shadow * (specular + diffuse + ambient), 1.0);
    } else {
        result_color = vec4(in_dto.color.xyz, 1.0);
    }

    return result_color;
}

void main() {
    out_colour = illuminationBlinnPhong();
}