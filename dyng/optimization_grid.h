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

#include <vector>
#include <cmath> // std::floor, ceil
#include <algorithm> // std::min, max

namespace dyng {

namespace detail {

/// Represents a grid of squares holding nodes.
/**
 * Internally used by the class @ref fruchterman_reingold to significantly
 * optimize the algorithm.
 * 
 * @sa dyng::fruchterman_reingold
 */
class optimization_grid {
public:
    optimization_grid() = default;

    optimization_grid(float w, float h, float k) {
        reset(w, h, k);
    }

    void add(coords pos, unsigned index) {
        int x = std::floor((pos.x + m_w * 0.5f) / m_2k);
        int y = std::floor((pos.y + m_h * 0.5f) / m_2k);
        get(x, y).push_back(index);
    }

    template<typename Function>
    void for_each_around(coords pos, Function func) {
        int pos_x = std::floor((pos.x + m_w * 0.5f) / m_2k);
        int pos_y = std::floor((pos.y + m_h * 0.5f) / m_2k);
        for (int y = std::max(pos_y - 1, 0); y <= std::min(pos_y + 1, m_grid_h - 1); ++y) {
            for (int x = std::max(pos_x - 1, 0); x <= std::min(pos_x + 1, m_grid_w - 1); ++x) {
                for (auto& i : get(x, y)) {
                    func(i);
                }
            }
        }
    }

    void clear() {
        m_indices.clear();
    }

    void reset(float w, float h, float k) {
        m_2k = 2.0f * k;
        m_w = w;
        m_h = h;
        m_grid_w = std::ceil(w / m_2k);
        m_grid_h = std::ceil(h / m_2k);
        m_indices.resize(m_grid_w * m_grid_h);
    }

private:
    float m_2k = 0;
    float m_w = 0;
    float m_h = 0;
    int m_grid_w = 0;
    int m_grid_h = 0;
    std::vector<std::vector<unsigned>> m_indices;

    std::vector<unsigned>& get(int x, int y) {
        return m_indices[y * m_grid_w + x];
    }
};

} // namespace detail

} // namespace dyng
