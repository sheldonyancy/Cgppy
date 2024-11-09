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

#include "YMath.h"


void yMat4ToC(const glm::fmat4x4& src, mat4 dest) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            dest[i][j] = src[i][j];
        }
    }
}

void yVec2ToC(const glm::fvec2& src, vec2 dest) {
    for (int i = 0; i < 2; ++i) {
        dest[i] = src[i];
    }
}

void yVec3ToC(const glm::fvec3& src, vec3 dest) {
    for (int i = 0; i < 3; ++i) {
        dest[i] = src[i];
    }
}

void yVec4ToC(const glm::fvec4& src, vec4 dest) {
    for (int i = 0; i < 4; ++i) {
        dest[i] = src[i];
    }
}

bool yKStringEqual(kstring_t a, kstring_t b) {
    if (a.l != b.l) {
        return 0;
    }
    return strncmp(a.s, b.s, a.l) == 0;
}

bool yCStringEqual(const char* str0, const char* str1) {
    return strcmp(str0, str1) == 0;
}

glm::fvec4 yCalculatePlaneNormal(const glm::vec4& point1, const glm::vec4& point2, const glm::vec4& point3) {
    glm::vec3 vectorAB = point2 - point1;
    glm::vec3 vectorAC = point3 - point1;

    return glm::fvec4(glm::normalize(glm::cross(vectorAB, vectorAC)), 1.0f);
}