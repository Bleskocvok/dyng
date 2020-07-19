/*
   Copyright 2020 František Bráblík

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <memory> // std::unique_ptr
#include <utility> // std::move
#include <cmath> // std::ceil

namespace dyng {

namespace detail {

// (unfortunately std::barrier exists since C++20, so cannot be used here)
// using this method (generations) from Boost barrier implementation:
// https://stackoverflow.com/questions/24465533/implementing-boostbarrier-in-c11
// https://stackoverflow.com/questions/26190853/why-does-the-boost-library-use-an-m-generation-variable-in-its-implementation-of
/// A barrier for thread synchronization.
class barrier {
public:
    barrier(unsigned count)
            : m_size(count)
            , m_current(count) {}

    void reset(unsigned count) {
        m_size = count;
        m_current = count;
        m_gen = 0;
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        unsigned gen = m_gen;
        if (--m_current == 0) {
            ++m_gen;
            m_current = m_size;
            m_cv.notify_all();
        } else {
            m_cv.wait(lock, [this, gen](){ return gen != m_gen; });
        }
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    unsigned m_size;
    unsigned m_current = 0;
    unsigned m_gen = 0;
};


/// A thread pool.
/**
 * Used internally by @ref parallel_foresighted_layout.
 */
class parallel {

struct param {
    std::function<void()> func;
    bool run{ false };
};

public:
    parallel(unsigned count)
            : m_bar(count) {
        init(count);
    }

    ~parallel() { quit(); }

    /// Returns the number of threads.
    unsigned count() const { return m_threads.size() + 1; }

    /// Assigns a job to a thread.
    std::function<void()>& operator[](unsigned index) { return m_params[index].func; }
    const std::function<void()>& operator[](unsigned index) const { return m_params[index].func; }

    /// Performs assigned jobs and waits for them to complete.
    void perform() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            run_all();
        }
        m_cv.notify_all();
        // perform this threads function
        if (m_params[0].func) {
            m_params[0].func();
        }
        // barrier till all threads have finished
        m_bar.wait();
    }

    /// Removes all assigned jobs from threads.
    void clear() {
        for (unsigned i = 0; i < count(); ++i) {
            m_params[i].run = true;
        }
    }

    /// Assigns respective jobs to all threads and launches them.
    /**
     * Expected Func signature: void(unsigned thread).
     */
    template<typename Func>
    void for_each(Func func) {
        for (unsigned i = 0; i < count(); ++i) {
            operator[](i) = [i, func](){
                func(i);
            };
        }
        perform();
    }

    /// Assigns respective jobs to all threads and launches them.
    /**
     * Also calculates appropriate indices to split the task evenly.
     * Expected signature: void(unsigned begin, unsigned end).
     */
    template<typename Func>
    void for_each(unsigned size, const Func& func) {
        for (unsigned i = 0; i < count(); ++i) {
            auto chunk = get_chunk(i, size);
            operator[](i) = [&func, chunk](){
                func(chunk.first, chunk.second);
            };
        }
        perform();
    }

    /// Assigns respective jobs to all threads in an interleaved way and launches them.
    /**
     * 'Interleaved way' meaning that if count() == 2
     * the two threads get (0, 2, 4, 6, 8) and (1, 3, 5, 7)
     * instead of (0, 1, 2, 3, 4) and (5, 6, 7, 8).
     * 
     * Expected signature: void(unsigned begin, unsigned step).
     */
    template<typename Func>
    void for_each_interleaved(Func func) {
        for (unsigned i = 0; i < count(); ++i) {
            operator[](i) = [i, c = count(), func](){
                func(i, c);
            };
        }
        perform();
    }

    /// Returns beginning index and "past-the-end" index of a task chunk for a thread of given index.
    std::pair<unsigned, unsigned> get_chunk(unsigned thread, unsigned size) const {
        unsigned chunk = std::ceil(size / static_cast<float>(count()));
        unsigned start = 0;
        for (unsigned i = 0; i <= thread; ++i) {
            unsigned count = chunk;
            if (start + count > size) {
                count = size - start;
            }
            if (i == thread) {
                return { start, start + count };
            }
            start += count;
        }
        return { start, start + chunk };
    }

private:
    std::vector<std::thread> m_threads;
    std::unique_ptr<param[]> m_params;
    bool m_end{ false };
    std::mutex m_mutex;
    std::condition_variable m_cv;
    barrier m_bar{ 0 };

    void thread_func(param* p) {
        while (true) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [&](){ return p->run; });
            p->run = false;
            lock.unlock();
            if (m_end) {
                break;
            }
            if (p->func) {
                p->func();
            }
            m_bar.wait();
        }
    }

    void run_all() {
        for (unsigned i = 0; i < count(); ++i) {
            m_params[i].run = true;
        }
    }

    void init(unsigned count) {
        if (count == 0) {
            throw std::invalid_argument("initializing 0 threads");
        }
        m_threads.clear();
        m_bar.reset(count);
        m_params = std::make_unique<param[]>(count);
        m_threads.reserve(count - 1);
        for (unsigned i = 1; i < count; ++i) {
            std::thread th(&parallel::thread_func, this, &m_params[i]);
            m_threads.emplace_back(std::move(th));
        }
    }

    // turns off all threads
    void quit() {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_end = true;
            run_all();
        }
        m_cv.notify_all();
        for (auto& th : m_threads) {
            th.join();
        }
    }
};

} // namespace detail

} // namespace dyng
