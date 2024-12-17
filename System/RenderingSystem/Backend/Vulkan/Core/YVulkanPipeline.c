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

#include "YVulkanPipeline.h"
#include "YVulkanContext.h"
#include "YCMemoryManager.h"
#include "YLogger.h"


static b8 createShaderModule(YsVkContext* context,
                                VkShaderStageFlagBits shader_stage_flag,
                                u64 code_size,
                                u32* pcode,
                                YsVkShaderStage* out_shader_stage) {
    out_shader_stage->create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    out_shader_stage->create_info.codeSize = code_size;
    out_shader_stage->create_info.pCode = pcode;
    VK_CHECK(vkCreateShaderModule(context->device->logical_device,
                                  &out_shader_stage->create_info,
                                  context->allocator,
                                  &out_shader_stage->handle));
    out_shader_stage->shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    out_shader_stage->shader_stage_create_info.stage = shader_stage_flag;
    out_shader_stage->shader_stage_create_info.module = out_shader_stage->handle;
    out_shader_stage->shader_stage_create_info.pName = "main";

    return true;
}

static b8 shaderCreate(struct YsVkContext* context,
                       const YsVkShaderConfig* config,
                       YsVkShader* out_shader) {

    out_shader->stage_count = config->shader_stage_config_count;
    out_shader->stages = yCMemoryAllocate(sizeof(YsVkShaderStage) * out_shader->stage_count);
    for (int i = 0; i < out_shader->stage_count; ++i) {
        createShaderModule(context,
                           config->shader_stage_config[i].stage_flag,
                           config->shader_stage_config[i].source_length,
                           config->shader_stage_config[i].source,
                           &out_shader->stages[i]);
    }

    return true;
}

static b8 create(YsVkContext* context,
                 YsVkPipelineConfig* config,
                 YsVkPipeline* out_pipeline) {
    //
    out_pipeline->config = config;

    // Pipeline Shader Stage
    out_pipeline->shader = (YsVkShader*)yCMemoryAllocate(sizeof(YsVkShader));
    shaderCreate(context,
                 &config->shader_config,
                 out_pipeline->shader);
    VkPipelineShaderStageCreateInfo shader_stage_create_info[VULKAN_SHADER_MAX_STAGES];
    for(int i = 0; i <  config->shader_config.shader_stage_config_count; ++i) {
        shader_stage_create_info[i] = out_pipeline->shader->stages[i].shader_stage_create_info;
    }

    // Pipeline Layout
    VkDescriptorSetLayoutCreateInfo empty_layout_info = {};
    empty_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    empty_layout_info.bindingCount = 0;
    empty_layout_info.pBindings = NULL;
    VkDescriptorSetLayout empty_layout;
    VK_CHECK(vkCreateDescriptorSetLayout(context->device->logical_device, 
                                         &empty_layout_info, 
                                         context->allocator, 
                                         &empty_layout));

    VkDescriptorSetLayout set_layouts[context->device->properties.limits.maxBoundDescriptorSets] = {};
    for(int i = 0; i < context->device->properties.limits.maxBoundDescriptorSets; ++i) {
        set_layouts[i] = empty_layout; 
    }
    for(int i = 0; i < config->descriptor_count; ++i) {
        set_layouts[config->descriptors[i].set] = config->descriptors[i].descriptor_set_layout;
    }

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipeline_layout_create_info.setLayoutCount = context->device->properties.limits.maxBoundDescriptorSets;
    pipeline_layout_create_info.pSetLayouts = set_layouts;
    pipeline_layout_create_info.pushConstantRangeCount = config->push_constant_range_count;
    pipeline_layout_create_info.pPushConstantRanges = config->push_constant_range;
    VK_CHECK(vkCreatePipelineLayout(context->device->logical_device,
                                    &pipeline_layout_create_info,
                                    context->allocator,
                                    &out_pipeline->pipeline_layout));             

    //
    switch(config->pipeline_type) {
        case VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO: {
            // Viewport state
            VkPipelineViewportStateCreateInfo viewport_state = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            viewport_state.viewportCount = 1;
            viewport_state.pViewports = &config->viewport;
            viewport_state.scissorCount = 1;
            viewport_state.pScissors = &config->scissor;

            // Rasterizer
            VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            rasterizer_create_info.depthClampEnable = VK_FALSE;
            rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
            rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer_create_info.lineWidth = 1.0f;
            rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer_create_info.depthBiasEnable = VK_FALSE;
            rasterizer_create_info.depthBiasConstantFactor = 0.0f;
            rasterizer_create_info.depthBiasClamp = 0.0f;
            rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

            // Multisampling.
            VkPipelineMultisampleStateCreateInfo multisampling_create_info = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
            multisampling_create_info.sampleShadingEnable = VK_FALSE;
            multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling_create_info.minSampleShading = 1.0f;
            multisampling_create_info.pSampleMask = 0;
            multisampling_create_info.alphaToCoverageEnable = VK_FALSE;
            multisampling_create_info.alphaToOneEnable = VK_FALSE;

            // Depth and stencil testing.
            VkPipelineDepthStencilStateCreateInfo depth_stencil = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            depth_stencil.depthTestEnable = VK_TRUE;
            depth_stencil.depthWriteEnable = VK_TRUE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depth_stencil.depthBoundsTestEnable = VK_FALSE;
            depth_stencil.stencilTestEnable = VK_FALSE;

            u32 color_attachment_count = config->render_stage->create_info->subpass_configs[0].subpass_description.colorAttachmentCount;
            VkPipelineColorBlendAttachmentState color_blend_attachment_state[color_attachment_count];
            for(int i = 0; i < color_attachment_count; ++i ) {
                color_blend_attachment_state[i].blendEnable = VK_FALSE;
                color_blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                color_blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                color_blend_attachment_state[i].colorBlendOp = VK_BLEND_OP_ADD;
                color_blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                color_blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                color_blend_attachment_state[i].alphaBlendOp = VK_BLEND_OP_ADD;
                color_blend_attachment_state[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                                 VK_COLOR_COMPONENT_G_BIT |
                                                                 VK_COLOR_COMPONENT_B_BIT |
                                                                 VK_COLOR_COMPONENT_A_BIT;
            }
            VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
            color_blend_state_create_info.logicOpEnable = VK_FALSE;
            color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY;
            color_blend_state_create_info.attachmentCount = color_attachment_count;
            color_blend_state_create_info.pAttachments = color_blend_attachment_state;

            // Dynamic state
            const u32 dynamic_state_count = 3;
            VkDynamicState dynamic_states[dynamic_state_count] = {VK_DYNAMIC_STATE_VIEWPORT,
                                                                  VK_DYNAMIC_STATE_SCISSOR,
                                                                  VK_DYNAMIC_STATE_LINE_WIDTH};

            VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            dynamic_state_create_info.dynamicStateCount = dynamic_state_count;
            dynamic_state_create_info.pDynamicStates = dynamic_states;

            // Input assembly
            VkPipelineInputAssemblyStateCreateInfo input_assembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            input_assembly.primitiveRestartEnable = VK_FALSE;

            // Pipeline create
            VkGraphicsPipelineCreateInfo pipeline_create_info = {config->pipeline_type};
            pipeline_create_info.stageCount = config->shader_config.shader_stage_config_count;
            pipeline_create_info.pStages = shader_stage_create_info;
            pipeline_create_info.pVertexInputState = config->vertex_input_info;
            pipeline_create_info.pInputAssemblyState = &input_assembly;
            pipeline_create_info.pViewportState = &viewport_state;
            pipeline_create_info.pRasterizationState = &rasterizer_create_info;
            pipeline_create_info.pMultisampleState = &multisampling_create_info;
            pipeline_create_info.pDepthStencilState = &depth_stencil;
            pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
            pipeline_create_info.pDynamicState = &dynamic_state_create_info;
            pipeline_create_info.pTessellationState = NULL;
            pipeline_create_info.layout = out_pipeline->pipeline_layout;
            pipeline_create_info.renderPass = config->render_stage->render_pass_handle;
            pipeline_create_info.subpass = 0;
            VkResult result = vkCreateGraphicsPipelines(context->device->logical_device,
                                                        VK_NULL_HANDLE,
                                                        1,
                                                        &pipeline_create_info,
                                                        context->allocator,
                                                        &out_pipeline->handle);

            if (VK_SUCCESS != result) {
                YERROR("vkCreateGraphicsPipelines failed with %s.", string_VkResult(result));
                return false;
            } else {
                YINFO("Create Graphics Pipeline Success.");
            }

            break;
        }
        case VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO: {
            VkComputePipelineCreateInfo pipeline_create_info = {config->pipeline_type};
            pipeline_create_info.stage = shader_stage_create_info[0];
            pipeline_create_info.layout = out_pipeline->pipeline_layout;
            VkResult result = vkCreateComputePipelines(context->device->logical_device,
                                                       VK_NULL_HANDLE,
                                                       1,
                                                       &pipeline_create_info,
                                                       context->allocator,
                                                       &out_pipeline->handle);
            if (VK_SUCCESS != result) {
                YERROR("vkCreateComputePipelines failed with %s.", string_VkResult(result));
                return false;
            } else {
                YINFO("Create Compute Pipeline Success.");
            }

            break;
        }
    }             

    return true;
}

static void destroy(struct YsVkContext* context, YsVkPipeline* pipeline) {
    if (pipeline) {
        // Destroy pipeline
        if (pipeline->handle) {
            vkDestroyPipeline(context->device->logical_device, pipeline->handle, context->allocator);
            pipeline->handle = 0;
        }

        // Destroy layout
        if (pipeline->pipeline_layout) {
            vkDestroyPipelineLayout(context->device->logical_device, pipeline->pipeline_layout, context->allocator);
            pipeline->pipeline_layout = 0;
        }
    }
}

YsVkPipeline* yVkAllocatePipelineObject() {
    YsVkPipeline* pipeline = yCMemoryAllocate(sizeof(YsVkPipeline));
    if(pipeline) {
        pipeline->create = create;
        pipeline->destroy = destroy;
    }
    return pipeline;
}