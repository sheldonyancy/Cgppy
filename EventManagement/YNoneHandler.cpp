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

#include "YNoneHandler.hpp"
#include "YRendererFrontendManager.hpp"
#include "YCamera.hpp"
#include "YTrackball.hpp"
#include "YSceneManager.hpp"
#include "PhysicsSystem/YPhysicsSystem.hpp"
#include "MaterialSystem/YMaterialSystem.hpp"
#include "YRendererBackendManager.hpp"
#include "RenderingSystem/Backend/Vulkan/YVulkanBackend.hpp"
#include "YLogger.h"

#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


YNoneHandler::YNoneHandler() {

}

YNoneHandler::~YNoneHandler() {

}

void YNoneHandler::handleEvent(const YsEvent& event) {
    std::visit([this](auto&& arg) { handleEvent(arg); }, event);
}

void YNoneHandler::handleEvent(const YsKeyEvent& event) {
    YINFO("%i", event.key_code);
    switch (event.key_code) {
        case 18:{

            break;
        }
        case 19:{

            break;
        }
        case 20:{

            break;
        }
        case 21:{

            break;
        }
        case 22:{

            break;
        }
        case 23:{

            break;
        }
    }
}

void YNoneHandler::handleEvent(const YsMouseEvent& event) {
    switch(event.type) {
        case YeMouseEventType::BUTTON_DOWN: {
            this->m_mouse_press = event;
            break;
        }
        case YeMouseEventType::BUTTON_UP: {
            break;
        }
        case YeMouseEventType::DRAGGED: {
            glm::fquat rotation_model = YRendererFrontendManager::instance()->trackball()->getRotation(this->m_mouse_press.position, event.position);

            YRendererBackend* backend = YRendererBackendManager::instance()->backend();
            glm::fquat rotation_camera = glm::conjugate(rotation_model);
            backend->rotatePhysicallyBasedCamera(rotation_camera);

            glm::fmat4x4 rotation_model_matrix = glm::toMat4(rotation_model);
            YSceneManager::instance()->applyRotation(rotation_model_matrix);
            YRendererBackendManager::instance()->backend()->updateHostUbo();
            this->m_mouse_press = event;
            break;
        }
        case YeMouseEventType::SCROLL: {
            glm::fmat4x4 scale_matrix;
            if(event.magnification > 0.0f) {
                scale_matrix = glm::scale(glm::fmat4x4(1.0), glm::fvec3(1.05, 1.05, 1.05));
            }
            else {
                scale_matrix = glm::scale(glm::fmat4x4(1.0), glm::fvec3(0.95, 0.95, 0.95));
            }
            YSceneManager::instance()->applyScale(scale_matrix);
            break;
        }
    }
}

void YNoneHandler::handleEvent(const YsUpdateSceneEvent& event) {
    YMaterialSystem::instance()->updagteMaterial();

    YPhysicsSystem::instance()->buildBVH();

    YRendererBackendManager::instance()->backend()->updateHostVertexInput();
    YRendererBackendManager::instance()->backend()->updateHostSsbo();
    YRendererBackendManager::instance()->backend()->updateHostUbo();
    YRendererBackendManager::instance()->backend()->setNeedDraw(true);
}

void YNoneHandler::handleEvent(const YsChangingRenderingModelEvent& event) {
    YRendererBackendManager::instance()->backend()->changingRenderingModel(static_cast<int>(event.type));
    YRendererBackendManager::instance()->backend()->updateHostUbo();
}
