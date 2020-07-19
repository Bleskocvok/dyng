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
#include "mapped_graph.h"
#include "dynamic_graph.h"
#include "cooling.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <utility> // std::move
#include <algorithm> // std::max_element

namespace dyng {

/**
 * An implementation of the Foresighted Layout with Tolerance algorithm.
 * Used as a function object. Uses Layout to create a static layout of
 * a graph that represents the whole sequence. Then it applies tolerance
 * to increase the quality of individual states.
 * 
 * (Described in section 6.1.2 -- as the alias 'default_layout')
 * 
 * @tparam StaticLayout Function object that creates static layout and
 * also can be applied as singular iterations to improve the layouts.
 * 
 * @sa dyng::dynamic_graph,
 * dyng.h
 */

template <typename StaticLayout>
class foresighted_layout {

using node_live_sets = std::unordered_map<node_id, detail::live_set>;
using edge_live_sets = std::unordered_map<edge_id, detail::live_set>;

public:
    /**
     * Sets the tolerance and also sets the dimensions of the canvas where graph
     * layout will be located and also sets the center point.
     * This means that every node will be located in (x - w/2, x + w/2) and  (y - h/2, y + h/2).
     */
    foresighted_layout(
            float tolerance
            , float canvas_width
            , float canvas_height
            , coords center = coords())
            : m_tolerance(tolerance)
            , m_canvas_width(canvas_width)
            , m_canvas_height(canvas_height)
            , m_center(center) {}

    explicit foresighted_layout(float tolerance)
            : foresighted_layout(tolerance, 1, 1, { 0, 0 }) {}

    foresighted_layout()
            : foresighted_layout(0, 1, 1, { 0, 0 }) {}

    /**
     * This method sets the dimensions of the canvas where graph layout will be located
     * and sets the center point to (x, y).
     * This means that every node will be located in [x - w/2, x + w/2] and [y - h/2, y + h/2].
     */
    void set_canvas(float w, float h, coords center = coords()) {
        m_canvas_width = w;
        m_canvas_height = h;
        m_center = center;
    }

    void set_tolerance(float tolerance) { m_tolerance = tolerance; }

    /// Sets whether to use relative or absolute mental distance calculations.
    /**
     * By default relative is used.
     */
    void use_relative_distance(bool relative) { m_relative_distance = relative; }

    /// Returns the Layout object used for static layout.
    const StaticLayout& static_layout() const { return m_static_layout; }

    /// Returns the Layout object used for static layout.
    StaticLayout& static_layout() { return m_static_layout; }

    /// Sets a different cooling strategy.
    void set_cooling(cooling c) { m_cooling = std::move(c); }

    /// Performs the algorithm on a dynamic graph.
    void operator()(dynamic_graph& dgraph) {
        if (dgraph.states().empty()) {
            return;
        }
        // scale calculation canvas size to ratio
        float calculation_h = CalculationHeight;
        float calculation_w = calculation_h * m_canvas_width / m_canvas_height;

        // calculate using basic Foresighted Layout 
        basic_layout(dgraph.states(), calculation_w, calculation_h);

        // improve resulting layouts within tolerance
        if (m_tolerance != 0) {
            tolerance(dgraph.states(), calculation_w, calculation_h, m_tolerance);
        }

        // rescale to required dimensions
        for (auto& state : dgraph.states()) {
            rescale(state, calculation_w, calculation_h, m_canvas_width, m_canvas_height);
            move(state, 0.0, 0.0, m_center.x, m_center.y);
        }
    }

protected:
    static constexpr float CalculationHeight = 1;

    float m_tolerance;
    float m_canvas_width;
    float m_canvas_height;
    coords m_center;

    cooling m_cooling{ 250, 0.4, [](float t){ return t * 0.977; } };
    StaticLayout m_static_layout;
    bool m_relative_distance = true;

    // increases layout quality within tolerance
    virtual void tolerance(
            std::vector<graph_state>& states
            , float width
            , float height
            , float tolerance_value) {
        float temp = m_cooling.start_temperature;
        // when absolute mental distance should be used, make it relative to the largest graph state
        // (this doesn't defeat the purpose of it being absolute)
        // otherwise every differently sized graph would accept completely different values
        if (!m_relative_distance) {
            tolerance_value *= m_static_layout.relative_unit(width, height) * max_nodes(states);
        }
        for (unsigned i = 0; i < m_cooling.iterations; ++i) {
            for (unsigned s = 0; s < states.size(); ++s) {
                graph_state copy = states[s];
                m_static_layout.iteration(copy, width, height, temp);
                if ((s == 0 || distance(copy, states[s - 1]) < tolerance_value)
                        && (s >= states.size() - 1
                            || distance(copy, states[s + 1]) < tolerance_value)) {
                    states[s] = std::move(copy);
                }
            }
            temp = m_cooling.anneal(temp);
        }
    }

    void basic_layout(std::vector<graph_state>& states, float width, float height) {
        node_live_sets nodes_live = node_live_times(states);
        edge_live_sets edges_live = edge_live_times(states);

        // main steps
        auto supergraph = calculate_supergraph(states);
        auto gap = calculate_gap(std::move(supergraph), nodes_live, edges_live);
        auto rgap = calculate_rgap(std::move(gap));

        m_static_layout(rgap.graph(), width, height);

        // use results
        for (auto& state : states) {
            for (auto& node : state.nodes()) {
                const auto& target_node = rgap.node_at(node.id());
                node.pos() = target_node.pos();
            }
        }
    }

    void rescale(graph_state& graph
            , float src_width
            , float src_height
            , float dst_width
            , float dst_height) const {
        float w_coeff = dst_width / src_width;
        float h_coeff = dst_height / src_height;
        auto perform = [&w_coeff, &h_coeff](coords& c){
            c.x *= w_coeff;
            c.y *= h_coeff;
        };
        for (auto& node : graph.nodes()) {
            perform(node.pos());
        }
    }

    void move(graph_state& graph
            , float src_x
            , float src_y
            , float dst_x
            , float dst_y) const {
        float x_diff = dst_x - src_x;
        float y_diff = dst_y - src_y;
        auto perform = [&x_diff, &y_diff](coords& c){
            c.x += x_diff;
            c.y += y_diff;
        };
        for (auto& node : graph.nodes()) {
            perform(node.pos());
        }
    }

    node_live_sets node_live_times(const std::vector<graph_state>& states) const {
        std::unordered_map<node_id, detail::live_set> result;
        for (unsigned t = 0; t < states.size(); ++t) {
            for (const auto& node : states[t].nodes()) {
                result[node.id()].add(t);
            }
        }
        return result;
    }

    edge_live_sets edge_live_times(const std::vector<graph_state>& states) const {
        std::unordered_map<edge_id, detail::live_set> result;
        for (unsigned t = 0; t < states.size(); ++t) {
            for (const auto& edge : states[t].edges()) {
                result[edge.id()].add(t);
            }
        }
        return result;
    }

    // calculates a supergraph (a union of all edges and nodes) from the sequence
    graph_state calculate_supergraph(const std::vector<graph_state>& animation) const {
        graph_state supergraph;
        for (auto& state : animation) {
            for (const auto& node : state.nodes()) {
                supergraph.emplace_node(node.id());
            }
            for (const auto& edge : state.edges()) {
                supergraph.emplace_edge(edge.id(), edge.one_id(), edge.two_id());
            }
        }
        return supergraph;
    }

    // foresighted layout algorithm: GAP calculation
    // GAP - graph animation partitioning
    detail::mapped_graph calculate_gap(const graph_state& supergraph,
            const node_live_sets& nodes_live,
            const edge_live_sets& edges_live) const {
        detail::mapped_graph gap;
        for (auto& node : supergraph.nodes()) {
            bool exists = false;
            for (auto& partition : gap.graph().nodes()) {
                node_id partition_id = partition.id();
                if (partition.live_time().intersection(nodes_live.at(node.id())).empty()) {
                    partition.add_live_time(nodes_live.at(node.id()));
                    gap.map_node(node.id(), partition_id);
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                auto& added = gap.graph().push_node(node.id());
                added.add_live_time(nodes_live.at(node.id()));
            }
        }
        // add all partition edges
        for (auto& edge : supergraph.edges()) {
            node_id one = gap.node_at(edge.one_id()).id();
            node_id two = gap.node_at(edge.two_id()).id();
            auto& added = gap.graph().emplace_edge(edge.id(), one, two);
            added.add_live_time(edges_live.at(edge.id()));
        }
        return gap;
    }

    // foresighted layout algorithm: heuristic for calculating an RGAP
    // RGAP - reduced graph animation partitioning
    detail::mapped_graph calculate_rgap(detail::mapped_graph gap) const {
        detail::mapped_graph rgap = gap;
        rgap.clear_edges();
        std::unordered_set<edge_id> removed; // used to mark removed edges
        auto same_edge = [](const auto& a, const auto& b){
            // return if 'a' and 'b' are between the same nodes
            return (a.one_id() == b.one_id() && a.two_id() == b.two_id())
                || (a.one_id() == b.two_id() && a.two_id() == b.one_id());
        };
        for (unsigned i = 0; i < gap.graph().edges().size(); ++i) {
            auto& edge_i = gap.graph().edges()[i];
            if (!removed.count(edge_i.id())) {
                detail::edge_partition& current_partition
                        = rgap.graph().emplace_edge(edge_i.id(), edge_i.one_id(), edge_i.two_id());
                current_partition.add_live_time(edge_i.live_time());
                for (unsigned k = i + 1; k < gap.graph().edges().size(); ++k) {
                    auto& edge_k = gap.graph().edges()[k];
                    if (!removed.count(edge_k.id())
                            && same_edge(edge_i, edge_k)
                            && current_partition.live_time()
                                .intersection(edge_k.live_time()).empty()) {
                        rgap.map_edge(edge_k.id(), current_partition.id());
                        current_partition.add_live_time(edge_k.live_time());
                        removed.insert(edge_k.id());
                    }
                }
            }
        }
        // no need to remove inactive edges
        return rgap;
    }

    unsigned max_nodes(std::vector<graph_state>& states) const {
        const auto& max = *std::max_element(states.begin(), states.end(),
                [](const graph_state& a, const graph_state& b) {
                    return a.nodes().size() < b.nodes().size();
                });
        return max.nodes().size();
    }

    // calculates euclidean mental distance between two layouts
    float distance(const graph_state& one, const graph_state& two) const {
        float result = 0;
        unsigned count = 0;
        for (const auto& node : one.nodes()) {
            if (two.node_exists(node.id())) {
                const auto& other = two.node_at(node.id());
                float diff_x = node.pos().x - other.pos().x;
                float diff_y = node.pos().y - other.pos().y;
                result += std::sqrt(diff_x * diff_x + diff_y * diff_y);
                ++count;
            }
        }
        if (m_relative_distance) {
            return result / static_cast<float>(count);
        }
        return result;
    }
};

} // namespace dyng
