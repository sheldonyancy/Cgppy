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

#ifndef CGPPY_YMATH_H
#define CGPPY_YMATH_H

#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <cglm/vec4.h>
#include <cglm/vec3.h>
#include <cglm/vec4-ext.h>

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "klib/kstring.h"


#ifdef __cplusplus
extern "C" {
#endif


void yMat4ToC(const glm::fmat4x4& src, mat4 dest);
void yVec2ToC(const glm::fvec2& src, vec2 dest);
void yVec3ToC(const glm::fvec3& src, vec3 dest);
void yVec4ToC(const glm::fvec4& src, vec4 dest);

bool yKStringEqual(kstring_t a, kstring_t b);
bool yCStringEqual(const char* str0, const char* str1);

glm::fvec4 yCalculatePlaneNormal(const glm::fvec4& point1, const glm::fvec4& point2, const glm::fvec4& point3);

#ifdef __cplusplus
}
#endif


#endif //CGPPY_YMATH_H
