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

#include "node.h"
#include "edge.h"
#include "live_set.h"

namespace dyng {

namespace detail {

/// Represents a node that holds information about its live times.
/**
 * Used internally by @ref foresighted_layout.
 * 
 * @sa dyng::live_time,
 * foresighted_layout
 */
class node_partition : public node {
public:
    node_partition(node_id id)
            : node(id) {}

    void add_live_time(const live_set& node_live) {
        m_live_time.join(node_live);
    }

    const live_set& live_time() const { return m_live_time; }
    live_set& live_time() { return m_live_time; }

private:
    live_set m_live_time;
};

/// Represents an edge that holds information about its live times.
/**
 * Used internally by @ref foresighted_layout.
 * 
 * @sa dyng::live_time,
 * foresighted_layout
 */
class edge_partition : public basic_edge<node_partition> {
public:
    edge_partition(edge_id id, node_id one, node_id two)
            : basic_edge(id, one, two) {}

    void add_live_time(const live_set& edge_live) {
        m_live_time.join(edge_live);
    }

    const live_set& live_time() const { return m_live_time; }
    live_set& live_time() { return m_live_time; }

private:
    live_set m_live_time;
};

} // namespace detail

} // namespace dyng
