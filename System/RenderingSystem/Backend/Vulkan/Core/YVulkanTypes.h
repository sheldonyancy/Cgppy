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

#ifndef CGPPY_YVULKANSTRUCT_HPP
#define CGPPY_YVULKANSTRUCT_HPP


#include "YDefines.h"

#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_macos.h>
#include <vulkan/vk_enum_string_helper.h>


struct YsVkContext;
struct YsVkDevice;
struct YsVkSwapchain;
struct YsVkImage;
struct YsVkRenderStage;
struct YsVkBuffer;
struct YsVkResources;
struct YsVkPipeline;
struct YsVkRenderingSystem;
struct YsVkOutputSystem;
struct YsVkRasterizationSystem;
struct YsVkShadowMappingSystem;
struct YsVkPathTracingSystem;

typedef struct YsVkDescriptor {
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet* descriptor_sets;
    u32 set;
    u8 is_single_descriptor_set;
} YsVkDescriptor;

typedef struct YsVkCommandUnit {
    u32 queue_family_index;
    u32 queue_index;
    VkQueue queue;
    u32 command_pool_count;
    VkCommandPool* command_pools;
    u32 command_buffer_count;
    VkCommandBuffer* command_buffers;
    VkQueryPool query_pool_timestamps;
}YsVkCommandUnit;

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
    VkStructureType pipeline_type;

    YsVkShaderConfig shader_config;

    struct YsVkRenderStage* render_stage;

    VkViewport viewport;
    VkRect2D scissor;

    VkPipelineVertexInputStateCreateInfo* vertex_input_info;

    u32 descriptor_count;
    YsVkDescriptor* descriptors;

    u32 push_constant_range_count;
    VkPushConstantRange* push_constant_range;
} YsVkPipelineConfig;

typedef struct YsSubpassConfig {
    u8 reference_count;
    VkAttachmentReference* attachment_references;

    VkSubpassDescription subpass_description;
} YsSubpassConfig;

typedef struct YsVkRenderStageCreateInfo {
    u8 attachment_count;
    VkAttachmentDescription* attachment_descriptions;

    u8 subpass_count;
    YsSubpassConfig* subpass_configs;

    u8 subpass_dependency_count;
    VkSubpassDependency* subpass_dependencies;

    u8 framebuffer_count;
    VkFramebufferCreateInfo* framebuffer_create_info;
} YsVkRenderStageCreateInfo;

typedef struct YsVkSwapchainSupportInfo {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} YsVkSwapchainSupportInfo;


#endif //CGPPY_YVULKANSTRUCT_HPP
