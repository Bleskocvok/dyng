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

#include <vector>
#include <algorithm> // set functions
#include <iterator> // set functions

namespace dyng {

namespace detail {

/// Represents a set of all states where a node or an edge exists.
/**
 * Used internally by @ref foresighted_layout.
 */
class live_set {
public:
    void add(unsigned time) {
        m_values.push_back(time);
    }

    live_set intersection(const live_set& other) const {
        live_set result;
        std::set_intersection(m_values.begin(), m_values.end(),
                other.m_values.begin(), other.m_values.end(),
                std::back_inserter(result.m_values));
        return result;
    }

    live_set setunion(const live_set& other) const {
        live_set result;
        std::set_union(m_values.begin(), m_values.end(),
                other.m_values.begin(), other.m_values.end(),
                std::back_inserter(result.m_values));
        return result;
    }

    // adds all values from other to this
    void join(const live_set& other) {
        m_values = setunion(other).m_values;
    }

    bool empty() const { return m_values.empty(); }

private:
    std::vector<unsigned> m_values;
};

} // namespace detail

} // namespace dyng
