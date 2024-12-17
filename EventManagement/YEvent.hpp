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

#ifndef CGPPY_YEVENT_HPP
#define CGPPY_YEVENT_HPP


#include <variant>

#include <glm/fwd.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "YDefines.h"
#include "YRendererBackendManager.hpp"


enum class YeKeyEventType : unsigned short {
    KEY_DOWN,
    KEY_UP
};

enum class YeMouseEventType : unsigned char {
    BUTTON_DOWN,
    BUTTON_UP,
    MOVE,
    SCROLL,
    DRAGGED
};

enum class YeMouseButton : unsigned short {
    NONE,
    LEFT,
    RIGHT,
    MIDDLE,
    WHEEL_UP,
    WHEEL_DOWN,
    WHEEL_LEFT,
    WHEEL_RIGHT
};

enum class YeMouseButtonMask : unsigned short{
    NONE = 0,
    LEFT = (1 << (int(YeMouseButton::LEFT) - 1)),
    RIGHT = (1 << (int(YeMouseButton::RIGHT) - 1)),
    MIDDLE = (1 << (int(YeMouseButton::MIDDLE) - 1)),
};

struct YsKeyEvent {
    YeKeyEventType type;
    unsigned short key_code;
};

struct YsMouseEvent {
    YeMouseEventType type;
    glm::ivec2 position;
    float magnification;
};

struct YsUpdateSceneEvent {
    int unknow;
};

struct YsChangingRenderingModelEvent {
    YeRenderingModelType type;
};

struct YsChangingPathTracingSppEvent {
    u32 spp;
};

struct YsChangingPathTracingMaxDepthEvent { 
    u32 max_depth;
};

struct YsChangingPathTracingEnableBvhAccelerationEvent {
    u8 enable_bvh_acceleration;
};

struct YsChangingPathTracingEnableDenoiserEvent {
    u8 enable_denoiser;
};

using YsEvent = std::variant<YsChangingRenderingModelEvent,
                             YsChangingPathTracingSppEvent,
                             YsChangingPathTracingMaxDepthEvent,
                             YsChangingPathTracingEnableBvhAccelerationEvent,
                             YsChangingPathTracingEnableDenoiserEvent,
                             YsUpdateSceneEvent, 
                             YsKeyEvent, 
                             YsMouseEvent>;


#endif //CGPPY_YEVENT_HPP
