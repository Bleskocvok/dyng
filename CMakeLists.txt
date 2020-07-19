
#   Copyright 2020 František Bráblík
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

cmake_minimum_required(VERSION 3.10.2)

project(dyng)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic -Wold-style-cast -O2 -pthread")

set(LIBRARIES SDL2main SDL2)

# targets for demonstration
add_executable(demo demo/demo.cpp)
add_executable(example demo/example.cpp)
add_executable(layout demo/layout.cpp)
add_executable(parallel_layout demo/parallel_layout.cpp)
add_executable(generate demo/generate.cpp)
add_executable(draw demo/draw.cpp)

# utilities
add_executable(tests test/test_main.cpp test/dyng_test.cpp)
add_executable(benchmark demo/benchmark.cpp)
add_executable(draw_states demo/draw_states.cpp)

target_link_libraries(demo ${LIBRARIES})
target_link_libraries(draw ${LIBRARIES})
target_link_libraries(draw_states ${LIBRARIES})

