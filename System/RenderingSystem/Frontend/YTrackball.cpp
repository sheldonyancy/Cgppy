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

#include "YTrackball.hpp"
#include "YDefines.h"
#include "YGlobalInterface.hpp"
#include "YRendererBackendManager.hpp"
#include "YRendererBackend.hpp"
#include "YLogger.h"


YTrackball::YTrackball() {

}

YTrackball::~YTrackball() {

}

glm::quat YTrackball::getRotation(glm::fvec2 start_position, glm::fvec2 end_position) {
    glm::vec3 start = mapToSphere(start_position.x, start_position.y);
    glm::vec3 end = mapToSphere(end_position.x, end_position.y);
    glm::vec3 axis = glm::cross(start, end);
    float angle = glm::acos(glm::min(1.0f, glm::dot(start, end)));
    return glm::angleAxis(angle, glm::normalize(axis));
}

glm::fvec3 YTrackball::mapToSphere(int x, int y) {
    float x_ndc = (float(x) / YGlobalInterface::instance()->getMainWindowSize().x - 0.5) * 2.0;
    float y_ndc = (float(y) / YGlobalInterface::instance()->getMainWindowSize().y - 0.5) * 2.0;
    glm::fvec2 point(x_ndc, y_ndc);
    float length2 = point.x * point.x + point.y * point.y;

    if (length2 > 1.0f) {
        return glm::vec3(point, 0.0f);
    } else {
        return glm::vec3(point, sqrt(1.0f - length2));
    }
}