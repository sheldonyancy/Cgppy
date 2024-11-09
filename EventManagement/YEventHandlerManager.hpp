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

#ifndef CGPPY_YEVENTHANDLERMANAGER_HPP
#define CGPPY_YEVENTHANDLERMANAGER_HPP


#include <map>
#include <memory>

#include "YEvent.hpp"
#include "YThreadSafeQueue.hpp"

class YEventHandler;


enum class YeEventHandler : unsigned char {
    NONE,
    CUTTING
};


class YEventHandlerManager {
public:
    static YEventHandlerManager* instance();

    void switchHandler(YeEventHandler type);

    inline void pushEvent(const YsEvent& event) { this->m_event_queue.push(event); }

    void pollEvents();

private:
    YEventHandlerManager();
    ~YEventHandlerManager();

private:
    YThreadSafeQueue<YsEvent> m_event_queue;

    std::map<YeEventHandler, std::unique_ptr<YEventHandler>> m_handler_map;

    YeEventHandler m_current_handler;
};


#endif //CGPPY_YEVENTHANDLERMANAGER_HPP
