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

#include "generators/generator.h"
#include "generators/tree_generator.h"
#include "generators/coupled_generator.h"
#include "generators/grid_generator.h"
#include "generators/triangle_grid_generator.h"

#include <vector>
#include <string>
#include <sstream> // std::stringstream
#include <map>

namespace demo {

inline dyng::dynamic_graph example_dense() {
    return generate<coupled_generator>(10, 120, 40, 24, 1592672255);
}

inline dyng::dynamic_graph example_long() {
    return generate<generator>(30, 100, 100, 30);
}

inline dyng::dynamic_graph example_simple() {
    generator gen(10, 100, 60, 10, 1591);
    gen.generate();
    return gen.result();
}

inline dyng::dynamic_graph example_plant_1() {
    return generate<tree_generator>(60, 3, 3, 0);
}

inline dyng::dynamic_graph example_plant_2() {
    tree_generator gen(60, 20, 1, 2);
    gen.generate();
    return gen.result();
}

/// Parses command line arguments and returns corresponding dynamic graph example.
class example_parser {
public:
    example_parser() {
        m_examples.emplace("example_dense", example_dense);
        m_examples.emplace("example_long", example_long);
        m_examples.emplace("example_tree_1", example_plant_1);
        m_examples.emplace("example_tree_2", example_plant_2);
        m_examples.emplace("example_simple", example_simple);

        m_generators.emplace("gen_basic", generator::parse);
        m_generators.emplace("gen_coupled", coupled_generator::parse);
        m_generators.emplace("gen_tree", tree_generator::parse);
        m_generators.emplace("gen_grid", grid_generator::parse);
        m_generators.emplace("gen_triangle_grid", triangle_grid_generator::parse);
    }

    dyng::dynamic_graph operator()(const std::vector<std::string>& args) {
        using namespace std::string_literals;
        if (args.size() == 1) {
            throw std::runtime_error("wrong arguments");
        }
        if (m_examples.count(args[1])) {
            if (args.size() > 2) {
                throw std::runtime_error(
                        "wrong arguments, usage: "s
                        + args[0] + " "s + m_examples.find(args[1])->first);
            }
            return m_examples.at(args[1])();
        }
        if (m_generators.count(args[1])) {
            auto gen = m_generators.at(args[1])(args);
            gen->generate();
            return gen->result();
        }
        throw std::runtime_error("wrong arguments");
    }

    std::string help(const std::string& app_name) const {
        std::stringstream result;
        result << "usage:\n\t" << app_name << " [example/generator] [generator arguments...]\n"
                << "examples:\n";
        for (const auto& e : m_examples) {
            result << "\t" << e.first << "\n";
        }
        result << "generators:\n";
        for (const auto& g : m_generators) {
            result << "\t" << g.first << "\n";
        }
        return result.str();
    }

private:
    // uses std::function instead of simply dyng::dynamic_graph to avoid having to
    // generate each dynamic graph every time
    // this way only selected graph is generated
    std::map<std::string, std::function<dyng::dynamic_graph()>> m_examples;
    using generator_func = std::function<std::unique_ptr<generator>(const std::vector<std::string>&)>;
    std::map<std::string, generator_func> m_generators;
};

} // namespace demo
