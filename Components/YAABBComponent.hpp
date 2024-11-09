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

#ifndef CGPPY_YAABBCOMPONENT_HPP
#define CGPPY_YAABBCOMPONENT_HPP


#include "YDefines.h"
#include "YComponent.hpp"

#include <vector>



struct YsAABBComponent : public YsComponent {
    enum class YeAxis : unsigned char {
        X_AXIS,
        Y_AXIS,
        Z_AXIS
    };

    glm::fvec3 min = glm::fvec3(9999.0f);
    glm::fvec3 max = glm::fvec3(-9999.0f);

    glm::fvec3 center() const {
        return (min + max) * 0.5f;
    }

    bool intersects(const YsAABBComponent& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    void expand(const YsAABBComponent& other) {
        for (int i = 0; i < 3; ++i) {
            min[i] = std::min(min[i], other.min[i]);
            max[i] = std::max(max[i], other.max[i]);
        }
    }

    void reset() {
        min = glm::fvec3(9999.0f);
        max = glm::fvec3(-9999.0f);
    }

    f32 boundingSphereRadius() {
        glm::fvec3 length = max - min;

        f32 diagonal = sqrtf(pow(length.x, 2) + pow(length.y, 2) + pow(length.z, 2));
        f32 radius = 0.5f * diagonal;
        return radius;
    }

    YeAxis longestAxis() {
        glm::fvec3 length = max - min;
        if ((length.x >= length.y) && (length.x >= length.z)) {
            return YeAxis::X_AXIS;
        }
        if((length.y >= length.x) && (length.y >= length.z)) {
            return YeAxis::Y_AXIS;
        }
        if((length.z >= length.x) && (length.z >= length.y)) {
            return YeAxis::Z_AXIS;
        }

        return YeAxis::X_AXIS;
    }
};


#endif //CGPPY_YAABBCOMPONENT_HPP
