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

#include "YVulkanSwapchain.h"
#include "Backend/Vulkan/Core/YVulkanContext.h"
#include "Backend/Vulkan/Core/YVulkanDevice.h"
#include "YLogger.h"
#include "YDefines.h"
#include "YCMemoryManager.h"


static void destroy(YsVkContext* context, YsVkSwapchain* swapchain) {
    vkDeviceWaitIdle(context->device->logical_device);

    for (u32 i = 0; i < swapchain->image_count; ++i) {
        vkDestroyImageView(context->device->logical_device, swapchain->present_src_images[i].individual_views[0], context->allocator);
    }

    vkDestroySwapchainKHR(context->device->logical_device, swapchain->handle, context->allocator);
}

static void create(YsVkContext* context,
                   u32 width,
                   u32 height,
                   VkPresentModeKHR present_mode,
                   YsVkSwapchain* swapchain) {
    destroy(context, swapchain);

    VkExtent2D swapchain_extent = {width, height};

    b8 found = false;
    for (u32 i = 0; i < context->device->swapchain_support.format_count; ++i) {
        VkSurfaceFormatKHR format = context->device->swapchain_support.formats[i];

        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchain->image_format = format;
            found = true;
            break;
        }
    }

    if (!found) {
        swapchain->image_format = context->device->swapchain_support.formats[0];
    }

    if (context->device->swapchain_support.capabilities.currentExtent.width != UINT32_MAX) {
        swapchain_extent = context->device->swapchain_support.capabilities.currentExtent;
    }

    // Clamp to the value allowed by the GPU.
    VkExtent2D min = context->device->swapchain_support.capabilities.minImageExtent;
    VkExtent2D max = context->device->swapchain_support.capabilities.maxImageExtent;
    swapchain_extent.width = YCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = YCLAMP(swapchain_extent.height, min.height, max.height);

    u32 image_count = context->device->swapchain_support.capabilities.minImageCount + 1;
    if (context->device->swapchain_support.capabilities.maxImageCount > 0 && image_count > context->device->swapchain_support.capabilities.maxImageCount) {
        image_count = context->device->swapchain_support.capabilities.maxImageCount;
    }

    swapchain->max_frames_in_flight = image_count;

    VkSwapchainCreateInfoKHR swapchain_create_info = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    swapchain_create_info.surface = context->surface;
    swapchain_create_info.minImageCount = image_count;
    swapchain_create_info.imageFormat = swapchain->image_format.format;
    swapchain_create_info.imageColorSpace = swapchain->image_format.colorSpace;
    swapchain_create_info.imageExtent = swapchain_extent;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queue_family_indices[4] = {0, 1, 2, 3};
    swapchain_create_info.queueFamilyIndexCount = 4;
    swapchain_create_info.pQueueFamilyIndices = queue_family_indices;
    swapchain_create_info.preTransform = context->device->swapchain_support.capabilities.currentTransform;
    swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_create_info.presentMode = present_mode;
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.oldSwapchain = 0;

    VK_CHECK(vkCreateSwapchainKHR(context->device->logical_device, &swapchain_create_info, context->allocator, &swapchain->handle));

    if (!context->device->detectDepthFormat(context->device)) {
        context->device->depth_format = VK_FORMAT_UNDEFINED;
        YFATAL("Failed to find a supported format!");
    }

    swapchain->image_count = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->device->logical_device, swapchain->handle, &swapchain->image_count, 0));
    VkImage swapchain_images[swapchain->image_count];
    VK_CHECK(vkGetSwapchainImagesKHR(context->device->logical_device, swapchain->handle, &swapchain->image_count, swapchain_images));

    if (NULL == swapchain->present_src_images) {
        swapchain->present_src_images = yCMemoryAllocate(sizeof(YsVkImage) * swapchain->image_count);
    }

    for (u32 i = 0; i < swapchain->image_count; ++i) {
        //
        swapchain->present_src_images[i].handle = swapchain_images[i];
        swapchain->present_src_images[i].create_info = yCMemoryAllocate(sizeof(VkImageCreateInfo));
        swapchain->present_src_images[i].create_info->format = swapchain->image_format.format;
        swapchain->present_src_images[i].create_info->arrayLayers = 1;
        swapchain->present_src_images[i].create_info->extent.width = swapchain_extent.width;
        swapchain->present_src_images[i].create_info->extent.height = swapchain_extent.height;

        VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        view_info.image = swapchain->present_src_images[i].handle;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = swapchain->image_format.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        swapchain->present_src_images[i].individual_views = yCMemoryAllocate(sizeof(VkImageView));
        VK_CHECK(vkCreateImageView(context->device->logical_device,
                                   &view_info,
                                   context->allocator,
                                   swapchain->present_src_images[i].individual_views));
    }

    YINFO("Swapchain created successfully.");
}


YsVkSwapchain* yVkAllocateSwapchainObject() {
    YsVkSwapchain* swapchain = yCMemoryAllocate(sizeof(YsVkSwapchain));
    if(swapchain) {
        swapchain->create = create;
        swapchain->destroy = destroy;
    }
    return swapchain;
}