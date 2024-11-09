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

#include "YCMemoryManager.h"
#include "YLogger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <jemalloc/jemalloc.h>


static u32 g_arena_index;
u64 pool_size = GIBIBYTES(1);

void yCMemorySystemInitialize() {
    size_t sz = sizeof(g_arena_index);

    if (mallctl("arenas.create", &g_arena_index, &sz, NULL, 0) != 0) {
        YERROR("Error in creating arena");
    }
}

void* yCMemoryAllocate(u64 size) {
    void* ptr = mallocx(size, MALLOCX_ARENA(g_arena_index));
    if (NULL == ptr) {
        YERROR("mallocx error");
        return NULL;
    }

    yCMemoryZero(ptr);

    return ptr;
}

void* yCMemoryAlignedAllocate(u64 size, u16 alignment) {
    int flags = MALLOCX_ALIGN(alignment) | MALLOCX_ARENA(g_arena_index);
    void* ptr = mallocx(size, flags);
    if (NULL == ptr) {
        YERROR("mallocx error");
    }

    yCMemoryZero(ptr);

    return ptr;
}

void yCMemoryFree(void* ptr) {
    u64 usable_size = malloc_usable_size(ptr);
    dallocx(ptr, 0);
}

void yCMemoryCopy(void* dest, const void* source, u64 size) {
    memcpy(dest, source, size);
}

void yCMemoryAllocateReport(u64 size) {

}

void yCMemoryFreeReport(u64 size) {

}

u64 yCMemoryUsableSize(void* ptr) {
    u64 usable_size = malloc_usable_size(ptr);
    return usable_size;
}

void yCMemoryZero(void* ptr) {
    u64 usable_size = malloc_usable_size(ptr);
    memset(ptr, 0, usable_size);
}