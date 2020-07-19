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

#include "generator.h"

namespace demo {

/// Generates a growing grid.
class grid_generator : public generator {
public:
    static std::unique_ptr<generator> parse(const std::vector<std::string>& args) {
        using namespace std::string_literals;
        if (args.size() != 3) {
            throw std::runtime_error(
                "wrong arguments, usage: "s
                + args[0] + " "s + args[1]
                + " [size]"s);
        }
        return std::make_unique<grid_generator>(
                std::stoi(args[2]));
    }

    grid_generator(unsigned steps)
            : generator(steps + 1) {}

    void next_step() override {
        unsigned middle = step() - 1;
        std::vector<dyng::node_id> next_layer;
        for (unsigned i = 0; i < prev_layer.size(); ++i) {
            if (i == middle) {
                auto a = add_node();
                auto b = add_node();
                auto c = add_node();
                if (!next_layer.empty()) {
                    add_edge(next_layer.back(), a);
                }
                add_edge(prev_layer[i], a);
                add_edge(prev_layer[i], c);
                add_edge(a, b);
                add_edge(b, c);
                next_layer.push_back(a);
                next_layer.push_back(b);
                next_layer.push_back(c);
            } else {
                auto node = add_node();
                add_edge(prev_layer[i], node);
                if (!next_layer.empty()) {
                    add_edge(next_layer.back(), node);
                }
                next_layer.push_back(node);
            }
        }
        prev_layer = next_layer;
    }

    void initial_node_step() override {}
    void initial_edge_step() override {}

    void initial_setup() override {
        prev_layer.push_back(add_node());
    }

private:
    std::vector<dyng::node_id> prev_layer;
};

} // namespace demo
