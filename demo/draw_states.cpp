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
#include "../dyng/dyng.h"
#include "headers/sdl_wrappers.h"
#include "headers/draw_context.h"
#include "headers/examples.h"

#include <string> // std::to_string
#include <cstdlib> // std::atoi
#include <iostream>

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "wrong arguments, usage: " << argv[0] << " [width] [height]\n";
        return 1;
    }
    int w = std::atoi(argv[1]);
    int h = std::atoi(argv[2]);
    dyng::dynamic_graph dgraph;
    try {
        std::cin >> dgraph;
    } catch (std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n";
        return 1;
    }

    demo::sdl::sdl_init init;
    demo::draw_context draw_context(w + 16, h + 16, "draw states");

    for (unsigned i = 0; i < dgraph.states().size(); ++i) {
        draw_context.clear();
        const auto& state = dgraph.states()[i];
        for (const auto& edge : state.edges()) {
            draw_context.draw_edge(edge);
        }
        for (const auto& node : state.nodes()) {
            draw_context.draw_node(node);
        }
        draw_context.render();
        draw_context.screenshot(std::to_string(i) + ".bmp");
    }

    return 0;
}
