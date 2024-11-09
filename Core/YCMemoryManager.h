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

#ifndef CGPPY_YCMEMORYMANAGER_H
#define CGPPY_YCMEMORYMANAGER_H


#ifdef __cplusplus
extern "C" {
#endif

#include "YDefines.h"


typedef enum YeCMemoryTag {
    MEMORY_TAG_UNKNOWN,

} YeCMemoryTag;

void yCMemorySystemInitialize();

void* yCMemoryAllocate(u64 size);

void* yCMemoryAlignedAllocate(u64 size, u16 alignment);

void yCMemoryFree(void* ptr);

void yCMemoryCopy(void* dest, const void* source, u64 size);

void yCMemoryAllocateReport(u64 size);

void yCMemoryFreeReport(u64 size);

u64 yCMemoryUsableSize(void* ptr);

void yCMemoryZero(void* ptr);


#ifdef __cplusplus
}
#endif



#endif //CGPPY_YCMEMORYMANAGER_H
