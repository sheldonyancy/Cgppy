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

#include "YVulkanContext.h"
#include "YLogger.h"
#include "YCMemoryManager.h"
#include "GLFW/glfw3.h"


static void* yVkAllocAllocationCallback(void* user_data,
                                 size_t size,
                                 size_t alignment,
                                 VkSystemAllocationScope allocation_scope) {
    if (size == 0) {
        return NULL;
    }

    void* result = yCMemoryAlignedAllocate(size, alignment);
#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("yVkAllocAllocationCallback block %p. Size=%llu, Alignment=%llu", result, size, alignment);
#endif
    return result;
}

static void yVkAllocFreeCallback(void* user_data, void* memory) {
    if (!memory) {
#ifdef YVULKAN_ALLOCATOR_TRACE
        YTRACE("Block is null, nothing to free: %p", memory);
#endif
        return;
    }

#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("Attempting to free block %p...", memory);
#endif
#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("Block %p found with size: %llu. Freeing aligned block...", memory, yCMemoryUsableSize(memory));
#endif
    yCMemoryFree(memory);
}

static void* yVkAllocReallocationCallback(void* user_data,
                                   void* original,
                                   size_t size,
                                   size_t alignment,
                                   VkSystemAllocationScope allocation_scope) {
    if (!original) {
        return yVkAllocAllocationCallback(user_data, size, alignment,allocation_scope);
    }

    if (size == 0) {
        yVkAllocFreeCallback(user_data, original);
        return NULL;
    }

#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("Attempting to realloc block %p...", original);
#endif
    void* result = yVkAllocAllocationCallback(user_data, size, alignment,allocation_scope);
    if (result) {
#ifdef YVULKAN_ALLOCATOR_TRACE
        YTRACE("Block %p reallocated to %p, copying data...", original, result);
#endif
        u64 original_size = yCMemoryUsableSize(original);
        u64 copy_size = size < original_size ? size : original_size;
        yCMemoryCopy(result, original, copy_size);
#ifdef YVULKAN_ALLOCATOR_TRACE
        YTRACE("Freeing original aligned block %p...", original);
#endif
        yCMemoryFree(original);
    } else {
#ifdef YVULKAN_ALLOCATOR_TRACE
        YERROR("Failed to realloc %p.", original);
#endif
    }

    return result;
}

static void yVkAllocInternalAllocCallback(void* user_data,
                                   size_t size,
                                   VkInternalAllocationType allocationType,
                                   VkSystemAllocationScope allocationScope) {
#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("External allocation of size: %llu", size);
#endif
    yCMemoryAllocateReport(size);
}

static void yVkAllocInternalFreeCallback(void* user_data,
                                  size_t size,
                                  VkInternalAllocationType allocationType,
                                  VkSystemAllocationScope allocationScope) {
#ifdef YVULKAN_ALLOCATOR_TRACE
    YTRACE("External free of size: %llu", size);
#endif
    yCMemoryFreeReport(size);
}

static bool yVkAllocatorCreate(struct YsVkContext *yvk_context) {
    yvk_context->allocator = (VkAllocationCallbacks*) yCMemoryAllocate(sizeof(VkAllocationCallbacks));

    if (yvk_context->allocator) {
        yvk_context->allocator->pUserData = yvk_context;
        yvk_context->allocator->pfnAllocation = yVkAllocAllocationCallback;
        yvk_context->allocator->pfnReallocation = yVkAllocReallocationCallback;
        yvk_context->allocator->pfnFree = yVkAllocFreeCallback;
        yvk_context->allocator->pfnInternalAllocation = yVkAllocInternalAllocCallback;
        yvk_context->allocator->pfnInternalFree = yVkAllocInternalFreeCallback;
    }else{
        YFATAL("Failed to create custom Vulkan allocator. Continuing using the driver's default allocator.");
        yCMemoryFree(yvk_context->allocator);
        yvk_context->allocator = NULL;
        return false;
    }
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL yVkDebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data) {

    switch (message_severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            YERROR(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            YWARN(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            YINFO(callback_data->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            YTRACE(callback_data->pMessage);
            break;
        default:
    }
}

static i32 yVkFindMemoryIndex(u32 type_filter, u32 property_flags, YsVkContext* context) {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(context->device->physical_device, &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (type_filter & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
            return i;
        }
    }

    YWARN("Unable to find suitable memory type!");
    return -1;
}

static b8 yVkInstanceCreate(const i8* const* enable_extensions,
                            i32 enable_extensions_count,
                            const i8* const* enable_layers,
                            i32 enable_layers_count,
                            YsVkContext* context) {
    context->application_name = "Cgppy";
    context->engine_name = "Cgppy";
    vkEnumerateInstanceVersion(&context->api_version);
    YINFO("System can support vulkan Variant: %i, Major: %i, Minor: %i, Patch: %i",
          VK_API_VERSION_VARIANT(context->api_version),
          VK_API_VERSION_MAJOR(context->api_version),
          VK_API_VERSION_MINOR(context->api_version),
          VK_API_VERSION_PATCH(context->api_version));
    context->api_version &= ~(0xFFFU);

    VkApplicationInfo vk_app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    vk_app_info.apiVersion = context->api_version;
    vk_app_info.pApplicationName = context->application_name;
    vk_app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vk_app_info.pEngineName = context->engine_name;
    vk_app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    VkValidationFeaturesEXT validation_features = {};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    VkValidationFeatureEnableEXT enabled_validation_features[1] = {VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
    validation_features.pEnabledValidationFeatures = enabled_validation_features;

    VkInstanceCreateInfo vk_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    vk_create_info.pApplicationInfo = &vk_app_info;
#ifdef __APPLE__
    vk_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
    vk_create_info.enabledLayerCount = enable_layers_count;
    vk_create_info.ppEnabledLayerNames = enable_layers;
    vk_create_info.enabledExtensionCount = enable_extensions_count;
    vk_create_info.ppEnabledExtensionNames = enable_extensions;

    validation_features.pNext = vk_create_info.pNext;
    vk_create_info.pNext = &validation_features;

    VkResult result = vkCreateInstance(&vk_create_info, context->allocator, &context->instance);
    if (VK_SUCCESS != result) {
        YERROR(string_VkResult(result));
        return false;
    }
    return true;
}

static b8 yVkDebuggerCreate(YsVkContext* context) {
#ifndef NDEBUG
    YDEBUG("Creating Vulkan debugger...");
    u32 log_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    debug_create_info.messageSeverity = log_severity;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | 
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debug_create_info.pfnUserCallback = yVkDebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func =
            (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(context->instance, "vkCreateDebugUtilsMessengerEXT");
    YASSERT_MSG(func, "Failed to create debug messenger!");
    VK_CHECK(func(context->instance, &debug_create_info, context->allocator, &context->debug_messenger));
    YDEBUG("Vulkan debugger created.");
#endif
    return true;
}

static b8 initialize(u32 swapchain_width,
                     u32 swapchain_height,
                     struct GLFWwindow* glfw_window,
                     const i8* const* enable_extensions,
                     i32 enable_extensions_count,
                     const i8* const* enable_layers,
                     i32 enable_layers_count,
                     YsVkContext* context) {
    context->find_memory_index = yVkFindMemoryIndex;
    context->allocator = NULL;

    // Allocator
    if (!yVkAllocatorCreate(context)) {
        YERROR("YVulkanContext failed to create Allocator!");
        return false;
    }

    // Instance
    if (!yVkInstanceCreate(enable_extensions,
                           enable_extensions_count,
                           enable_layers,
                           enable_layers_count,
                           context)) {
        YERROR("YVulkanContext failed to create Instance!");
        return false;
    }

    // Debugger
    if (!yVkDebuggerCreate(context)) {
        YERROR("YVulkanContext failed to create Debugger!");
        return false;
    }

    // Surface
    if (glfwCreateWindowSurface(context->instance, 
                                glfw_window, 
                                NULL, 
                                &context->surface) != VK_SUCCESS) {
        YFATAL("failed to create window surface!");
    }

    // Device
    context->device = yVkAllocateDeviceObject();
    if (!context->device->create(context)) {
        YERROR("YVulkanContext failed to create Device!");
        return false;
    }

    // Swapchain
    context->swapchain = yVkAllocateSwapchainObject();
    context->swapchain->create(context,
                               swapchain_width,
                               swapchain_height,
                               VK_PRESENT_MODE_FIFO_KHR, //VK_PRESENT_MODE_IMMEDIATE_KHR,
                               context->swapchain);
    YINFO("Vulkan Context Initialized Successfully.");

    return true;
}

YsVkContext* yVkContextCreate() {
    YsVkContext* context = yCMemoryAllocate(sizeof(YsVkContext));
    if(context) {
        context->initialize = initialize;
    }

    return  context;
}
















