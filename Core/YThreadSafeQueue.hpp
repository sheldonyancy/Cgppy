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

#ifndef CGPPY_YTHREADSAFEQUEUE_HPP
#define CGPPY_YTHREADSAFEQUEUE_HPP


#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>


template<typename T>
class YThreadSafeQueue {
public:
    YThreadSafeQueue() {}
    ~YThreadSafeQueue() {}

    void push(T value) {
        std::shared_ptr<T> data(std::make_shared<T>(std::move(value)));
        {
            std::lock_guard<std::mutex> lk(this->m_mtx);
            this->m_queue.push(data);
        }
        this->m_data_cond.notify_one();
    }

    std::shared_ptr<T> tryPop() {
        std::lock_guard<std::mutex> lk(this->m_mtx);
        if (this->m_queue.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res = this->m_queue.front();
        this->m_queue.pop();
        return res;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(this->m_mtx);
        return this->m_queue.empty();
    }

private:
    mutable std::mutex m_mtx;
    std::queue<std::shared_ptr<T>> m_queue;
    std::condition_variable m_data_cond;
};


#endif //CGPPY_YTHREADSAFEQUEUE_HPP
