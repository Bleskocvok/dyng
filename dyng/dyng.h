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
 * This is the main file of the library.
 * This file contains type aliases for the library objects meant for simple use.
 * This is the file that you should include in your project if you want to use the library.
 * 
 * @sa dyng::default_layout
 * dyng::default_layout_parallel
 */

#pragma once

#include "dynamic_graph.h"
#include "interpolator.h"
#include "parse.h"

#include "foresighted_layout.h"
#include "foresighted_parallel.h"
#include "fruchterman_reingold.h"
#include "initial_placement.h"

namespace dyng {

/**
 * Type alias for a specialization of foresighted_layout that uses fruchterman_reingold
 * with trivial initial placement.
 * 
 * (Described in section 6.1.2)
 * 
 * @sa dyng::foresighted_layout,
 * dyng::fruchterman_reingold,
 * dyng::initial_placement
 */
using default_layout = foresighted_layout<fruchterman_reingold<initial_placement>>;

/**
 * The same algorithms as @ref default_layout, but uses a multithreaded implementation
 * @ref parallel_foresighted_layout instead of @ref foresighted_layout.
 * 
 * @sa dyng::parallel_foresighted_layout,
 * dyng::default_layout
 */
using default_layout_parallel = parallel_foresighted_layout
        <fruchterman_reingold<initial_placement>>;

} // namespace dyng
