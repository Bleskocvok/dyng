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
 * This file contains tests for things that can't be tested visually.
 */

#include "catch2/catch.hpp"

#include "../dyng/dyng.h"
#include "../demo/headers/examples.h"

#include <map>
#include <iterator> // std::next
#include <sstream> // std::stringstream

using namespace dyng;

TEST_CASE("building dynamic graph") {
    dynamic_graph graph;
    CHECK_NOTHROW(graph.build());
    REQUIRE(graph.states().size() == 0);
    auto one = graph.add_node(0);
    auto two = graph.add_node(1);
    graph.add_edge(2, one, two);

    auto new_node = graph.add_node(10);
    auto new_edge = graph.add_edge(5, one, two);
    auto old_node = graph.add_node(1);
    REQUIRE_NOTHROW(graph.build());
    REQUIRE(graph.states().size() == 11);
    REQUIRE(!graph.states().at(4).edge_exists(new_edge));
    REQUIRE(graph.states().at(5).edge_exists(new_edge));
    REQUIRE(graph.states().at(6).edge_exists(new_edge));
    REQUIRE(!graph.states().at(9).node_exists(new_node));
    REQUIRE(graph.states().at(10).node_exists(new_node));
    REQUIRE(!graph.states().at(0).node_exists(old_node));
    REQUIRE(graph.states().at(1).node_exists(old_node));
    REQUIRE(graph.states().at(2).node_exists(old_node));
    REQUIRE(graph.states().at(10).node_exists(old_node));
    SECTION("check 'build(vector)' overload") {
        dynamic_graph other;
        CHECK_NOTHROW(other.build());
        node_id not_existing = other.add_node(100);
        unsigned count = graph.node_count();
        for (unsigned i = 0; i < count; ++i) {
            not_existing = other.add_node(100);
        }
        REQUIRE_NOTHROW(other.build(graph.states())); // resets
        REQUIRE(other.states().size() == 11);
        REQUIRE(!other.states().at(4).edge_exists(new_edge));
        REQUIRE(other.states().at(5).edge_exists(new_edge));
        REQUIRE(other.states().at(6).edge_exists(new_edge));
        REQUIRE(!other.states().at(9).node_exists(new_node));
        REQUIRE(other.states().at(10).node_exists(new_node));
        REQUIRE(!other.states().at(0).node_exists(old_node));
        REQUIRE(other.states().at(1).node_exists(old_node));
        REQUIRE(other.states().at(2).node_exists(old_node));
        REQUIRE(other.states().at(10).node_exists(old_node));
    }
    SECTION("check clear") {
        graph.clear();
        CHECK_NOTHROW(graph.build());
        REQUIRE(graph.states().size() == 0);
    }
}

TEST_CASE("exceptions check") {
    SECTION("dangling edge ignorance") {
        dynamic_graph graph;
        auto one = graph.add_node(0);
        auto two = graph.add_node(0);
        auto edge = graph.add_edge(0, one, two);
        graph.add_node(3);
        SECTION("correct nothrow") {
            CHECK_NOTHROW(graph.build());
        }
        SECTION("edge remove + correct afterwards") {
            graph.remove_node(1, one);
            CHECK_NOTHROW(graph.build());
            CHECK(!graph.states()[1].edge_exists(edge));
            CHECK(!graph.states()[2].edge_exists(edge));
            CHECK_THROWS(graph.states()[1].edge_exists(one, two));
            CHECK_THROWS(graph.states()[1].edge_exists(two, one));
        }
        SECTION("remove other nothrow") {
            graph.remove_node(2, two);
            CHECK_NOTHROW(graph.build());
        }
    }
    SECTION("wrong edge nodes") {
        dynamic_graph graph;
        auto one = graph.add_node(3);
        auto two = graph.add_node(3);
        graph.add_edge(2, one, two);
        CHECK_THROWS(graph.build());
        CHECK_THROWS_AS(graph.build(), invalid_graph);
    }
    SECTION("remove wrong edge or node") {
        dynamic_graph graph;
        auto a = graph.add_node(1);
        auto b = graph.add_node(2);
        auto c = graph.add_node(3);
        auto ab = graph.add_edge(4, a, b);
        auto bc = graph.add_edge(5, b, c);
        auto ac = graph.add_edge(6, a, c);
        SECTION("1") {
            graph.remove_edge(3, ab);
            CHECK_THROWS_AS(graph.build(), invalid_graph);
        }
        SECTION("2") {
            graph.remove_edge(4, bc);
            CHECK_THROWS_AS(graph.build(), invalid_graph);
        }
        SECTION("3") {
            graph.remove_edge(1, ab);
            CHECK_THROWS_AS(graph.build(), invalid_graph);
        }
        SECTION("4") {
            graph.remove_edge(7, ac);
            CHECK_NOTHROW(graph.build());
        }
        SECTION("5") {
            graph.remove_node(7, a);
            CHECK_NOTHROW(graph.build());
        }
        SECTION("6") {
            graph.remove_node(7, a);
            graph.remove_node(8, a);
            CHECK_THROWS_AS(graph.build(), invalid_graph);
        }
        SECTION("7") {
            graph.remove_node(1, c);
            CHECK_THROWS_AS(graph.build(), invalid_graph);
        }
    }
    SECTION("interpolator out of range") {
        dynamic_graph graph;
        auto one = graph.add_node(0);
        auto two = graph.add_node(0);
        graph.add_edge(5, one, two);
        graph.build();
        interpolator interpolator;
        CHECK_NOTHROW(interpolator(graph, interpolator.length(graph) / 2.0));
        CHECK_NOTHROW(interpolator(graph, interpolator.length(graph)));
        CHECK_NOTHROW(interpolator(graph, 0));
        CHECK_THROWS(interpolator(graph, interpolator.length(graph) + 0.01));
        CHECK_THROWS(interpolator(graph, -0.01));
    }
    SECTION("interpolator invalid phases") {
        interpolator interpolator;
        SECTION("correct phases") {
            CHECK_NOTHROW(interpolator.set_phases({
                phase::simultaneous }));
            CHECK_NOTHROW(interpolator.set_phases({
                phase::morph,
                phase::appear,
                phase::disappear }));
            CHECK_NOTHROW(interpolator.set_phases({
                phase::morph,
                phase::idle,
                phase::appear,
                phase::idle,
                phase::disappear }));
        }
        SECTION("incorrect phases") {
            CHECK_THROWS(interpolator.set_phases({
                phase::appear,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::appear,
                phase::appear,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::appear,
                phase::simultaneous,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::simultaneous,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::appear,
                phase::disappear }));
            CHECK_THROWS(interpolator.set_phases({
                phase::morph,
                phase::appear,
                phase::disappear,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::morph,
                phase::morph,
                phase::appear,
                phase::disappear,
                phase::simultaneous }));
            CHECK_THROWS(interpolator.set_phases({
                phase::morph,
                phase::morph,
                phase::appear,
                phase::disappear }));
        }
    }
    SECTION("empty graph") {
        dynamic_graph graph;
        CHECK_NOTHROW(graph.build());
        default_layout layout(1);
        CHECK_NOTHROW(layout(graph));
    }
}

TEST_CASE("bool values") {
    dynamic_graph graph;
    auto one = graph.add_node(0);
    auto two = graph.add_node(0);
    graph.add_edge(0, one, two);
    graph.remove_node(1, one);
    graph.remove_node(2, two);
    graph.build();
    SECTION("old elements in the last state") {
        for (auto& node : graph.states()[graph.states().size() - 1].nodes()) {
            CHECK(!node.is_old());
        }
        for (auto& edge : graph.states()[graph.states().size() - 1].edges()) {
            CHECK(!edge.is_old());
        }
    }
    SECTION("new elements in the last state") {
        for (auto& node : graph.states()[0].nodes()) {
            CHECK(!node.is_new());
        }
        for (auto& edge : graph.states()[0].edges()) {
            CHECK(!edge.is_new());
        }
    }
    SECTION("general graph") {
        dynamic_graph graph = demo::generate<demo::generator>();
        for (unsigned i = 0; i < graph.states().size(); ++i) {
            for (const auto& n : graph.states()[i].nodes()) {
                if (i < graph.states().size() - 1) {
                    REQUIRE(n.is_old() != graph.states()[i + 1].node_exists(n.id()));
                }
                if (i > 0) {
                    REQUIRE(n.is_new() != graph.states()[i - 1].node_exists(n.id()));
                }
            }
        }
    }
}

TEST_CASE("id usability") {
    SECTION("compare operators") {
        node_id one = 1;
        node_id two = 2;
        CHECK(one == one);
        CHECK(one <= one);
        CHECK(one >= one);
        CHECK(two >= one);
        CHECK(one <= two);
        CHECK(two >= two);
        CHECK(two <= two);
        CHECK(two > one);
        CHECK(one < two);
        CHECK(one != two);
    }
    SECTION("used in std::map") {
        std::map<node_id, int> map;
        map.emplace(node_id(1), 1);
        map.emplace(node_id(200), 1);
        map.emplace(node_id(3), 1);
        map.emplace(node_id(2), 1);
        REQUIRE(map.begin()->first == node_id(1));
        REQUIRE(std::next(map.begin())->first == node_id(2));
        REQUIRE(std::next(map.begin(), 2)->first == node_id(3));
        REQUIRE(std::next(map.begin(), 3)->first == node_id(200));
    }
}

TEST_CASE("parallel FLT - set_threads") {
    dynamic_graph dgraph = demo::generate<demo::generator>();
    default_layout_parallel layout(2, 0.04);
    layout.set_threads(4);
    REQUIRE_NOTHROW(layout(dgraph));
}

TEST_CASE("copying graph") {
    graph_state graph;
    graph.emplace_node(0);
    graph.emplace_node(1);
    graph.emplace_node(2);
    graph.emplace_edge(0, 0, 1);
    graph.emplace_edge(1, 1, 2);
    graph.node_at(0).pos().x = 666.0f;
    graph.node_at(0).pos().y = 420.0f;
    graph.node_at(1).pos().x = 1.0f;
    graph.node_at(1).pos().y = 36.0f;
    auto check = [](const auto& graph){
        CHECK(graph.edge_at(0).node_one().pos().x == 666.0f);
        CHECK(graph.edge_at(0).node_one().pos().y == 420.0f);
        CHECK(graph.edge_at(1).node_one().pos().x == 1.0f);
        CHECK(graph.edge_at(1).node_one().pos().y == 36.0f);
    };
    check(graph);
    graph_state copy = graph;
    check(copy);
    graph_state copy2;
    for (unsigned i = 0; i < 10; ++i) {
        copy2.emplace_node(i);
    }
}

TEST_CASE("parser") {
    SECTION("simple") {
        std::stringstream str("n 666 1.5 3.6;");
        node n(789);
        str >> n;
        REQUIRE(n.id() == node_id(666));
        REQUIRE(n.pos().x == 1.5f);
        REQUIRE(n.pos().y == 3.6f);
    }
    SECTION("full process simple") {
        dynamic_graph dgraph = demo::generate<demo::generator>();
        std::stringstream str;
        REQUIRE_NOTHROW(str << dgraph);
        dynamic_graph other;
        REQUIRE_NOTHROW(str >> dgraph);
        default_layout layout(0.04);
        REQUIRE_NOTHROW(layout(dgraph));
    }
}
