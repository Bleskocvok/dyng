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

#include "sdl_wrappers.h"
#include "draw_context.h"
#include "examples.h"
#include "../../dyng/dyng.h"

#include <algorithm> // std::max

namespace demo {

inline int draw_animation(int width, int height, const dyng::dynamic_graph& dgraph) {
    // initialize sdl2 library
    demo::sdl::sdl_init init;
    demo::draw_context draw_context(width, height, "dyng demo");

    dyng::interpolator interpolator;

    float state = 0;
    bool playing = true;
    bool fast = false;
    bool back = false;

    demo::sdl::timer timer;

    bool end = false;
    while (!end) {
        // controls:
        // SPACEBAR to pause/unpause
        // LEFT ARROW KEY to rewind
        // RIGHT ARROW KEY to fast forward
        // ESCAPE to exit
        // following code updates controls
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                end = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    end = true;
                } else if (event.key.keysym.sym == SDLK_SPACE) {
                    playing = !playing;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    fast = true;
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    back = true;
                }
            } else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_RIGHT) {
                    fast = false;
                } else if (event.key.keysym.sym == SDLK_LEFT) {
                    back = false;
                }
            }
        }

        // calculate frame time
        unsigned frame_time = timer.time();
        // wait 1 ms if frames are too fast
        // (vsync should be turned on, but sometimes it isn't available)
        if (frame_time == 0) {
            timer.wait(1);
            frame_time = timer.time();
        }
        timer.reset();

        if (playing) {
            state += frame_time * 0.001;
        }
        if (back) {
            state = std::max(state - frame_time * 0.005, 0.0);
        }
        if (fast) {
            state += frame_time * 0.003;
        }
        if (state >= interpolator.length(dgraph)) {
            state = interpolator.length(dgraph);
        }

        draw_context.clear();

        auto graph_state = interpolator(dgraph, state);
        for (const auto& edge : graph_state.edges()) {
            draw_context.draw_edge(edge);
        }
        for (const auto& node : graph_state.nodes()) {
            draw_context.draw_node(node);
        }

        draw_context.render();
    }
    return 0;
}

} // namespace demo
