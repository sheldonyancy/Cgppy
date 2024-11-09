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

#ifndef CGPPY_YLIGHTCOMPONENT_H
#define CGPPY_YLIGHTCOMPONENT_H


#include "YComponent.hpp"


struct YsPointLightComponent : public YsComponent {
    glm::fvec3 center;
    glm::fvec3 color;
    float intensity;
    float radius;
};

struct YsDirectionalLightComponent : public YsComponent {
    glm::fvec3 direction;
    glm::fvec3 color;
    float intensity;
};

struct YsSpotLightComponent : public YsComponent {
    glm::fvec3 center;
    glm::fvec3 direction;
    glm::fvec3 color;
    float intensity;
    float inner_cone;
    float outer_cone;
};

struct YsAreaLightComponent : public YsComponent {
    glm::fvec3 center;
    glm::fvec3 p0;
    glm::fvec3 p1;
    glm::fvec3 p2;
    glm::fvec3 p3;
    glm::fvec3 normal;
    glm::fvec3 le;
    float intensity;
    glm::fvec3 light_space_target;
    glm::fvec3 light_space_up;
    glm::fmat4x4 light_space_matrix;
};


#endif //CGPPY_YLIGHTCOMPONENT_H
