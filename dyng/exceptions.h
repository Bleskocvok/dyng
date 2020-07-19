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
 * Contains definitions for the exception used by the library.
 * 
 * @sa invalid_graph
 */

#pragma once

#include <exception> // std::exception
#include <utility> // std::move
#include <string> // std::string

namespace dyng {

/**
 * This exception is thrown if the dynamic graph is invalid.
 * Returned when dynamic_graph::build() is called if:
 *   - you attempted to add an edge connecting nodes that dont
 * exist in the current state.
 *   - you attempted to remove a node or an edge that does not
 * exist in the current state.
 */
class invalid_graph : public std::exception {
public:
    explicit invalid_graph(std::string msg)
            : m_msg(std::move(msg)) {}

    const char* what() const noexcept override { return m_msg.c_str(); }

private:
    std::string m_msg;
};

} // namespace dyng
