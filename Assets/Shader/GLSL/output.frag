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
#include "global_scene_block.glsl"

layout(set = 2, binding = 0) uniform sampler2D rasterization_texture;
layout(set = 2, binding = 4) uniform sampler2DArray accumulate_textures;

layout(location = 0) in struct dto {
    vec2 texcoord;
} in_dto;

layout(location = 0) out vec4 out_colour;

void main() {
    switch(global_ubo.object.rendering_model) {
        case 0: {
            vec3 color0 = texture(accumulate_textures, vec3(in_dto.texcoord, 0.0)).xyz;
            vec3 color1 = texture(accumulate_textures, vec3(in_dto.texcoord, 1.0)).xyz;
            vec3 color2 = texture(accumulate_textures, vec3(in_dto.texcoord, 2.0)).xyz;

            vec3 color = (color0 + color1 + color2) / global_ubo.object.samples;

            out_colour = vec4(pow(color, vec3(0.4545)), 1.0);
            break;
        }
        case 1: {
            out_colour = vec4(texture(rasterization_texture, in_dto.texcoord).xyz, 1.0);
            break;
        }
    }
}