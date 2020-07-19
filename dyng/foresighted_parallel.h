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

#include "foresighted_layout.h"
#include "parallel.h"

#include <memory>

namespace dyng {

/**
 * A parallel implementation of the Foresighted Layout with Tolerance algorithm.
 * More specifically, it only uses parallel execution in the most performance demanding
 * section.
 * 
 * Produces the same results as @ref foresighted_layout but quicker when tolerance > 0 is
 * used.
 * 
 * (Described in section 6.2)
 * 
 * @sa foresighted_layout
 */
template<typename StaticLayout>
class parallel_foresighted_layout : public foresighted_layout<StaticLayout> {
public:
    parallel_foresighted_layout(
            unsigned threads
            , float tolerance
            , float canvas_width
            , float canvas_height
            , coords center = coords())
            : foresighted_layout<StaticLayout>(tolerance, canvas_width, canvas_height, center)
            , m_parallel(std::make_unique<detail::parallel>(threads)) {}

    /// Initializes this with a given number of threads and given tolerance.
    parallel_foresighted_layout(unsigned threads, float tolerance)
            : parallel_foresighted_layout(threads, tolerance, 1, 1) {}

    /// Initializes this with the default thread count of 4 and tolerance = 0.
    parallel_foresighted_layout()
            : parallel_foresighted_layout(4, 0) {}

    /// Sets the thread count.
    void set_threads(unsigned count) {
        m_parallel = std::make_unique<detail::parallel>(count);
    }

private:
    // is a unique ptr because class parallel is neither copyable nor movable
    std::unique_ptr<detail::parallel> m_parallel;

    void tolerance(
            std::vector<graph_state>& states
            , float width
            , float height
            , float tolerance_value) override {
        float temp = this->m_cooling.start_temperature;
        if (!this->m_relative_distance) {
            tolerance_value *= this->m_static_layout.relative_unit(width, height)
                    * this->max_nodes(states);
        }
        detail::barrier bar(m_parallel->count());
        std::vector<graph_state> copies = states;
        std::vector<bool> apply(states.size());
        auto get = [&](unsigned i) -> const graph_state& {
            if (apply[i]) {
                return copies[i];
            }
            return states[i];
        };
        // there is no benefit in iterating sequentially, so we can split the
        // indices not in chunks but in an interleaved way;
        // this makes it faster for incrementally growing dynamic graphs
        // because the tasks are split more evenly
        m_parallel->for_each_interleaved([&](unsigned begin, unsigned step){
            for (unsigned r = 0; r < this->m_cooling.iterations; ++r) {
                for (unsigned i = begin; i < states.size(); i += step) {
                    if (apply[i]) {
                        states[i] = copies[i];
                    } else {
                        copies[i] = states[i];
                    }
                }
                for (unsigned i = begin; i < states.size(); i += step) {
                    this->m_static_layout.iteration(copies[i], width, height, temp);
                }
                bar.wait();
                if (begin == 0) {
                    // only the first thread will do this
                    // this has to be sequential
                    for (unsigned i = 0; i < states.size(); ++i) {
                        apply[i] = false;
                        if ((i == 0 || this->distance(copies[i], get(i - 1)) < tolerance_value)
                                && (i >= states.size() - 1
                                    || this->distance(copies[i], states[i + 1]) < tolerance_value)) {
                            apply[i] = true;
                        }
                    }
                    temp = this->m_cooling.anneal(temp);
                }
                bar.wait();
            }
        });
    }
};

} // namespace dyng
