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
#include "node.h"
#include "edge.h"
#include "exceptions.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional> // std::function
#include <utility> // std::move
#include <algorithm> // std::max_element, std::max

namespace dyng {

/// Represents the sequence of states of a dynamic graph.
/**
 * Holds a sequence of modifications applied and builds a sequence of graph states.
 * Also used to access the resulting layout.
 * 
 * (Described in section 6.1.1)
 * 
 * @sa dyng::graph_state,
 * dyng::default_layout
 */
class dynamic_graph {
public:
    /// Default constructor.
    dynamic_graph() = default;

    /**
     * Adds a node at specified time.
     * 
     * @param time Time when to create the node. When time == 0 the node counts
     * as a part of the initial state.
     * @return node_id id of the created node.
     */
    node_id add_node(unsigned time) {
        node_id id = m_last_node_id++;
        add_modification(time, [n = node(id)](graph_state& graph){
            graph.push_node(std::move(n));
        });
        return id;
    }

    /**
     * Adds an edge at specified time.
     * 
     * @param time Time when to create the edge. When time == 0 the edge counts
     * as a part of the initial state.
     * @param one One of the nodes the edge connects to.
     * @param two The other node the edge connects to.
     * @return edge_id id of the created edge.
     */
    edge_id add_edge(unsigned time, node_id one, node_id two) {
        edge_id id = m_last_edge_id++;
        add_modification(time, [e = edge(id, one, two)](graph_state& graph){
            graph.push_edge(std::move(e));
        });
        return id;
    }

    /**
     * Removes a node at specified time.
     * 
     * @param time Time to remove the node.
     * @param node The id of the node to remove.
     */
    void remove_node(unsigned time, node_id id) {
        add_modification(time, [id](graph_state& graph){
            graph.remove_node(id);
        });
    }

    /**
     * Removes an edge at specified time.
     * 
     * @param time Time to remove the edge.
     * @param edge The id of the edge to remove.
     */
    void remove_edge(unsigned time, edge_id id) {
        add_modification(time, [id](graph_state& graph){
            graph.remove_edge(id);
        });
    }

    /**
     * Builds the sequence of states from modifications queued up until this point.
     * The states can be accessed by the method @ref states().
     * 
     * @throw invalid_graph If an edge connects to non-existent node or
     * there is an attempt to remove a non-existent node or edge.
     * 
     * @sa states() const,
     * states()
     */
    void build() {
        m_states.clear();
        apply_modifications();
        set_bool_values();
    }

    /**
     * Clears the modification queue and assigns parameter @p states as its states.
     * 
     * You can use this if you have a sequence of graph states to setup the dynamic graph
     * instead of having to call a sequence of modifications and then
     * building it with @ref build().
     * 
     * @throw invalid_graph If an edge connects to non-existent node or
     * there is an attempt to remove a non-existent node or edge.
     * 
     * @sa build()
     */
    void build(std::vector<graph_state> states) {
        m_modifications.clear();
        m_states = std::move(states);
        set_bool_values();
        recalculate_ids();
    }

    /// Clears all states and queued modifications.
    void clear() {
        m_states.clear();
        m_modifications.clear();
    }

    /// Returns reference to the vector of graph states.
    /**
     * In order for this method to return non-empty vector the @ref build method has to
     * have been called once (with non-empty sequence of modifications).
     * 
     * You can access the resulting sequence of layouts after this dynamic_graph object
     * has been given to a layout object.
     * 
     * You shouldn't use this method to add/remove any elements.
     * 
     * @sa default_layout,
     * graph_state
     */
    std::vector<graph_state>& states() { return m_states; }

    /// Returns const reference to the vector of graph states.
    /**
     * In order for this method to return non-empty vector the @ref build method has to
     * have been called once (with non-empty sequence of modifications).
     * 
     * You can access the resulting sequence of layouts after this dynamic_graph object
     * has been given to a layout object.
     * 
     * @sa default_layout,
     * graph_state
     */
    const std::vector<graph_state>& states() const { return m_states; }

    /// Returns the number of unique nodes in this dynamic graph.
    /**
     * Can be inaccurate if you add nodes in the same state you remove them in.
     * That way the nodes don't end up being present in any state but
     * the return value will be greater by that number of nodes. 
     */
    unsigned node_count() const { return m_last_node_id; }

    /// Returns the number of unique edges in this dynamic graph.
    /**
     * Can be inaccurate if you add edges in the same state you remove them in.
     * That way the edges don't end up being present in any state but
     * the return value will be greater by that number of edges. 
     */
    unsigned edge_count() const { return m_last_edge_id; }

private:
    unsigned m_last_node_id = 0;
    unsigned m_last_edge_id = 0;

    std::vector<graph_state> m_states;
    std::vector<std::vector<std::function<void(graph_state&)>>> m_modifications;


    void add_modification(unsigned time, std::function<void(graph_state&)> operation) {
        if (time >= m_modifications.size()) {
            m_modifications.resize(time + 1);
        }
        m_modifications[time].push_back(operation);
    }

    void apply_modifications() {
        m_states.reserve(m_modifications.size());
        for (const auto& mod : m_modifications) {
            graph_state new_state;
            if (!m_states.empty()) {
                new_state = m_states.back();
            }
            for (const auto& operation : mod) {
                operation(new_state);
            }
            m_states.push_back(std::move(new_state));
        }
        // remove applied modifications
        m_modifications.clear();
    }

    void recalculate_ids() {
        auto id_comp = [](const auto& a, const auto& b){ return a.id() < b.id(); };
        auto max_id = [id_comp](auto begin, auto end){
            return std::max_element(begin, end, id_comp)->id().value;
        };
        for (const auto& state : m_states) {
            if (!state.nodes().empty()) {
                m_last_node_id = std::max(m_last_node_id,
                        max_id(state.nodes().begin(), state.nodes().end()) + 1);
            }
            if (!state.edges().empty()) {
                m_last_edge_id = std::max(m_last_edge_id,
                        max_id(state.edges().begin(), state.edges().end()) + 1);
            }
        }
    }

    void set_bool_values() {
        for (unsigned i = 0; i < m_states.size(); ++i) {
            for (auto& node : m_states[i].nodes()) {
                node.is_old((i < m_states.size() - 1)
                        && !m_states[i + 1].node_exists(node.id()));
                node.is_new((i > 0)
                        && !m_states[i - 1].node_exists(node.id()));
            }
            for (auto& edge : m_states[i].edges()) {
                edge.is_old((i < m_states.size() - 1)
                        && !m_states[i + 1].edge_exists(edge.id()));
                edge.is_new((i > 0)
                        && !m_states[i - 1].edge_exists(edge.id()));
            }
        }
    }
};

} // namespace dyng
