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

#include "graph.h"

#include <unordered_map>
#include <utility> // std::move

namespace dyng {

namespace detail {

/// Wrapper class for graph used to represent graph partitioning.
/**
 * Enables multiple ids to point to the same node/edge.
 * Used internally in @ref foresighted_layout.
 * 
 * node_at and edge_at both return either node / edge returned by
 * graph::node_at / graph::edge_at or the associated node/edge that has
 * been associated to another node / edge in the graph by the method map_node / map_edge.
 * 
 * @sa graph,
 * foresighted_layout
 */
class mapped_graph {
public:
    mapped_graph() = default;

    mapped_graph(graph_partitioning g)
            : m_graph(std::move(g)) {}

    const graph_partitioning& graph() const { return m_graph; }
    graph_partitioning& graph() { return m_graph; }

    node_partition& node_at(node_id id) {
        return const_cast<node_partition&>(const_cast<const mapped_graph*>(this)->node_at(id));
    }

    const node_partition& node_at(node_id id) const {
        auto found = m_node_map.find(id);
        if (found != m_node_map.end()) {
            return m_graph.node_at(found->second);
        }
        return m_graph.node_at(id);
    }

    edge_partition& edge_at(edge_id id) {
        return const_cast<edge_partition&>(const_cast<const mapped_graph*>(this)->edge_at(id));
    }

    const edge_partition& edge_at(edge_id id) const {
        auto found = m_edge_map.find(id);
        if (found != m_edge_map.end()) {
            return m_graph.edge_at(found->second);
        }
        return m_graph.edge_at(id);
    }

    /**
     * Associates another node_id with a node of target id.
     * Used for partitioning a graph.
     * 
     * @param node Id to assign to the target id.
     * @param target The target id.
     * @throw std::out_of_range If node_id is invalid.
     */
    void map_node(node_id node, node_id target) {
        m_node_map.emplace(node, target);
    }

    /**
     * Associates another edge_id with an edge of target id.
     * Used for partitioning a graph.
     * 
     * @param edge Id to assign to the target id.
     * @param target The target id.
     * @throw std::out_of_range If edge_id is invalid.
     */
    void map_edge(edge_id edge, edge_id target) {
        m_edge_map.emplace(edge, target);
    }

    void clear_nodes() {
        m_graph.clear_nodes();
        m_node_map.clear();
    }

    void clear_edges() {
        m_graph.clear_edges();
        m_edge_map.clear();
    }

private:
    graph_partitioning m_graph;
    std::unordered_map<node_id, node_id> m_node_map;
    std::unordered_map<edge_id, edge_id> m_edge_map;
};

} // namespace detail

} // namespace dyng
