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
#include <unordered_map>

namespace dyng {

namespace detail {

/// Contains a vector of graph entities and a map to them.
/**
 * Used internally by @ref graph and @ref edge ( @ref basic_edge).
 * 
 * @sa graph
 * edge
 * basic_edge
 */
template<typename T, typename Id>
struct container {
    std::vector<T> vec;
    std::unordered_map<Id, unsigned> map;
    const T& at(Id id) const { return vec[map.at(id)]; }
    T& at(Id id) { return vec[map.at(id)]; }
};

} // namespace detail

} // namespace dyng
