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
#include "klib/kvec.h"

#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <cglm/vec4.h>
#include <cglm/vec4-ext.h>

#define VK_USE_PLATFORM_METAL_EXT
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_metal.h>
#include <vulkan/vulkan_macos.h>
#include <vulkan/vk_enum_string_helper.h>


struct YsVkContext;
struct YsVkDevice;
struct YsVkCommandUnit;
struct YsVkSwapchain;
struct YsVkImage;
struct YsVkRenderStage;
struct YsVkBuffer;
struct YsVkResources;
struct YsVkPipeline;
struct YsVkShader;
struct YsVkDescription;
struct YsVkRenderingSystem;
struct YsVkOutputSystem;
struct YsVkRasterizationSystem;
struct YsVkShadowMappingSystem;
struct YsVkPathTracingSystem;


#endif //CGPPY_YVULKANSTRUCT_HPP
