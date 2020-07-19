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

/// Generates a random dynamic graph similar to @ref generator, but more coupled together.
class coupled_generator : public generator {
public:
    static std::unique_ptr<generator> parse(const std::vector<std::string>& args) {
        using namespace std::string_literals;
        if (args.size() != 7) {
            throw std::runtime_error(
                "wrong arguments, usage: "s
                + args[0] + " "s + args[1]
                + " [steps] [start node count] [start edge count] [step modification count] [seed]"s);
        }
        return std::make_unique<coupled_generator>(
                std::stoi(args[2]),
                std::stoi(args[3]),
                std::stoi(args[4]),
                std::stoi(args[5]),
                std::stoi(args[6]));
    }

    coupled_generator(unsigned step_count = 10
            , unsigned start_nodes = 10
            , unsigned start_edges = 6
            , unsigned change = 2
            , unsigned seed = 0)
            : generator(step_count, start_nodes, start_edges, 1, seed)
            , m_change_ratio(change / static_cast<float>(start_nodes)) {}

    void initial_node_step() override {
        add_element();
    }

    void next_step() override {
        unsigned changes = m_nodes.size() * m_change_ratio;
        for (unsigned i = 0; i < changes; ++i) {
            switch (rand_int(0, 3)) {
                case 0:
                    if (rand_int(0, 1) == 0) {
                        add_random_node_edge();
                    } else {
                        add_element();
                    }
                    break;
                case 1:
                    add_random_node_edge();
                    break;
                case 2:
                    add_random_edge();
                    break;
                case 3:
                    remove_random_edge();
                    break;
            }
        }
    }

    void initial_setup() override {
        add_random_node();
    }

private:
    float m_change_ratio;

    void add_element() {
        auto one = add_node();
        auto two = add_node();
        add_edge(one, two);
    }

    void add_random_node_edge() {
        auto node = m_result.add_node(step());
        if (!m_nodes.empty()) {
            auto other = random_node();
            add_edge(node, other);
        }
        m_nodes.push_back(node);
    }
};

} // namespace demo
