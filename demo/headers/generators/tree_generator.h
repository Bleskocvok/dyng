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

/// Generates a random growing tree structure. 
class tree_generator : public generator {
public:
    static std::unique_ptr<generator> parse(const std::vector<std::string>& args) {
        using namespace std::string_literals;
        if (args.size() != 6) {
            throw std::runtime_error(
                "wrong arguments, usage: "s
                + args[0] + " "s + args[1]
                + " [steps] [start size] [step modification count] [seed]"s);
        }
        return std::make_unique<tree_generator>(
                std::stoi(args[2]),
                std::stoi(args[3]),
                std::stoi(args[4]),
                std::stoi(args[5]));
    }

    tree_generator(unsigned step_count
            , unsigned start_size
            , unsigned change
            , unsigned seed)
            : generator(step_count, start_size, 0, change, seed) {}

    void initial_node_step() override {
        add_element();
    }

    void initial_edge_step() override {}

    void next_step() override {
        add_element();
    }

private:
    void add_element() {
        auto node = m_result.add_node(step());
        if (!m_nodes.empty()) {
            auto other = random_node();
            add_edge(node, other);
            m_nodes.push_back(other);
        }
        m_nodes.push_back(node);
        m_nodes.push_back(node);
    }
};

} // namespace demo
