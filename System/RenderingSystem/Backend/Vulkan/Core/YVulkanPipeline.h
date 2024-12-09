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

#ifndef CGPPY_YVULKANPIPELINE_H
#define CGPPY_YVULKANPIPELINE_H


#include "YVulkanTypes.h"
#include "YVulkanResource.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkShaderStageConfig {
    VkShaderStageFlagBits stage_flag;
    u32 source_length;
    u32* source;
} YsVkShaderStageConfig;

typedef struct YsVkShaderStage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} YsVkShaderStage;

typedef struct YsVkShaderConfig {
    u8 shader_stage_config_count;
    YsVkShaderStageConfig shader_stage_config[VULKAN_SHADER_MAX_STAGES];
} YsVkShaderConfig;

typedef struct YsVkShader {
    u32 id;
    u8 stage_count;
    YsVkShaderStage* stages;
} YsVkShader;

typedef struct YsVkPipelineConfig {
    YsVkShaderConfig shader_config;

    struct YsVkRenderStage* render_stage;

    VkViewport viewport;
    VkRect2D scissor;

    VkPipelineVertexInputStateCreateInfo* vertex_input_info;

    u32 descriptor_set_layout_count;
    VkDescriptorSetLayout* descriptor_set_layouts;

    u32 push_constant_range_count;
    VkPushConstantRange* push_constant_range;
} YsVkPipelineConfig;

typedef struct YsVkPipeline {
    b8 (*create)(struct YsVkContext* context,
                         YsVkPipelineConfig* config,
                         struct YsVkPipeline* out_pipeline);

    void (*destroy)(struct YsVkContext* context, struct YsVkPipeline* pipeline);

    YsVkPipelineConfig* config;
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
    YsVkShader* shader;
} YsVkPipeline;

YsVkPipeline* yVkAllocatePipelineObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANPIPELINE_H
