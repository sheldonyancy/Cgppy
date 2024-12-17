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

#ifndef CGPPY_YVULKANBUFFER_H
#define CGPPY_YVULKANBUFFER_H


#include "YVulkanTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct YsVkBuffer {
    b8 (*create)(struct YsVkContext* context,
                 u64 size,
                 VkBufferUsageFlagBits usage,
                 VkMemoryPropertyFlagBits memory_property_flags,
                 struct YsVkBuffer* out_buffer);

    void (*destroy)(struct YsVkContext* context, struct YsVkBuffer* buffer);

    void (*copyToImage)(struct YsVkContext* context,
                        struct YsVkCommandUnit* command_unit,
                        VkFence fence,
                        struct YsVkImage* image,
                        struct YsVkBuffer* buffer);

    void (*directUpdate)(struct YsVkContext* context,
                         u64 offset,
                         u32 flags,
                         const void* data,
                         struct YsVkBuffer* buffer);

    void (*indirectUpdate)(struct YsVkContext* context,
                           struct YsVkCommandUnit* command_unit,
                           VkFence fence,
                           u64 offset,
                           void* data,
                           struct YsVkBuffer* buffer);

                           void (*copyToBuffer)(struct YsVkContext* context,
                         struct YsVkCommandUnit* command_unit,
                         VkFence fence,
                         VkBuffer source,
                         u64 source_offset,
                         VkBuffer dest,
                         u64 dest_offset,
                         u64 size);                    

    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    VkMemoryPropertyFlagBits memory_property_flags;
} YsVkBuffer;

YsVkBuffer* yVkAllocateBufferObject();


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YVULKANBUFFER_H