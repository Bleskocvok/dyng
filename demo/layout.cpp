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
#include <string> // std::stof

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "wrong arguments, usage: " << argv[0] << " [tolerance] [width] [height]\n";
        return 1;
    }
    dyng::default_layout layout;
    try {
        layout.set_tolerance(std::stof(argv[1]));
        layout.set_canvas(std::stof(argv[2]), std::stof(argv[3]));
    } catch (std::exception& ex) {
        std::cerr << "invalid numbers, usage: " << argv[0] << " [tolerance] [width] [height]\n";
        return 1;
    }
    dyng::dynamic_graph dgraph;
    try {
        while (std::cin >> dgraph) {
            layout(dgraph);
            std::cout << dgraph;
        }
    } catch (const std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
