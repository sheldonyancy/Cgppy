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

#include "YRendererFrontendManager.hpp"
#include "YGlobalInterface.hpp"
#include "YCamera.hpp"
#include "YTrackball.hpp"
#include "YGlfwWindow.hpp"
#include "GLFW/glfw3.h"
#include "YLogger.h"


YRendererFrontendManager* YRendererFrontendManager::instance() {
    static YRendererFrontendManager camera_manager;
    return &camera_manager;
}

YRendererFrontendManager::YRendererFrontendManager() {

}

YRendererFrontendManager::~YRendererFrontendManager() {

}

void YRendererFrontendManager::initFrontend() {
    glfwInit();

    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    if (!primary_monitor) {
        YERROR("Failed to get primary monitor");
        glfwTerminate();
        return;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    if (mode) {
        YINFO("Current screen resolution: %ix%i", mode->width, mode->height);
        YGlobalInterface::instance()->setMainWindowSize(glm::ivec2(mode->width, mode->height * 0.8f));
    } else {
        YERROR("Failed to get video mode for primary monitor");
    }

    this->m_glfw_window = std::make_unique<YGlfwWindow>();
    this->m_camera = std::make_unique<YCamera>();
    this->m_trackball = std::make_unique<YTrackball>();
}

void YRendererFrontendManager::eventLoop() {
    this->m_glfw_window->eventLoop();
}

void YRendererFrontendManager::rotateCameraOnSphere(glm::fvec3& position,
                                                    const glm::fvec3& target,
                                                    glm::fvec3& up,
                                                    glm::fvec3& right,
                                                    const glm::fquat& rotation) {
    glm::fvec3 direction = glm::normalize(position - target);

    glm::fvec3 new_direction = rotation * direction * glm::conjugate(rotation);

    position = target + new_direction * glm::distance(position, target);

    up = glm::normalize(rotation * up * glm::conjugate(rotation));
    right = glm::normalize(rotation * right * glm::conjugate(rotation));
}

