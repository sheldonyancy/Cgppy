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

#include "YGlfwWindow.hpp"
#include "YLogger.h"
#include "YDefines.h"
#include "RenderingSystem/Backend/Vulkan/YVulkanBackend.hpp"
#include "YOpenGLBackend.hpp"
#include "YGlobalInterface.hpp"
#include "YSceneManager.hpp"
#include "YEvent.hpp"
#include "YEventHandlerManager.hpp"
#include "YRendererBackendManager.hpp"
#include "GLFW/glfw3.h"


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
            glm::ivec2 main_window_size = YGlobalInterface::instance()->getMainWindowSize();
            YsMouseEvent mouse_event;
            mouse_event.type = YeMouseEventType::BUTTON_DOWN;
            mouse_event.position.x = xpos;
            mouse_event.position.y = main_window_size.y - ypos;
            YEventHandlerManager::instance()->pushEvent(mouse_event);
        }
        if(GLFW_RELEASE == action) {
            mouse_left_button_is_press = false;
        }
    }
}

void glfwCursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if(mouse_left_button_is_press) {
        glm::ivec2 main_window_size = YGlobalInterface::instance()->getMainWindowSize();
        YsMouseEvent mouse_event;
        mouse_event.type = YeMouseEventType::DRAGGED;
        mouse_event.position.x = xpos;
        mouse_event.position.y = main_window_size.y - ypos;

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


YGlfwWindow::YGlfwWindow() {
    this->initVulkanEnv();
}

YGlfwWindow::~YGlfwWindow() {

}

void YGlfwWindow::eventLoop() {
    while (!glfwWindowShouldClose(this->m_glfw_window)) {
        glfwPollEvents();

        YEventHandlerManager::instance()->pollEvents();

        YRendererBackendManager::instance()->backend()->draw();
    }
}

void YGlfwWindow::initVulkanEnv() {
    glfwSetErrorCallback(glfwErrorCallback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glm::ivec2 main_window_size = YGlobalInterface::instance()->getMainWindowSize();
    this->m_glfw_window = glfwCreateWindow(main_window_size.x, main_window_size.y, "Cgppy", nullptr, nullptr);

    glfwSetKeyCallback(this->m_glfw_window, glfwKeyCallback);
    glfwSetMouseButtonCallback(this->m_glfw_window, glfwMouseButtonCallback);
    glfwSetCursorPosCallback(this->m_glfw_window, glfwCursorPositionCallback);
    glfwSetScrollCallback(this->m_glfw_window, glfwScrollCallback);
    glfwSetFramebufferSizeCallback(this->m_glfw_window, glfwFramebufferSizeCallback);
}

void YGlfwWindow::initOpenGLEnv() {
    GLenum init_glew = glewInit();

    if (GLEW_OK != init_glew) {
        YERROR("YOpenGLWindow::initializeGL glewInit failed");
        return;
    }

    this->checkOpenGLInfo();
}

void YGlfwWindow::checkOpenGLInfo() {
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