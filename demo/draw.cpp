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
#include "headers/draw_animation.h"

#include <iostream>

int main(int argc, char** argv) {
    int w;
    int h;
    if (argc != 3) {
        std::cerr << "wrong arguments, usage: " << argv[0] << " [width] [height]\n";
        return 1;
    }
    try {
        w = std::stoi(argv[1]);
        h = std::stoi(argv[2]);
    } catch (const std::exception& ex) {
        std::cerr << "invalid numbers, usage: " << argv[0] << " [width] [height]\n";
        return 1;
    }
    dyng::dynamic_graph dgraph;
    try {
        std::cin >> dgraph;
    } catch (std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
    return demo::draw_animation(w, h, dgraph);
}
