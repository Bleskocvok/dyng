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

#include <cmath>
#include <algorithm> // std::min

namespace dyng {

/**
 * Function object that creates an initial placement of nodes and edges.
 * Places nodes in a circle around the center of the canvas.
 * 
 * @param canvas_width Width of the canvas.
 * @param canvas_height Height of the canvas.
 * @param graph Graph to create the initial placement of.
 */
class initial_placement {
public:
    template<typename Graph>
    void operator()(Graph& graph, float canvas_width, float canvas_height) {
        float radius = std::min(canvas_width, canvas_height) * 0.333f;
        float angle = 2.0f * 3.14159f / graph.nodes().size();
        for (unsigned i = 0; i < graph.nodes().size(); ++i) {
            graph.nodes()[i].pos().x = std::cos(i * angle) * radius;
            graph.nodes()[i].pos().y = std::sin(i * angle) * radius;
        }
    }
};

} // namespace dyng
