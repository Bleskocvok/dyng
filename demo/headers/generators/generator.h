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

#include "../../../dyng/dyng.h"

#include <random>
#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

namespace demo {

template<typename Type, typename... Args>
dyng::dynamic_graph generate(Args&&... args) {
    Type generator(std::forward<Args>(args)...);
    generator.generate();
    return generator.result();
}

/// Basic random dynamic graph generator.
/**
 * Simply repeats a given number of random modifications.
 * 
 * Each modification being one of:
 *   - add node
 *   - remove node
 *   - add edge
 *   - remove edge
 */
class generator {

template<typename T = int>
using uid = std::uniform_int_distribution<T>;

struct edge_entry {
    dyng::edge_id id;
    dyng::node_id one;
    dyng::node_id two;
};

public:
    static std::unique_ptr<generator> parse(const std::vector<std::string>& args) {
        using namespace std::string_literals;
        if (args.size() != 7) {
            throw std::runtime_error(
                "wrong arguments, usage: "s
                + args[0] + " "s + args[1]
                + " [steps] [start node count] [start edge count] [step modification count] [seed]"s);
        }
        return std::make_unique<generator>(
                std::stoi(args[2]),
                std::stoi(args[3]),
                std::stoi(args[4]),
                std::stoi(args[5]),
                std::stoi(args[6]));
    }

    generator(unsigned step_count = 10
            , unsigned start_nodes = 1
            , unsigned start_edges = 0
            , unsigned change = 1
            , unsigned seed = 0)
            : m_gen(seed)
            , m_step_count(step_count)
            , m_start_nodes(start_nodes)
            , m_start_edges(start_edges)
            , m_change(change) {}

    virtual ~generator() = default;

    dyng::dynamic_graph result() const {
        return m_result;
    }

    virtual void generate() {
        m_step = 0;
        initial_setup();
        for (unsigned i = 0; i < m_start_nodes; ++i) {
            initial_node_step();
        }
        for (unsigned i = 0; i < m_start_edges; ++i) {
            initial_edge_step();
        }
        ++m_step;
        for (unsigned i = 1; i < m_step_count; ++i) {
            for (unsigned k = 0; k < m_change; ++k) {
                next_step();
            }
            ++m_step;
        }
        m_result.build();
    }

    virtual void initial_node_step() {
        add_random_node();
    }

    virtual void initial_edge_step() {
        add_random_edge();
    }

    virtual void next_step() {
        switch (rand_int(0, 2)) {
            case 0:
                add_random_node();
                break;
            case 1:
                remove_random_node();
                break;
            case 2:
                add_random_edge();
                break;
        }
    }

    virtual void initial_setup() {}

private:
    std::mt19937 m_gen{ 0 };
    unsigned m_step_count = 10;
    unsigned m_start_nodes = 10;
    unsigned m_start_edges = 6;
    unsigned m_change = 3;
    unsigned m_step = 0;

protected:
    dyng::dynamic_graph m_result;
    std::vector<dyng::node_id> m_nodes;
    std::vector<edge_entry> m_edges;

    unsigned step() const {
        return m_step;
    }

    int rand_int(int min, int max) {
        uid<> dis(min, max);
        return dis(m_gen);
    }

    void remove_random_node() {
        if (m_nodes.empty()) {
            return;
        }
        remove_node(m_nodes[rand_int(0, m_nodes.size() - 1)]);
    }

    void remove_random_edge() {
        if (m_edges.empty()) {
            return;
        }
        remove_edge(m_edges[rand_int(0, m_edges.size() - 1)].id);
    }

    dyng::node_id add_node() {
        auto node = m_result.add_node(m_step);
        m_nodes.push_back(node);
        return node;
    }

    dyng::edge_id add_edge(dyng::node_id one, dyng::node_id two) {
        auto edge = m_result.add_edge(m_step, one, two);
        m_edges.push_back({ edge, one, two });
        return edge;
    }

    dyng::node_id random_node() {
        return m_nodes[rand_int(0, m_nodes.size() - 1)];
    }

    void add_random_node() {
        add_node();
    }

    void add_random_edge() {
        if (!m_nodes.empty()) {
            add_edge(random_node(), random_node());
        }
    }

    void remove_node(dyng::node_id id) {
        auto del_it = std::remove_if(m_nodes.begin(),
                    m_nodes.end(),
                    [del = id](dyng::node_id id){ return id == del; });
        for (auto it = del_it; it != m_nodes.end(); ++it) {
            m_edges.erase(std::remove_if(m_edges.begin(),
                    m_edges.end(),
                    [del = *it](edge_entry entry){
                        return entry.one == del || entry.two == del;
                    }),
                m_edges.end());
        }
        m_nodes.erase(del_it, m_nodes.end());
        m_result.remove_node(m_step, id);
    }

    void remove_edge(dyng::edge_id id) {
        m_edges.erase(std::remove_if(m_edges.begin(), 
                    m_edges.end(),
                    [del = id](edge_entry entry){ return entry.id == del; }),
                m_edges.end());
        m_result.remove_edge(m_step, id);
    }
};

} // namespace demo
