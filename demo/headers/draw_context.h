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

#include "sdl_wrappers.h"

#include "../../dyng/dyng.h"

namespace demo {

class draw_context {
public:
    draw_context(int width
            , int height
            , const char* title)
            : m_window_renderer(width, height, title)
            , width(width)
            , height(height) {
        m_point = m_window_renderer.make_circle(16);
        m_line = m_window_renderer.make_black_pixel();
    }

    void clear() {
        m_window_renderer.clear();
    }

    void render() {
        m_window_renderer.render();
    }

    void screenshot(const std::string &filename) const {
        m_window_renderer.screenshot(filename);
    }

    void draw_node(const dyng::node& node) {
        float dx = width * 0.5f;
        float dy = height * 0.5f;
        float size = node.alpha() * NodeSize;
        float x = node.pos().x + dx - size * 0.5f;
        float y = node.pos().y + dy - size * 0.5f;

        m_window_renderer.draw(
                m_point,
                std::round(x),
                std::round(y),
                size,
                size);
    }

    void draw_edge(const dyng::edge& edge) {
        float disp_x = width * 0.5f;
        float disp_y = height * 0.5f;
        float diff_x = edge.node_one().pos().x - edge.node_two().pos().x;
        float diff_y = edge.node_one().pos().y - edge.node_two().pos().y;
        float angle = std::atan2(diff_y, diff_x) / 3.14159f * 180.0f;
        float length = std::sqrt(diff_x * diff_x + diff_y * diff_y);
        float mid_x = (edge.node_one().pos().x + edge.node_two().pos().x) * 0.5f + disp_x - length * 0.5f;
        float mid_y = (edge.node_one().pos().y + edge.node_two().pos().y) * 0.5f + disp_y;

        m_window_renderer.draw_rotated(
                m_line,
                std::round(mid_x),
                std::round(mid_y),
                length,
                EdgeWidth,
                angle,
                edge.alpha());
    }

private:
    static constexpr float NodeSize = 11;
    static constexpr unsigned EdgeWidth = 2;

    sdl::window_renderer m_window_renderer;
    sdl::types::texture m_point;
    sdl::types::texture m_line;
    int width;
    int height;
};

} // namespace demo
