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
/**
 * @file
 * 
 * This file contains the definitons of functions used for parsing input and
 * output in a custom text format.
 * Defined for all classes that are used in the hierarchy to describe a dynamic
 * graph.
 * 
 * (Described in section 6.1.7 -- text format)
 * 
 * @sa graph,
 * dynamic_graph,
 * node,
 * edge
 */
#pragma once

#include "dynamic_graph.h"

#include <ostream>
#include <istream>
#include <string> // string literals
#include <cctype> // std::isspace
#include <stdexcept>

namespace dyng {

namespace detail {

inline std::string reduce_whitespace(const std::string& str) {
    std::string result;
    bool ignore = false;
    for (auto& ch : str) {
        bool white = std::isspace(ch);
        if (!ignore || !white) {
            result += white ? ' ' : ch;
        }
        ignore = white;
    }
    if (!result.empty() && result.front() == ' ') {
        result.erase(result.begin());
    }
    if (!result.empty() && result.back() == ' ') {
        result.pop_back();
    }
    return result;
}

inline std::vector<std::string> split(const std::string& str, char dlm = ' ') {
    std::vector<std::string> result;
    std::string chunk;
    for (unsigned i = 0; i < str.length(); ++i) {
        if (str[i] != dlm) {
            chunk += str[i];
        }
        if (str[i] == dlm || i == str.length() - 1) {
            result.push_back(chunk);
            chunk.clear();
        }
    }
    return result;
}

inline bool skip_until(std::istream& in, char ch) {
    do {
        if (!in.good()) {
            return false;
        }
    } while (in.get() != ch);
    return true;
}

inline std::string read_until(std::istream& in, char ch) {
    using namespace std::string_literals;
    std::string result;
    char last;
    do {
        if (!in.good()) {
            throw std::runtime_error("stream ended, expected '"s + ch + "'"s);
        }
        last = in.get();
        if (last != ch) {
            result += last;
        }
    } while (last != ch);
    return result;
}

inline void validate(char ch) {
    using namespace std::string_literals;
    if (ch == std::char_traits<char>::eof()) {
        throw std::runtime_error("unexpected end of input");
    }
    if (!std::isspace(ch)) {
        throw std::runtime_error("unexpected character '"s + ch + "'"s);
    }
}

} // namespace detail


inline std::ostream& operator<<(std::ostream& out, const node& n) {
    out << "n " << n.id()
        << " " << n.pos().x
        << " " << n.pos().y
        << ";";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const edge& e) {
    out << "e " << e.id()
        << " " << e.one_id()
        << " " << e.two_id()
        << ";";
    return out;
}

template<typename N, typename E>
inline std::ostream& operator<<(std::ostream& out, const graph<N, E>& g) {
    out << "[\n";
    for (const node& n : g.nodes()) {
        out << n << "\n";
    }
    for (const edge& e : g.edges()) {
        out << e << "\n";
    }
    out << "]\n";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, const dynamic_graph& dgraph) {
    out << "{\n";
    for (const auto& s : dgraph.states()) {
        out << s;
    }
    out << "}\n";
    return out;
}

inline std::istream& operator>>(std::istream& in, node& n) {
    if (!detail::skip_until(in, 'n')) {
        return in;
    }
    std::string str = detail::read_until(in, ';');
    str = detail::reduce_whitespace(str);
    auto bits = detail::split(str);
    if (bits.size() != 3) {
        throw std::runtime_error("invalid number of node parameters");
    }
    try {
        n = node(std::stoi(bits[0]));
        n.pos().x = std::stof(bits[1]);
        n.pos().y = std::stof(bits[2]);
    } catch (const std::exception&) {
        throw std::runtime_error("invalid node parameters");
    }
    return in;
}

inline std::istream& operator>>(std::istream& in, edge& e) {
    if (!detail::skip_until(in, 'e')) {
        return in;
    }
    std::string str = detail::read_until(in, ';');
    str = detail::reduce_whitespace(str);
    auto bits = detail::split(str);
    if (bits.size() != 3) {
        throw std::runtime_error("invalid number of edge parameters");
    }
    try {
        e = edge(std::stoi(bits.at(0)), std::stoi(bits.at(1)), std::stoi(bits.at(2)));
    } catch (const std::exception&) {
        throw std::runtime_error("invalid edge parameters");
    }
    return in;
}

template<typename N, typename E>
inline std::istream& operator>>(std::istream& in, graph<N, E>& g) {
    if (!detail::skip_until(in, '[')) {
        return in;
    }
    while (in.good()) {
        switch (in.peek()) {
            case ']':
                in.get();
                return in;
            case 'n': {
                node n(1337); // any value, will be overwritten
                in >> n;
                g.push_node(n);
            } break;
            case 'e': {
                edge e(1337, 666, 420); // any values, will be overwritten
                in >> e;
                g.push_edge(e);
            } break;
            default:
                detail::validate(in.get());
                break;
        }
    }
    return in;
}

inline std::istream& operator>>(std::istream& in, dynamic_graph& dgraph) {
    if (!detail::skip_until(in, '{')) {
        return in;
    }
    std::vector<graph_state> states;
    while (in.good()) {
        switch (in.peek()) {
            case '}':
                dgraph.build(std::move(states));
                in.get();
                return in;
            case '[': {
                graph_state state;
                in >> state;
                states.push_back(std::move(state));
            } break;
            default:
                detail::validate(in.get());
            break;
        }
    }
    throw std::out_of_range("stream ended, expected '}'");
    return in;
}

} // namespace dyng
