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
#include "headers/examples.h"

#include <chrono>
#include <iostream>
#include <iomanip> // std::setw, std::fixed, std::setprecision
#include <string> // std::stoi

void header(int w) {
    std::cout << std::setw(w) << "size" << " | "
            << std::setw(w) << "nodes" << " | "
            << std::setw(w) << "edges" << " | "
            << std::setw(w) << "d=0" << " | "
            << std::setw(w) << "d>0" << " | "
            << std::setw(w) << "par d>0" << " | "
            <<"\n";
    for (int i = 0; i < 6; ++i) {
        for (int k = 0; k < w; ++k) {
            std::cout << "-";
        }
        std::cout << "-+-";
    }
    std::cout << std::endl;
}

template<typename Layouter>
float measure_time(dyng::dynamic_graph graph, Layouter& layout, bool tolerance = false) {
    using clock = std::chrono::steady_clock;
    layout.set_tolerance(tolerance ? 0.1 : 0);
    clock::time_point start = clock::now();
    layout(graph);
    clock::time_point now = clock::now();

    // force compiler to not optimize out any calculations when using -O2
    volatile auto deoptimizer = graph;

    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
    return us / 1000000.0f;
}

int main(int argc, char** argv) {
    int repeat = 1;
    int threads = 4;
    std::vector<int> sizes{ 4, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80 };

    auto error = [&](){
        std::cerr << "wrong arguments\n"
                << "usage: " << argv[0] << " (iterations=1) (threads=4)\n";
    };
    if (argc > 3) {
        error();
        return 1;
    }
    if (argc == 2) {
        try {
            repeat = std::stoi(argv[1]);
        } catch (const std::exception& ex) {
            error();
            return 1;
        }
    } else if (argc == 3) {
        try {
            threads = std::stoi(argv[2]);
        } catch (const std::exception& ex) {
            error();
            return 1;
        }
    }

    int w = 8;
    std::cout << "iterations: " << repeat << "\n"
            << "threads: " << threads << std::endl;
    header(w);

    std::cout << std::fixed << std::setprecision(2);

    dyng::default_layout layout;
    dyng::default_layout_parallel layout_par(threads, 0.0);
    for (int size : sizes) {
        float notol_total = 0;
        float tol_total = 0;
        float par_tol_total = 0;
        dyng::dynamic_graph graph = demo::generate<demo::grid_generator>(size);
        for (int i = 0; i < repeat; ++i) {
            notol_total += measure_time(graph, layout, false);
            tol_total += measure_time(graph, layout, true);
            par_tol_total += measure_time(graph, layout_par, true);
        }
        std::cout << std::setw(w) << size << " | "
                << std::setw(w) << graph.node_count() << " | "
                << std::setw(w) << graph.edge_count() << " | "
                << std::setw(w - 1) << notol_total / static_cast<float>(repeat) << "s" << " | "
                << std::setw(w - 1) << tol_total / static_cast<float>(repeat) << "s" << " | "
                << std::setw(w - 1) << par_tol_total / static_cast<float>(repeat) << "s" << " | "
                << std::endl;
    }
    return 0;
}
