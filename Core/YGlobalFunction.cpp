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

#include "YGlobalFunction.h"
#include "YGLSLStructs.hpp"

#include <random>
#include <algorithm>
#include <cstdint>
#include <limits>


void yGenerateUintRand(u32 count, u32* data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    u32 max = std::numeric_limits<uint32_t>::max();
    std::uniform_int_distribution<uint32_t> dis(0, max);

    for(u32 i = 0; i < count; ++i) {
        data[i] = dis(gen);
    }
}

u32 ySsboSize() {
    u32 size = sizeof(GLSL_SSBO);
    return size;
}

u32 yUboSize() {
    u32 size = sizeof(GLSL_UBO);
    return size;
}

u32 yPushConstantSize() {
    u32 size = sizeof(GLSL_PushConstantObject);
    return size;
}

f32 yRoundToOneDecimal(f32 value){
    return floor(value * 10 + 0.5) / 10;
}