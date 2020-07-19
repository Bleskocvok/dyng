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

#include "coords.h"
#include "identifiers.h"
#include "container.h"

namespace dyng {

/**
 * Class representing an edge in a specific graph state.
 * It holds an id and information about it's current state and coordinates.
 * Used to animate transitions.
 * 
 * (Described in section 6.1.5)
 * 
 * @sa node
 */
class edge;

/**
 * A base class that @ref edge and @ref edge_partition are derived from.
 * 
 * @tparam ConnectedNode Type of node this edge connects to.
 */
template<typename ConnectedNode>
class basic_edge {
public:
    /// Sets the id of the edge and ids of two connected nodes.
    basic_edge(edge_id id, node_id one, node_id two)
            : m_one(one)
            , m_two(two)
            , m_id(id) {}

    /// Returns the id of one connected node.
    node_id one_id() const { return m_one; }

    /// Returns the id of the other connected node.
    node_id two_id() const { return m_two; }

    edge_id id() const { return m_id; }

    /// Returns if the edge is new in its current state.
    bool is_new() const { return m_newly_added; }

    /// Sets if the node is new in its current state.
    void is_new(bool value) { m_newly_added = value; }

    /// Returns if the edge is going to be deleted in the next state.
    bool is_old() const { return m_to_be_deleted; }

    /// Sets if the node is going to be deleted in the next state.
    void is_old(bool value) { m_to_be_deleted = value; }

    /// Returns current alpha value.
    /**
     * This is used to create an animation of the edge appearing of disappearing.
     * Automatically set by @ref interpolator.
     * 
     * @sa dyng::interpolator
     */
    float alpha() const { return m_alpha; }

    /// Sets the current alpha value.
    void alpha(float value) { m_alpha = value; }

    /// Returns a reference to one connected node.
    ConnectedNode& node_one() { return m_container->at(m_one); }
    
    /// Returns a const reference to one connected node.
    const ConnectedNode& node_one() const { return m_container->at(m_one); }

    /// Returns a reference to the other connected node.
    ConnectedNode& node_two() { return m_container->at(m_two); }

    /// Returns a const reference to the other connected node.
    const ConnectedNode& node_two() const { return m_container->at(m_two); }

    /// Sets the pointer to the object @ref detail::container.
    /**
     * The internal pointer to the container allows the methods @ref one and @ref two
     * to exist.
     * 
     * Set internally by @ref graph.
     * 
     * @sa graph
     * graph_state
     */
    void set_ptr(detail::container<ConnectedNode, node_id>* cont) {
        m_container = cont;
    }

private:
    node_id m_one;
    node_id m_two;
    edge_id m_id;
    float m_alpha = 1.0;
    bool m_newly_added = false;
    bool m_to_be_deleted = false;

    detail::container<ConnectedNode, node_id>* m_container = nullptr;
};


class edge : public basic_edge<node> {
public:
    /// Sets the id of the edge and ids of two connected nodes.
    edge(edge_id id, node_id one, node_id two)
            : basic_edge(id, one, two) {}
};

} // namespace dyng
