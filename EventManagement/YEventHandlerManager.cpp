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

#include "YEventHandlerManager.hpp"
#include "YNoneHandler.hpp"
#include "YCuttingHandler.hpp"


YEventHandlerManager* YEventHandlerManager::instance() {
    static YEventHandlerManager m;
    return &m;
}

YEventHandlerManager::YEventHandlerManager() {
    this->m_handler_map.try_emplace(YeEventHandler::NONE, std::make_unique<YNoneHandler>());
    this->m_handler_map.try_emplace(YeEventHandler::CUTTING, std::make_unique<YCuttingHandler>());

    this->m_current_handler = YeEventHandler::NONE;
}

YEventHandlerManager::~YEventHandlerManager() {

}

void YEventHandlerManager::switchHandler(YeEventHandler type) {
    this->m_current_handler = type;
}

void YEventHandlerManager::pollEvents() {
    if (!this->m_event_queue.empty()) {
        std::shared_ptr<YsEvent> event = this->m_event_queue.tryPop();
        this->m_handler_map[this->m_current_handler]->handleEvent(*event);
    }
}