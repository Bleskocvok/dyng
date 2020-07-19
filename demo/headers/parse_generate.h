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

#include "examples.h"

#include <iostream>

namespace demo {

inline int parse_generate(dyng::dynamic_graph& result, int argc, char** argv) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
    demo::example_parser parser;
    if (args.size() == 2
            && (args[1] == "help"
                || args[1] == "h"
                || args[1] == "--help"
                || args[1] == "-h")) {
        std::cout << "HELP:\n"
                << parser.help(args[0]);
        return 1;
    }
    try {
        result = parser(args);
    } catch (std::exception& ex) {
        std::cerr << "ERROR: " << ex.what() << "\n"
                << "HELP:\n"
                << parser.help(args[0]);
        return 1;
    }
    return 0;
}

} // namespace demo
