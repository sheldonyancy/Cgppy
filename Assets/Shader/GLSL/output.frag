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


#include "struct.glsl"
#include "push_constant_object.glsl"
#include "uniform_buffer_object.glsl"
#include "uniform_sampler_rasterization.glsl"
#include "uniform_sampler_path_tracing.glsl"

layout(location = 0) in struct dto {
    vec2 texcoord;
} in_dto;

layout(location = 0) out vec4 out_colour;

void main() {
    switch(ubo.rendering_model) {
        case 0: {
            out_colour = vec4(texture(uniform_path_tracing_sampler, in_dto.texcoord).xyz, 1.0);
            break;
        }
        case 1: {
            out_colour = vec4(texture(uniform_rasterization_sampler, in_dto.texcoord).xyz, 1.0);
            break;
        }
    }
}