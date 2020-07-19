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
#include "partitions.h"
#include "exceptions.h"

#include <unordered_map>
#include <vector>
#include <utility> // std::move, std::forward

namespace dyng {

/// Templated class for representing a static graph and its layout.
/**
 * (Described in section 6.1.4 -- as the alias 'graph_state')
 * 
 * @tparam NodeType Type of the node.
 * @tparam EdgeType Type of the edge.
 * 
 * @sa graph_state,
 * dynamic_graph
 */
template<typename NodeType, typename EdgeType>
class graph;


/// Specialization of the @ref graph class for a normal static graph.
/**
 * (Described in section 6.1.4)
 * 
 * @sa dyng::node,
 * dyng::edge
 */
using graph_state = graph<node, edge>;


namespace detail {

/**
 * Type alias for graph specialization used by foresighted_layout to hold information about
 * livetimes of nodes and edges.
 */
using graph_partitioning = graph<node_partition, edge_partition>;

} // namespace detail


template<typename NodeType, typename EdgeType>
class graph {
public:
    graph() = default;

    // need to revalidate edge pointers when copying
    graph(const graph<NodeType, EdgeType>& other) { *this = other; }
    graph<NodeType, EdgeType>& operator=(const graph<NodeType, EdgeType>& other) {
        m_nodes = other.m_nodes;
        m_edges = other.m_edges;
        m_index = other.m_index;
        revalidate_edge_ptrs();
        return *this;
    }

    // also need to revalidate edge pointers when moving
    graph(graph<NodeType, EdgeType>&& other) { *this = std::move(other); }
    graph<NodeType, EdgeType>& operator=(graph<NodeType, EdgeType>&& other) {
        m_nodes = std::move(other.m_nodes);
        m_edges = std::move(other.m_edges);
        m_index = std::move(other.m_index);
        revalidate_edge_ptrs();
        return *this;
    }

    // defined to comply with the rule of five
    ~graph() = default;

    /// Typedef to simplify return type of the method edges_at_node().
    using node_edges = std::unordered_map<node_id, edge_id>;

    /// Returns const reference to the vector of all nodes in the graph.
    const std::vector<NodeType>& nodes() const { return m_nodes.vec; }

    /// Returns reference to the vector of all nodes in the graph.
    /**
     * You should not add/remove
     * any elements to/from the vector (use coresponding methods instead).
     * 
     * @sa push_node,
     * emplace_node,
     * remove_node,
     * remove_nodes_if,
     * clear_nodes
     */
    std::vector<NodeType>& nodes() { return m_nodes.vec; }

    /// Returns const reference to the vector of all edges in the graph.
    const std::vector<EdgeType>& edges() const { return m_edges.vec; }

    /// Returns reference to the vector of all edges in the graph.
    /**
     * You should not add/remove
     * any elements to/from the vector (use coresponding graph methods instead).
     * 
     * @sa push_edge,
     * emplace_edge,
     * remove_edge,
     * remove_edges_if,
     * clear_edges
     */
    std::vector<EdgeType>& edges() { return m_edges.vec; }

    /// Returns reference to node of given id.
    /**
     * @throw std::out_of_range If node of @p id doesn't exist.
     */
    const NodeType& node_at(node_id id) const { return m_nodes.at(id); }

    /// Returns reference to node of given id.
    /**
     * @throw std::out_of_range If node of @p id doesn't exist.
     */
    NodeType& node_at(node_id id) { return m_nodes.at(id); }

    /// Returns reference to edge of given id.
    /**
     * @throw std::out_of_range If edge of @p id doesn't exist.
     */
    const EdgeType& edge_at(edge_id id) const { return m_edges.at(id); }

    /// Returns reference to edge of given id.
    /**
     * @throw std::out_of_range If edge of @p id doesn't exist.
     */
    EdgeType& edge_at(edge_id id) { return m_edges.at(id); }

    /// Returns the index of a node corresponding to its position in @ref nodes().
    /**
     * @throw std::out_of_range If node of @p id doesn't exist.
     */
    unsigned node_index(node_id id) const { return m_nodes.map.at(id); }

    /// Returns the index of an edge corresponding to its position in @ref edges().
    /**
     * @throw std::out_of_range If edge of @p id doesn't exist.
     */
    unsigned edge_index(edge_id id) const { return m_edges.map.at(id); }

    /**
     * Removes all edges for which parameter function returns true.
     * This methods needs to make sure all internal structures remain consistent
     * which makes it very inefficient.
     * 
     * @param function Expected signature: 'bool(const EdgeType&)'.
     */
    template<typename Function>
    void remove_edges_if(Function function) {
        // remove edges from all entries
        auto side_effect = [&function, this](const EdgeType& edge){
            bool result = function(edge);
            if (result) {
                for (auto& entry : m_index) {
                    entry.second.erase(edge.one_id());
                    entry.second.erase(edge.two_id());
                }
            }
            return result;
        };
        remove_elements_if(m_edges, side_effect);
    }

    /**
     * Removes all nodes for which parameter function returns true.
     * This methods needs to make sure all internal structures remain consistent
     * which makes it very inefficient.
     * 
     * @param function Expected signature: 'bool(const NodeType&)'.
     */
    template<typename Function>
    void remove_nodes_if(Function function) {
        // remove necessary entries from index
        auto side_effect = [&function, this](const NodeType& node){
            bool result = function(node);
            if (result) {
                // remove all edges connected to this node
                remove_edges_if([id = node.id()](const EdgeType& edge){
                    return edge.one_id() == id || edge.two_id() == id;
                });
                // remove index entry for this node
                m_index.erase(node.id());
            }
            return result;
        };
        remove_elements_if(m_nodes, side_effect);
    }

    /// Removes a single edge.
    /**
     * Has the same complexity as @ref remove_edges_if().
     * 
     * @throw invalid_graph If the edge does not exist.
     */
    void remove_edge(edge_id id) {
        if (!edge_exists(id)) {
            throw invalid_graph("edge not available");
        }
        remove_edges_if([id](const EdgeType& e){ return e.id() == id; });
    }

    /// Removes a single node.
    /**
     * Has the same complexity as @ref remove_nodes_if().
     * 
     * @throw invalid_graph If the node does not exist.
     */
    void remove_node(node_id id) {
        if (!node_exists(id)) {
            throw invalid_graph("edge not available");
        }
        remove_nodes_if([id](const NodeType& n){ return n.id() == id; });
    }

    /// Adds a new node to the graph.
    NodeType& push_node(NodeType node) {
        auto found = m_nodes.map.find(node.id());
        if (found != m_nodes.map.end()) {
            return m_nodes.vec[found->second];
        }
        m_nodes.map.emplace(node.id(), m_nodes.vec.size());
        m_index.emplace(node.id(), node_edges());
        m_nodes.vec.push_back(std::move(node));
        return m_nodes.vec.back();
    }

    /// Constructs a new node and adds it to the graph.
    template<typename ... Args>
    NodeType& emplace_node(Args&& ... args) {
        return push_node(NodeType(std::forward<Args>(args)...));
    }

    /// Adds a new edge to the graph.
    EdgeType& push_edge(EdgeType edge) {
        auto found = m_edges.map.find(edge.id());
        if (found != m_edges.map.end()) {
            return m_edges.vec[found->second];
        }
        try {
            m_index.at(edge.one_id()).emplace(edge.two_id(), edge.id());
            m_index.at(edge.two_id()).emplace(edge.one_id(), edge.id());
        } catch (std::out_of_range&) {
            throw invalid_graph("node not available");
        }
        edge.set_ptr(&m_nodes);
        m_edges.map.emplace(edge.id(), m_edges.vec.size());
        m_edges.vec.push_back(std::move(edge));
        return m_edges.vec.back();
    }

    /// Constructs a new edge and adds it to the graph.
    template<typename ... Args>
    EdgeType& emplace_edge(Args&& ... args) {
        return push_edge(EdgeType(std::forward<Args>(args)...));
    }

    /// Removes all edges.
    void clear_edges() {
        m_edges.vec.clear();
        m_edges.map.clear();
        for (auto& entry : m_index) {
            entry.second.clear();
        }
    }

    /// Removes all nodes.
    void clear_nodes() {
        m_nodes.vec.clear();
        m_nodes.map.clear();
        m_index.clear();
    }

    /// Returns whether a node of a given id exists.
    bool node_exists(node_id id) const { return m_nodes.map.count(id) > 0; }

    /// Returns whether an edge of a given id exists.
    /**
     * @sa edge_exists(node_id, node_id)
     */
    bool edge_exists(edge_id id) const { return m_edges.map.count(id) > 0; }

    /**
     * Returns whether or not there is an edge between two nodes identified by
     * node_id @p one and @p two.
     * 
     * Note: edge_exists(a, b) is equivalent to edge_exists(b, a).
     * 
     * @throw std::out_of_range If either @p one or @p two doesn't exist.
     */
    bool edge_exists(node_id one, node_id two) const {
        return edges_at_node(one).count(node_at(two).id());
    }

    /// Returns a container containing all edges adjacent to a given node.
    /**
     * The container node_edges is a map of node ids to edge ids.
     * 
     * @throw std::out_of_range If @p one doesn't exist.
     * 
     * @sa graph::node_edges
     */
    const node_edges& edges_at_node(node_id node) const {
        return m_index.at(node_at(node).id());
    }

private:
    detail::container<NodeType, node_id> m_nodes;
    detail::container<EdgeType, edge_id> m_edges;
    std::unordered_map<node_id, std::unordered_map<node_id, edge_id>> m_index;

    void revalidate_edge_ptrs() {
        for (auto& e : m_edges.vec) {
            e.set_ptr(&m_nodes);
        }
    }

    template<typename Type, typename TypeId, typename Function>
    void remove_elements_if(
            detail::container<Type,  TypeId>& elem
            , Function function) {
        // erase the elements
        elem.vec.erase(std::remove_if(elem.vec.begin(), elem.vec.end(),
                [&function](const auto& n){ return function(n); }), elem.vec.end());
        // reestablish the map
        elem.map.clear();
        for (unsigned i = 0; i < elem.vec.size(); ++i) {
            elem.map[elem.vec[i].id()] = i;
        }
    }
};

} // namespace dyng
