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
#include "headers/parse_generate.h"

// you can set this to true/false
// whether or not you want the demo to use parallel implementation
#define USE_PARALLEL false

int main(int argc, char** argv) {
    dyng::dynamic_graph dgraph;
    int ret;
    if ((ret = demo::parse_generate(dgraph, argc, argv)) != 0) {
        return ret;
    }
    #if USE_PARALLEL == true
        dyng::default_layout_parallel layout(4, 0);
    #else
        dyng::default_layout layout;
    #endif
    layout.set_tolerance(0.04);
    layout.set_canvas(1024, 640);
    layout(dgraph);
    return demo::draw_animation(1280, 720, dgraph);
}
