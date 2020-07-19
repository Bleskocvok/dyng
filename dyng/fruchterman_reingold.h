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

#include "optimization_grid.h"
#include "cooling.h"

#include <random>
#include <cmath>
#include <unordered_map>

namespace dyng {

/**
 * An implementation of the Fruchterman and Reingold algorithm.
 * It's used as a function object. Before it performs the algorithm,
 * it first creates an initial placement using parameter InitialLayout.
 * 
 * (Referenced in section 6.1.2)
 * 
 * @tparam InitialLayout Function object that crates initial placement.
 * 
 * @sa cooling
 */
template<typename InitialLayout>
class fruchterman_reingold {

using disp_map = std::unordered_map<node_id, coords>;

public:
    /**
     * Creates a layout from initial placement of nodes on a static graph using the algorithm.
     * ([-width/2, width/2] and [-height/2, height/2])
     * All nodes are placed within those bounds.
     * 
     * @param canvas_width The width of the canvas.
     * @param canvas_height The height of the canvas.
     * @param graph Static graph to perform the algorithm on.
     */
    template<typename Graph>
    void operator()(Graph& graph, float canvas_width, float canvas_height) {
        // if no nodes, then graph must be empty
        if (graph.nodes().empty()) {
            return;
        }
        m_initial_layouter(graph, canvas_width, canvas_height);
        layout_pass(canvas_width, canvas_height, graph, m_first_cooling);
        layout_pass(canvas_width, canvas_height, graph, m_second_cooling);
    }

    /// Returns the object that crates initial placement.
    const InitialLayout& initial_layout() const { return m_initial_layouter; }

    /// Returns the object that crates initial placement.
    InitialLayout& initial_layout() { return m_initial_layouter; }

    /// Returns the relative unit that is used with temperature calculations.
    float relative_unit(float width, float height) const {
        return length(width, height) * UnitCoeff;
    }

    /// Sets the cooling strategy for the first algorithm pass.
    void set_first_cooling(cooling c) {
        m_first_cooling = std::move(c);
    }

    /// Sets the cooling strategy for the second algorithm pass.
    void set_second_cooling(cooling c) {
        m_second_cooling = std::move(c);
    }

    /// Sets the coefficient for the parameter k representing average edge length.
    /**
     * Default value is 0.6.
     */
    void set_k_coeff(float coeff) {
        m_k_coeff = coeff;
    }

    /// Sets the coefficient for border force calculations.
    /**
     * Relative to normal force calculations.
     * Default value is 0.6 meaning that border is only 0.6 times as strong
     * as repulsive forces between nodes.
     */
    void set_border_force_coeff(float coeff) {
        m_border_force = coeff;
    }

    /// Switches whether or not repulsive force should be calculated between all nodes.
    /**
     * As opposed to within a radius of 2k.
     * Switched off by default which enables a significant optimization and also the
     * resulting layouts are more pleasing. Turning this on requires other changes, namely
     * increasing set_border_force_coeff has positive effect.
     * 
     * @sa set_border_force_coeff
     */
    void use_global_repulsion(bool value) {
        m_use_global_repulsion = value;
    }

    /**
     * Does a single iteration of the algorithm with a given temperature within
     * specified bounds ([-width/2, width/2] and [-height/2, height/2]).
     * All nodes are placed within those bounds.
     * 
     * @param width The canvas width.
     * @param height The canvas height.
     * @param graph Static graph to perform the iteration on.
     * @param temperature The maximum distance a node can travel.
     * In a unit relative to canvas diagonal.
     */
    template<typename Graph>
    void iteration(Graph& graph, float width, float height, float temperature) {
        float area = width * height;
        float k = m_k_coeff * std::sqrt(area / static_cast<float>(graph.nodes().size()));
        temperature = temperature * relative_unit(width, height);

        std::vector<coords> displacements(graph.nodes().size());
        reset_and_border(graph, width, height, k, displacements);
        repulsive_forces(graph, width, height, k, temperature, displacements);
        attractive_forces(graph, k, displacements);
        displacement(graph, width, height, temperature, displacements);
    }

private:
    static constexpr float SmallOffset = 0.001f;
    static constexpr float UnitCoeff = 0.68;

    float m_border_force = 0.6;
    float m_k_coeff = 0.6;
    bool m_use_global_repulsion = false; // use local repulsion limited to the radius of 2k

    cooling m_first_cooling{ 500, 0.8, [](float t){ return t * 0.9893; } };
    cooling m_second_cooling{ 500, 0.05, [](float t){ return t * 0.993; } };

    InitialLayout m_initial_layouter;

    template<typename Graph>
    void reset_and_border(
            Graph& graph
            , float width
            , float height
            , float k
            , std::vector<coords>& disp) const {
        // reset displacement + border repulsion
        for (unsigned i = 0; i < graph.nodes().size(); ++i) {
            auto& pos = graph.nodes()[i].pos();
            disp[i].x = border_displacement(k, width, pos.x);
            disp[i].y = border_displacement(k, height, pos.y);
        }
    }

    template<typename Graph>
    void repulsive_forces(
            Graph& graph
            , float width
            , float height
            , float k
            , float t
            , std::vector<coords>& disp) const {
        std::mt19937 rand_gen(0); // to allow random displacement when necessary
        std::uniform_real_distribution<float> rand_angle(0.0f, 3.14159f * 2.0f);

        // calculate repulsive forces
        for_each_pair_of_nodes(graph, width, height, k, [&](unsigned i, unsigned j){
            auto& node_i = graph.nodes()[i];
            auto& node_j = graph.nodes()[j];
            float diff_x = node_j.pos().x - node_i.pos().x;
            float diff_y = node_j.pos().y - node_i.pos().y;
            float dst = length(diff_x, diff_y);
            if (dst == 0) {
                // this rarely happens, but when it does it needs displacement
                // otherwise resulting layout can be terrible
                float angle = rand_angle(rand_gen);
                float r = t * 0.5;
                disp[i].x -= std::cos(angle) * r;
                disp[i].y -= std::sin(angle) * r;
                disp[j].x += std::cos(angle) * r;
                disp[j].y += std::sin(angle) * r;
            } else if (m_use_global_repulsion || dst < k * 2.0f) {
                float rep_force = (1.0f / dst) * (k * k / dst);
                disp[i].x -= diff_x * rep_force;
                disp[i].y -= diff_y * rep_force;
                disp[j].x += diff_x * rep_force;
                disp[j].y += diff_y * rep_force;
            }
        });
    }

    // calls func exactly once for each pair of nodes
    template <typename Graph, typename Function>
    void for_each_pair_of_nodes(
            Graph& graph
            , float width
            , float height
            , float k
            , Function func) const {
        if (m_use_global_repulsion) {
            for (unsigned i = 0; i < graph.nodes().size(); ++i) {
                for (unsigned j = 0; j < i; ++j) {
                    func(i, j);
                }
            }
        } else {
            // setup the grid
            detail::optimization_grid m_grid(width, height, k);
            for (unsigned i = 0; i < graph.nodes().size(); ++i) {
                m_grid.add(graph.nodes()[i].pos(), i);
            }
            // iterate through all pairs that are close together
            for (unsigned i = 0; i < graph.nodes().size(); ++i) {
                auto& node_i = graph.nodes()[i];
                m_grid.for_each_around(node_i.pos(), [&](unsigned j){
                    if (j < i) {
                        func(i, j);
                    }
                });
            }
        }
    }

    template<typename Graph>
    void attractive_forces(Graph& graph, float k, std::vector<coords>& disp) const {
        for (const auto& e : graph.edges()) {
            unsigned index_one = graph.node_index(e.one_id());
            unsigned index_two = graph.node_index(e.two_id());
            auto& node_one = graph.nodes()[index_one];
            auto& node_two = graph.nodes()[index_two];
            float diff_x = node_two.pos().x - node_one.pos().x;
            float diff_y = node_two.pos().y - node_one.pos().y;
            float dst = length(diff_x, diff_y);
            if (dst != 0.0f) {
                auto& disp_one = disp[index_one];
                auto& disp_two = disp[index_two];
                float attr_force = (1.0f / dst) * (dst * dst / k);
                disp_one.x += diff_x * attr_force;
                disp_one.y += diff_y * attr_force;
                disp_two.x -= diff_x * attr_force;
                disp_two.y -= diff_y * attr_force;
            }
        }
    }

    template<typename Graph>
    void displacement(
            Graph& graph
            , float width
            , float height
            , float t
            , const std::vector<coords>& disp) const {
        for (unsigned i = 0; i < graph.nodes().size(); ++i) {
            auto& node = graph.nodes()[i];
            const auto& current_disp = disp[i];
            float disp_len = length(current_disp.x, current_disp.y);
            if (disp_len != 0) {
                float result_disp = std::min(disp_len, t) / disp_len;
                node.pos().x += result_disp * current_disp.x;
                node.pos().y += result_disp * current_disp.y;
            }
            auto clamp = [](float size, float coord){
                return std::min(size, std::max(-size, coord));
            };
            node.pos().x = clamp(width * 0.5f, node.pos().x);
            node.pos().y = clamp(height * 0.5f, node.pos().y);
        }
    }

    float border_displacement(float k, float coord, float size) const {
        auto displace = [&](float coord, float size){
            return (k * k * m_border_force) / (std::fabs(size * 0.5f - coord)
                + std::fabs(size * SmallOffset));
        };
        return displace(coord, -size)
                - displace(coord, size);
    }

    template<typename Graph>
    void layout_pass(
            float width
            , float height
            , Graph& graph
            , const cooling& c) {
        float t = c.start_temperature;
        for (unsigned r = 0; r < c.iterations; ++r) {
            iteration(graph, width, height, t);
            t = c.anneal(t);
        }
    }

    float pow2(float one) const {
        return one * one;
    }

    // calculates the length of hypotenuse
    float length(float a, float b) const {
        return std::sqrt(pow2(a) + pow2(b));
    }
};

} // namespace dyng
