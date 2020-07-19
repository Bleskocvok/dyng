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

#include "coords.h"
#include "identifiers.h"

namespace dyng {

/**
 * Class representing a node in a specific graph state.
 * It holds an id and information about it's current state and coordinates.
 * Used to animate transitions.
 * 
 * (Described in section 6.1.5)
 * 
 * @sa edge
 */
class node {
public:
    /// Sets the id of the node.
    node(node_id id)
            : m_id(id) {}

    /// Returns the id.
    node_id id() const { return m_id; }

    /// Returns if the node is new in its current state.
    bool is_new() const { return m_newly_added; }

    /// Sets if the node is new in its current state.
    void is_new(bool value) { m_newly_added = value; }

    /// Returns if the node is going to be deleted in the next state.
    bool is_old() const { return m_to_be_deleted; }

    /// Sets if the node is going to be deleted in the next state.
    void is_old(bool value) { m_to_be_deleted = value; }

    /// Returns current alpha value.
    /**
     * This is used to create an animation of the node appearing of disappearing.
     * Automatically set by @ref interpolator.
     * 
     * @sa dyng::interpolator
     */
    float alpha() const { return m_alpha; }

    /// Sets the current alpha value.
    void alpha(float value) { m_alpha = value; }

    /// Returns current coordinates.
    const coords& pos() const { return m_coords; }

    /// Returns current coordinates.
    coords& pos() { return m_coords; }

private:
    coords m_coords;
    node_id m_id;
    float m_alpha = 1.0;
    bool m_newly_added = false;
    bool m_to_be_deleted = false;
};

} // namespace dyng
