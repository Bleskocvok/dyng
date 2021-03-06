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

#include <functional>
#include <utility> // std::move

namespace dyng {

/// Structure representing a cooling strategy.
/**
 * Used in @ref fruchterman_reingold and @ref foresighted_layout.
 */
struct cooling {
    unsigned iterations;
    float start_temperature;
    std::function<float(float)> anneal;

    cooling(unsigned iterations, float start_temperature, std::function<float(float)> anneal)
            : iterations(iterations)
            , start_temperature(start_temperature)
            , anneal(std::move(anneal)) {}
};

} // namespace dyng
