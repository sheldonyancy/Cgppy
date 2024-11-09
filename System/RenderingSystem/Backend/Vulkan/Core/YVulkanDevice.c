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

#include "YVulkanDevice.h"
#include "YVulkanContext.h"
#include "YLogger.h"
#include "YCMemoryManager.h"

#define MAX_VK_GRAPHICS_COMPUTE_COMMAND_UNITS_COUNT 64

static void querySwapchainSupport(VkPhysicalDevice physical_device,
                                  VkSurfaceKHR surface,
                                  YsVkSwapchainSupportInfo* out_support_info) {
    // Surface capabilities
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physical_device,
            surface,
            &out_support_info->capabilities));

    // Surface formats
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physical_device,
            surface,
            &out_support_info->format_count,
            0));

    if (out_support_info->format_count != 0) {
        if (!out_support_info->formats) {
            out_support_info->formats = yCMemoryAllocate(sizeof(VkSurfaceFormatKHR) * out_support_info->format_count);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
                physical_device,
                surface,
                &out_support_info->format_count,
                out_support_info->formats));
    }
    YINFO("Vulkan surface format count: %d", out_support_info->format_count);

    // Present modes
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device,
            surface,
            &out_support_info->present_mode_count,
            0));
    if (out_support_info->present_mode_count != 0) {
        if (!out_support_info->present_modes) {
            out_support_info->present_modes = yCMemoryAllocate(sizeof(VkPresentModeKHR) * out_support_info->present_mode_count);
        }
        VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
                physical_device,
                surface,
                &out_support_info->present_mode_count,
                out_support_info->present_modes));
    }
}

static b8 physicalDeviceMeetsRequirements(VkPhysicalDevice device,
                                          const VkPhysicalDeviceProperties* properties,
                                          const VkPhysicalDeviceFeatures* features,
                                          b8 is_apple_silicon,
                                          YsVkContext* context) {
    if (!is_apple_silicon) {
        if (properties->deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            YINFO("Device is not a discrete GPU, and one is required. Skipping.");
            return false;
        }
    }

    context->device->graphics_compute_command_units = yCMemoryAllocate(sizeof(YsVkCommandUnit) * MAX_VK_GRAPHICS_COMPUTE_COMMAND_UNITS_COUNT);

    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queue_family_count,
                                             NULL);
    if (0 == queue_family_count) {
        return false;
    }
    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queue_family_count,
                                             queue_families);
    YINFO("Vulkan queue family count: %d", queue_family_count);
    YINFO("Index | Queue Count | Graphics | Compute | Transfer | Sparse Binding | Protected | Video Decode | Video Encode | Optical_Flow |    Name");
    for(u32 i = 0; i < queue_family_count; ++i) {
        u32 support_queue_count = queue_families[i].queueCount;

        b8 support_graphics = false;
        if(queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            support_graphics = true;
            if (queue_families[i].timestampValidBits == 0){
                YWARN("The selected graphics queue family does not support timestamp queries!");
            }
        }
        b8 support_compute = false;
        if(queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            support_compute = true;
        }
        b8 support_transfer = false;
        if(queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            support_transfer = true;
        }
        b8 support_sparse_binding = false;
        if(queue_families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
            support_sparse_binding = true;
        }
        b8 support_protected = false;
        if(queue_families[i].queueFlags & VK_QUEUE_PROTECTED_BIT) {
            support_protected = true;
        }
        b8 support_video_decode = false;
        if(queue_families[i].queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
            support_video_decode = true;
        }
        b8 support_video_encode = false;
        if(queue_families[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
            support_video_encode = true;
        }
        b8 support_optical_flow = false;
        if(queue_families[i].queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV) {
            support_optical_flow = true;
        }
        YINFO("%d |       %d |       %s |       %s |       %s |       %s |       %s |       %s |       %s |       %s |     %s",
              i,
              support_queue_count,
              support_graphics ? "true" : "false",
              support_compute ? "true" : "false",
              support_transfer ? "true" : "false",
              support_sparse_binding ? "true" : "false",
              support_protected ? "true" : "false",
              support_video_decode ? "true" : "false",
              support_video_encode ? "true" : "false",
              support_optical_flow ? "true" : "false",
              properties->deviceName);
        //
        if(support_graphics && support_compute) {
            u32 u_queue_index = 0;
            for(u32 u = context->device->graphics_compute_command_units_count;
                u < context->device->graphics_compute_command_units_count + support_queue_count;
                ++u) {
                context->device->graphics_compute_command_units[u].queue_family_index = i;
                context->device->graphics_compute_command_units[u].queue_index = u_queue_index++;
            }
            context->device->graphics_compute_command_units_count += support_queue_count;
        }
    }

    querySwapchainSupport(device,
                          context->surface,
                          &context->device->swapchain_support);

    if(context->device->swapchain_support.format_count < 1 || context->device->swapchain_support.present_mode_count < 1) {
        if (context->device->swapchain_support.formats) {
            yCMemoryFree(context->device->swapchain_support.formats);
        }
        if (context->device->swapchain_support.present_modes) {
            yCMemoryFree(context->device->swapchain_support.present_modes);
        }
        YERROR("Required swapchain support not present, skipping device.");
        return false;
    }

    if (!features->samplerAnisotropy) {
        YERROR("Device does not support samplerAnisotropy, skipping.");
        return false;
    }

    return true;
}

static b8 selectPhysicalDevice(YsVkContext* context) {
    u32 physical_device_count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, NULL));
    if (physical_device_count == 0) {
        YFATAL("No devices which support Vulkan were found.");
        return false;
    }
    VkPhysicalDevice physical_devices[physical_device_count];
    VK_CHECK(vkEnumeratePhysicalDevices(context->instance, &physical_device_count, physical_devices));

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_devices[0], &properties);

    VkPhysicalDeviceLimits limits = properties.limits;
    if (limits.timestampPeriod == 0){
        YWARN("The selected device does not support timestamp queries!");
    }

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physical_devices[0], &features);

    VkPhysicalDeviceMemoryProperties memory;
    vkGetPhysicalDeviceMemoryProperties(physical_devices[0], &memory);

#if __APPLE__
    b8 is_apple_silicon = true;
#else
    b8 is_apple_silicon = false;
#endif
    b8 result = physicalDeviceMeetsRequirements(physical_devices[0],
                                                &properties,
                                                &features,
                                                is_apple_silicon,
                                                context);
    if (result) {
        YINFO("Selected device: '%s'.", properties.deviceName);
        switch (properties.deviceType) {
            default:
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                YINFO("GPU type is Unknown.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                YINFO("GPU type is Integrated.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                YINFO("GPU type is Descrete.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                YINFO("GPU type is Virtual.");
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                YINFO("GPU type is CPU.");
                break;
        }

        YINFO("GPU Driver version: %d.%d.%d",
              VK_VERSION_MAJOR(properties.driverVersion),
              VK_VERSION_MINOR(properties.driverVersion),
              VK_VERSION_PATCH(properties.driverVersion));

        YINFO("Vulkan API version: %d.%d.%d",
              VK_VERSION_MAJOR(properties.apiVersion),
              VK_VERSION_MINOR(properties.apiVersion),
              VK_VERSION_PATCH(properties.apiVersion));

        for (u32 j = 0; j < memory.memoryHeapCount; ++j) {
            f32 memory_size_gib = (((f32)memory.memoryHeaps[j].size) / 1024.0f / 1024.0f / 1024.0f);
            if (memory.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                YINFO("Local GPU memory: %.2f GiB", memory_size_gib);
            } else {
                YINFO("Shared System memory: %.2f GiB", memory_size_gib);
            }
        }
    }

    context->device->physical_device = physical_devices[0];
    context->device->properties = properties;
    context->device->features = features;
    context->device->memory = memory;

    YINFO("Physical device selected.");
    return true;
}

static void commandBufferAllocate(YsVkContext* context,
                                  YsVkCommandUnit* command_unit,
                                  b8 is_primary,
                                  VkCommandBuffer* out_command_buffer) {
    VkCommandBufferAllocateInfo allocate_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocate_info.commandPool = command_unit->command_pools[0];
    allocate_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocate_info.commandBufferCount = 1;
    allocate_info.pNext = 0;
    VK_CHECK(vkAllocateCommandBuffers(context->device->logical_device,
                                      &allocate_info,
                                      out_command_buffer));
}

static b8 create(YsVkContext* context) {
    if (!selectPhysicalDevice(context)) {
        return false;
    }

    YINFO("Creating logical device...");

    VkDeviceQueueCreateInfo queue_create_infos[context->device->graphics_compute_command_units_count];
    for(int i = 0; i < context->device->graphics_compute_command_units_count; ++i) {
        YsVkCommandUnit* i_command_unit = &context->device->graphics_compute_command_units[i];
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = i_command_unit->queue_family_index;
        queue_create_infos[i].queueCount = 1;
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = 0;
        f32 queue_priority = 1.0f;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    b8 portability_required = false;
    u32 available_extension_count = 0;
    VK_CHECK(vkEnumerateDeviceExtensionProperties(context->device->physical_device, 0, &available_extension_count, NULL));
    if(0 == available_extension_count) {
        return false;
    }
    YINFO("Available Extension Count: %d", available_extension_count);
    VkExtensionProperties available_extensions[available_extension_count];
    VK_CHECK(vkEnumerateDeviceExtensionProperties(context->device->physical_device, 0, &available_extension_count, available_extensions));
    for (u32 i = 0; i < available_extension_count; ++i) {
        YINFO("Available Device Extension: %s", available_extensions[i].extensionName);
        if (0==strcmp(available_extensions[i].extensionName, "VK_KHR_portability_subset")) {
            YINFO("Adding Device Extension 'VK_KHR_portability_subset'.");
            portability_required = true;
            break;
        }
    }
    u32 extension_count = portability_required ? 3 : 1;
    const char** extension_names = portability_required
                                   ? (const char* [3]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset", "VK_KHR_shader_non_semantic_info" }
                                   : (const char* [1]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    device_create_info.queueCreateInfoCount = context->device->graphics_compute_command_units_count;
    device_create_info.pQueueCreateInfos = queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = extension_count;
    device_create_info.ppEnabledExtensionNames = extension_names;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = 0;
    VK_CHECK(vkCreateDevice(
            context->device->physical_device,
            &device_create_info,
            context->allocator,
            &context->device->logical_device));

    YINFO("Logical device created.");

    //
    for(int i = 0; i < context->device->graphics_compute_command_units_count; ++i) {
        YsVkCommandUnit* i_command_unit = &context->device->graphics_compute_command_units[i];
        vkGetDeviceQueue(context->device->logical_device,
                         i_command_unit->queue_family_index,
                         i_command_unit->queue_index,
                         &i_command_unit->queue);

        i_command_unit->command_pool_count = 1;
        i_command_unit->command_pools = yCMemoryAllocate(sizeof(VkCommandPool) * i_command_unit->command_pool_count);
        VkCommandPoolCreateInfo pool_create_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        pool_create_info.queueFamilyIndex = i_command_unit->queue_family_index;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK(vkCreateCommandPool(context->device->logical_device,
                                     &pool_create_info,
                                     context->allocator,
                                     &i_command_unit->command_pools[0]));

        i_command_unit->command_buffer_count = 2;
        i_command_unit->command_buffers = yCMemoryAllocate(sizeof(VkCommandBuffer) * i_command_unit->command_buffer_count);
        for(int c = 0; c < i_command_unit->command_buffer_count; ++c) {
            commandBufferAllocate(context,
                                  i_command_unit,
                                  true,
                                  &i_command_unit->command_buffers[c]);
        }

        VkQueryPoolCreateInfo query_pool_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
        query_pool_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
        query_pool_info.queryCount = 2;
        VK_CHECK(vkCreateQueryPool(context->device->logical_device,
                                   &query_pool_info,
                                   NULL,
                                   &i_command_unit->query_pool_timestamps));
        vkResetQueryPool(context->device->logical_device,
                         i_command_unit->query_pool_timestamps,
                         0,
                         2);
    }

    YDEBUG("Vulkan command buffers created.");

    return true;
}

static void destroy(YsVkContext* context) {
    for(int i = 0; i < context->device->graphics_compute_command_units_count; ++i) {
        YsVkCommandUnit* i_command_unit = &context->device->graphics_compute_command_units[i];
        YINFO("Destroying command pools...");
        vkDestroyCommandPool(context->device->logical_device,
                             i_command_unit->command_pools[0],
                             context->allocator);
    }

    context->device->graphics_compute_command_units_count = 0;
    yCMemoryFree(context->device->graphics_compute_command_units);

    YINFO("Destroying logical device...");
    if (context->device->logical_device) {
        vkDestroyDevice(context->device->logical_device, context->allocator);
        context->device->logical_device = 0;
    }

    YINFO("Releasing physical device resources...");
    context->device->physical_device = 0;

    if (context->device->swapchain_support.formats) {
        yCMemoryFree(context->device->swapchain_support.formats);
        context->device->swapchain_support.formats = 0;
        context->device->swapchain_support.format_count = 0;
    }

    if (context->device->swapchain_support.present_modes) {
        yCMemoryFree(context->device->swapchain_support.present_modes);
        context->device->swapchain_support.present_modes = 0;
        context->device->swapchain_support.present_mode_count = 0;
    }

    yCMemoryZero(&context->device->swapchain_support.capabilities);
}

static b8 detectDepthFormat(YsVkDevice* device) {
    const u64 candidate_count = 2;
    VkFormat candidates[2] = {VK_FORMAT_D32_SFLOAT_S8_UINT,
                              VK_FORMAT_D24_UNORM_S8_UINT};

    u32 flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    for (u64 i = 0; i < candidate_count; ++i) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(device->physical_device, candidates[i], &properties);

        if ((properties.linearTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        } else if ((properties.optimalTilingFeatures & flags) == flags) {
            device->depth_format = candidates[i];
            return true;
        }
    }

    return false;
}

static YsVkCommandUnit* commandUnitsFront(YsVkDevice* device) {
    return &device->graphics_compute_command_units[0];
}

static YsVkCommandUnit* commandUnitsBack(YsVkDevice* device) {
    return &device->graphics_compute_command_units[device->graphics_compute_command_units_count - 1];
}

static YsVkCommandUnit* commandUnitsAt(YsVkDevice* device, u32 index) {
    return &device->graphics_compute_command_units[index];
}

static void commandBufferFree(YsVkContext* context,
                              YsVkCommandUnit* command_unit,
                              VkCommandBuffer* command_buffer) {
    vkFreeCommandBuffers(context->device->logical_device,
                         command_unit->command_pools[0],
                         1,
                         command_buffer);
}

static void commandBufferBegin(VkCommandBuffer command_buffer, VkCommandBufferUsageFlags flags) {
    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = flags;
    VK_CHECK(vkBeginCommandBuffer(command_buffer, &begin_info));
}

static void commandBufferEnd(VkCommandBuffer command_buffer) {
    VK_CHECK(vkEndCommandBuffer(command_buffer));
}

static void commandBufferReset(VkCommandBuffer command_buffer) {
    VkCommandBufferResetFlags flag;
    VK_CHECK(vkResetCommandBuffer(command_buffer, flag));
}

static void commandBufferAllocateAndBeginSingleUse(YsVkContext* context,
                                                   YsVkCommandUnit* command_unit,
                                                   VkCommandBuffer* tmp_command_buffer) {
    commandBufferAllocate(context,
                          command_unit,
                          true,
                          tmp_command_buffer);
    commandBufferBegin(*tmp_command_buffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
}

static void commandBufferEndSingleUse(YsVkContext* context,
                                      YsVkCommandUnit* command_unit,
                                      VkCommandBuffer* tmp_command_buffer) {
    // End the command buffer.
    commandBufferEnd(*tmp_command_buffer);

    // Submit the queue
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = tmp_command_buffer;
    VK_CHECK(vkQueueSubmit(command_unit->queue,
                           1,
                           &submit_info,
                           VK_NULL_HANDLE));

    // Wait for it to finish
    VK_CHECK(vkQueueWaitIdle(command_unit->queue));

    // Free the command buffer.
    commandBufferFree(context,
                      command_unit,
                      tmp_command_buffer);
}

YsVkDevice* yVkAllocateDeviceObject() {
    YsVkDevice* device = yCMemoryAllocate(sizeof(YsVkDevice));
    if(device) {
        device->create = create;
        device->destroy = destroy;
        device->detectDepthFormat = detectDepthFormat;
        device->commandUnitsFront = commandUnitsFront;
        device->commandUnitsBack = commandUnitsBack;
        device->commandUnitsAt = commandUnitsAt;
        device->commandBufferFree = commandBufferFree;
        device->commandBufferBegin = commandBufferBegin;
        device->commandBufferEnd = commandBufferEnd;
        device->commandBufferReset = commandBufferReset;
        device->commandBufferAllocateAndBeginSingleUse = commandBufferAllocateAndBeginSingleUse;
        device->commandBufferEndSingleUse = commandBufferEndSingleUse;
    }

    return device;
}

