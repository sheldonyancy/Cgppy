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

#include "YOpenGLBackend.hpp"
#include "YRendererFrontendManager.hpp"
#include "YRendererBackendManager.hpp"
#include "GLFW/glfw3.h"
#include "YGlobalInterface.hpp"
#include "YCamera.hpp"
#include "YTrackball.hpp"
#include "YVulkanBackend.hpp"
#include "YProfiler.hpp"

#include "YSceneManager.hpp"
#include "YEvent.hpp"
#include "YEventHandlerManager.hpp"
#include "YDefines.h"
#include "YLogger.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>


static bool mouse_left_button_is_press = false;

void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {

    }
}

void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (GLFW_MOUSE_BUTTON_LEFT == button) {
        if(GLFW_PRESS == action) {
            mouse_left_button_is_press = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            
            YsMouseEvent mouse_event;
            mouse_event.type = YeMouseEventType::BUTTON_DOWN;
            mouse_event.position.x = xpos;
            mouse_event.position.y = height - ypos;
            YEventHandlerManager::instance()->pushEvent(mouse_event);
        }
        if(GLFW_RELEASE == action) {
            mouse_left_button_is_press = false;
        }
    }
}

void glfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if(mouse_left_button_is_press) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        YsMouseEvent mouse_event;
        mouse_event.type = YeMouseEventType::DRAGGED;
        mouse_event.position.x = xpos;
        mouse_event.position.y = height - ypos;

        YEventHandlerManager::instance()->pushEvent(mouse_event);
    }
}

void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    YsMouseEvent mouse_event;
    mouse_event.type = YeMouseEventType::SCROLL;
    mouse_event.magnification = yoffset;

    YEventHandlerManager::instance()->pushEvent(mouse_event);
}

void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height) {

}

void glfwErrorCallback(int error_code, const char* description) {
    YERROR("GLFW Error: %i: %s", error_code, description);
}

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
        this->m_main_window_size = glm::ivec2(mode->width, mode->height);
    } else {
        YERROR("Failed to get video mode for primary monitor");
    }

    this->initVulkanEnv();

    this->m_camera = std::make_unique<YCamera>();

    this->m_trackball = std::make_unique<YTrackball>();
}

void YRendererFrontendManager::initVulkanEnv() {
    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    this->m_glfw_window = glfwCreateWindow(this->m_main_window_size.x, 
                                           this->m_main_window_size.y, 
                                           "Cgppy", 
                                           nullptr, 
                                           nullptr);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwSetWindowSizeLimits(this->m_glfw_window, 
                            this->m_main_window_size.x, 
                            this->m_main_window_size.y, 
                            this->m_main_window_size.x, 
                            this->m_main_window_size.y);
    glfwSetKeyCallback(this->m_glfw_window, glfwKeyCallback);
    glfwSetMouseButtonCallback(this->m_glfw_window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(this->m_glfw_window, glfwCursorPositionCallback);
    glfwSetScrollCallback(this->m_glfw_window, glfwScrollCallback);
    glfwSetFramebufferSizeCallback(this->m_glfw_window, glfwFramebufferSizeCallback);

    glfwGetFramebufferSize(this->m_glfw_window, 
                           &this->m_main_window_framebuffer_size.x, 
                           &this->m_main_window_framebuffer_size.y);                      
}

void YRendererFrontendManager::eventLoop() {
    while (!glfwWindowShouldClose(this->m_glfw_window)) {
        auto start_rendering = std::chrono::high_resolution_clock::now();
        {
            glfwPollEvents();

            YEventHandlerManager::instance()->pollEvents();

            YRendererBackendManager::instance()->backend()->draw();
        }
        auto end_rendering = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> duration = end_rendering - start_rendering;
        double rendering_time = duration.count() / 1000.0;
        YProfiler::instance()->accumulateRenderingFrameTime(rendering_time);
    }
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

void YRendererFrontendManager::initOpenGLEnv() {
    GLenum init_glew = glewInit();

    if (GLEW_OK != init_glew) {
        YERROR("YOpenGLWindow::initializeGL glewInit failed");
        return;
    }

    this->checkOpenGLInfo();
}

void YRendererFrontendManager::checkOpenGLInfo() {
    YINFO("******************** OpenGL GPU INFO *********************");
    i32 Max_Texture_Image_Units = 0;
    i32 Max_Combined_Texture_Image_Units = 0;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &Max_Texture_Image_Units);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &Max_Combined_Texture_Image_Units);

    YINFO("Max_Texture_Image_Units: %i", Max_Texture_Image_Units);
    YINFO("Max_Combined_Texture_Image_Units: %i" + Max_Combined_Texture_Image_Units);

    const GLubyte* byteGlVersion = glGetString(GL_VERSION);
    const GLubyte* byteGlVendor = glGetString(GL_VENDOR);
    const GLubyte* byteGlRenderer = glGetString(GL_RENDERER);
    const GLubyte* byteSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    YINFO("GL_VERSION: %s", byteGlVersion);
    YINFO("GL_VENDOR: %s", byteGlVendor);
    YINFO("GL_RENDERER: %s", byteGlRenderer);
    YINFO("GL_SHADING_LANGUAGE_VERSION: %s", byteSLVersion);

    GLint int_tmp = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &int_tmp);
    YINFO("GL_MAX_TEXTURE_SIZE: %i", int_tmp);

    int_tmp = 0;
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &int_tmp);
    YINFO("GL_MAX_RENDERBUFFER_SIZE_EXT: %i", int_tmp);
    YINFO("*************************************************************");
}
