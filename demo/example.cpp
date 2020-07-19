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

#include <iostream>

// dummy macros
#define DRAW_DOT(x, y) (void)(x);(void)(y)
#define DRAW_LINE(x1, y1, x2, y2) (void)(x1);(void)(y1);(void)(x2);(void)(y2)

int main() {
    // initialize dynamic graph
    dyng::dynamic_graph dgraph;

    // add series of modifications
    auto a = dgraph.add_node(0);        // add node 'a' to initial state
    auto b = dgraph.add_node(0);        // add node 'b' to initial state
    dgraph.add_edge(0, a, b);           // add edge between 'a' and 'b' in initial state
    auto c = dgraph.add_node(1);        // add node 'c' in step 1
    auto bc = dgraph.add_edge(1, b, c); // add edge between 'b' and 'c' in step 1
    dgraph.remove_edge(2, bc);          // remove edge between 'b' and 'c' in step 2
    dgraph.remove_node(3, c);           // remove node 'c' in step 3
    // ...

    // build states from modifications
    dgraph.build();

    // initialize function object for computing layout
    dyng::default_layout layout;
    layout.set_canvas(1024, 640);
    layout.set_tolerance(0.05);

    // compute a layout using tolerance of 0.05
    // and canvas size of 1024x640
    layout(dgraph);

    // use interpolator for smooth animation
    dyng::interpolator inter;
    float timeslice = 0.016; // 16 ms
    for (float t = 0; t <= inter.length(dgraph); t += timeslice) {
        dyng::graph_state frame = inter(dgraph, t);
        // draw edges
        for (const auto& e : frame.edges()) {
            DRAW_LINE(e.node_one().pos().x, e.node_one().pos().y, e.node_two().pos().x, e.node_two().pos().y);
        }
        // draw nodes
        for (const auto& n : frame.nodes()) {
            DRAW_DOT(n.pos().x, n.pos().y);
        }
    }

    // you can also output the result using a simple custom format
    std::cout << dgraph;
    return 0;
}
