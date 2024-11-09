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

#ifndef CGPPY_YASSETS_H
#define CGPPY_YASSETS_H


#ifdef __cplusplus
extern "C" {
#endif

#include "YDefines.h"

enum YeAssetsImage{
    Lenna,
    Baboon,
    Barbara,
    Cameraman,
    Goldhill
};

enum YeAssetsShader{
    Output_Vert,
    Output_Frag,
    Rasterization_Vert,
    Rasterization_Frag,
    Shadow_Map_Vert,
    Shadow_Map_Frag,
    Path_Tracing_Vert,
    Path_Tracing_Frag,
};

void yInitAssets();
const char* yAssetsImageFile(enum YeAssetsImage e);
const char* yAssetsGlslFile(enum YeAssetsShader s);
const char* yAssetsSpvFile(enum YeAssetsShader s);

u32* getSpvCode(enum YeAssetsShader s);
u64 getSpvCodeSize(enum YeAssetsShader s);


#ifdef __cplusplus
}
#endif


#endif //CGPPY_YASSETS_H
