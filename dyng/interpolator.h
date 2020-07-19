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

#include "dynamic_graph.h"

#include <cmath> // std::floor, std::ceil
#include <algorithm> // std::min, std::count
#include <stdexcept> // std::out_of_range
#include <utility> // std::move

namespace dyng {

enum class phase {
    /// no transition happening
    idle,
    /// new elements appear (their alpha() goes 0.0 -> 1.0)
    appear,
    /// old elements disappear (their alpha() goes 1.0 -> 0.0)
    disappear,
    /// interpolates between current and next element positions
    morph,
    /// simultaneous transition - combination of 'appear', 'disappear' and 'morph'
    simultaneous
};

/// Tag object that indicates to use default simultaneous interpolation.
struct simultaneous {};

/// Tag object that indicates to use default phased interpolation.
struct phased {};

/// Used to create a smooth animation from a sequence of states given by a dynamic graph.
/**
 * Calls the method dynamic_graph::states() respectively to create a graph_state
 * corresponding to a given time frame.
 * 
 * (Described in section 6.1.6)
 * 
 * @sa dyng::phase
 */
class interpolator {

private:
    struct frame_state {
        float interpolation = 0;
        float alpha = 0;
        bool adding = false;
        bool added = false;
        bool deleting = false;
        bool deleted = false;
    };

public:
    /// Sets up the default order of phases.
    /**
     * Default order being: idle, disappear, morph, appear.
     * Transition lasts 2 seconds.
     * 
     * @sa dyng::phase
     */
    interpolator(phased)
            : m_phases{ phase::idle, phase::disappear, phase::morph, phase::appear } {}

    /// Sets up the default order of phases creating a simultaneous transition.
    /**
     * Default order being: idle, simultaneous.
     * Transition lasts 2 seconds.
     * 
     * @sa dyng::phase
     */
    interpolator(simultaneous)
            : m_phases{ phase::idle, phase::simultaneous } {}

    /// Sets up the default order of phases.
    /**
     * The same as interpolator(phased).
     * 
     * @sa interpolator(phased)
     */
    interpolator() : interpolator(phased{}) {}

    /// Initializes a custom order of phases.
    /**
     * The same as set_phases().
     * 
     * @sa set_phases()
     */
    interpolator(std::vector<phase> phases) {
        set_phases(std::move(phases));
    }

    /// Sets custom order of phases.
    /**
     * You can either set to use simultaneous or phased transition.
     * 
     * For phased transition all three phases have to be present exactly once:
     * phase::appear, phase::disappear, phase::morph.
     * 
     * For simultaneous transition phase::simultaneous has to be present exactly once.
     * 
     * These two approaches are incompatible. However, both can have as many idle
     * phases as you want.
     * 
     * @throw std::invalid_argument If incompatible or incorrect phases are used.
     * 
     * @sa phase
     */
    void set_phases(std::vector<phase> phases) {
        auto count = [&](phase p){
            return std::count(phases.begin(), phases.end(), p);
        };
        unsigned a = count(phase::appear);
        unsigned d = count(phase::disappear);
        unsigned m = count(phase::morph);
        unsigned s = count(phase::simultaneous);
        bool either_three = a > 0 || d > 0 || m > 0;
        bool three_correct = a == 1 && d == 1 && m == 1;
        if (s > 1 || a > 1 || d > 1 || m > 1) {
            throw std::invalid_argument("a phase != idle present multiple times");
        }
        if ((s == 0 && !three_correct)
                || (s == 1 && either_three)) {
            throw std::invalid_argument("invalid phases");
        }
        m_phases = std::move(phases);
    }

    const std::vector<phase>& get_phases() const { return m_phases; }

    /// You can read or set the duration of a specific phase type.
    /**
     * Default durations are:
     *   - phase::idle: 0.5
     *   - phase::appear: 0.25
     *   - phase::disappear: 0.25
     *   - phase::morph: 1.0
     *   - phase::simultaneous: 1.5
     */
    float& duration(phase p) {
        return const_cast<float&>(const_cast<const interpolator*>(this)->duration(p));
    }

    /// You can read the duration of a specific phase type.
    /**
     * Default durations are:
     *   - phase::idle: 0.5
     *   - phase::appear: 0.25
     *   - phase::disappear: 0.25
     *   - phase::morph: 1.0
     *   - phase::simultaneous: 1.5
     */
    const float& duration(phase p) const {
        switch (p) {
            case phase::idle: return m_idle_time;
            case phase::appear: return m_appear_time;
            case phase::disappear: return m_disappear_time;
            case phase::morph: return m_morph_time;
            case phase::simultaneous: return m_simultaneous_time;
            default: throw std::out_of_range("invalid phase");
        }
    }

    /// Returns the total length of a transition between two states.
    /**
     * It is equal to the sum of durations of all phases.
     */
    float transition_duration() const {
        float total = 0;
        for (const auto& p : m_phases) {
            total += duration(p);
        }
        return total;
    }

    /// Returns the total length of the animation.
    /**
     * It's equal to (dgraph.states().size() - 1) * transition_duration().
     * 
     * @return The length of the animation.
     */
    float length(const dynamic_graph& dgraph) const {
        return (dgraph.states().size() - 1) * transition_duration();
    }

    /// Returns a graph state representing a single frame of animation.
    /**
     * Interpolates between states and returns corresponding graph_state
     * representing a single frame of animation.
     * 
     * @param time The time to interpolate.
     * @throw std::out_of_range If time < 0 or time > length().
     */
    graph_state operator()(const dynamic_graph& dgraph, float time) const {
        if (time < 0) {
            throw std::out_of_range("time < 0");
        }
        if (time > length(dgraph)) {
            throw std::out_of_range("time > length()");
        }
        if (dgraph.states().empty()) {
            return graph_state();
        }
        unsigned index_one = std::floor(time / transition_duration());
        unsigned index_two = std::ceil(time / transition_duration());
        float value = time - index_one * transition_duration();

        frame_state animation;

        std::pair<float, unsigned> current = get_current_phase(value);
        // perform all previous phases
        for (unsigned i = 0; i < current.second; ++i) {
            perform_phase(m_phases[i], duration(m_phases[i]), animation);
        }
        // perform current phase according to current time
        perform_phase(m_phases[current.second], current.first, animation);

        index_one = std::min<unsigned>(index_one, dgraph.states().size() - 1);
        index_two = std::min<unsigned>(index_two, dgraph.states().size() - 1);
        graph_state current_state = dgraph.states()[index_one];
        const graph_state& next_state = dgraph.states()[index_two];

        // add new nodes and edges from the next state
        for (auto& n : current_state.nodes()) {
            n.is_new(false);
        }
        for (auto& e : current_state.edges()) {
            e.is_new(false);
        }
        for (auto n : next_state.nodes()) {
            if (n.is_new()) {
                n.is_old(false);
                current_state.push_node(n);
            }
        }
        for (auto e : next_state.edges()) {
            if (e.is_new()) {
                e.is_old(false);
                current_state.push_edge(e);
            }
        }

        // interpolate between the two states and assign alpha values
        for (auto& node : current_state.nodes()) {
            if (next_state.node_exists(node.id())) {
                const auto& next = next_state.node_at(node.id());
                node.pos().x = lerp(node.pos().x, next.pos().x, animation.interpolation);
                node.pos().y = lerp(node.pos().y, next.pos().y, animation.interpolation);
            }
            calc_alpha(node, animation);
        }
        for (auto& edge : current_state.edges()) {
            calc_alpha(edge, animation);
        }
        return current_state;
    }

private:
    std::vector<phase> m_phases;
    float m_idle_time = 0.5;
    float m_appear_time = 0.25;
    float m_disappear_time = 0.25;
    float m_morph_time = 1.0;
    float m_simultaneous_time = 1.5;

    float lerp(float a, float b, float value) const {
        return a + value * (b - a);
    }

    // calculates and assigns alpha to an element
    template<typename T>
    void calc_alpha(T& element, const frame_state& animation) const {
        bool is_new = element.is_new();
        bool is_old = element.is_old();
        if (!is_old && !is_new) {
            return;
        }
        if (is_new && !animation.added) {
            element.alpha(0);
        }
        if (is_old && animation.deleted) {
            element.alpha(0);
        }
        bool ape = is_new && animation.adding && !animation.added;
        bool dis = is_old && animation.deleting;
        if (ape || dis) {
            element.alpha((!ape + animation.alpha * ape)
                    * (1.0f - animation.alpha * dis));
        }
    }

    std::pair<float, unsigned> get_current_phase(float time) const {
        for (unsigned i = 0; i < m_phases.size(); ++i) {
            if (time < duration(m_phases[i])) {
                return { time, i };
            }
            time -= duration(m_phases[i]);
        }
        // this should never happen
        throw std::out_of_range("time overflow phases");
    }

    void perform_phase(phase p, float time, frame_state& animation) const {
        float d = duration(p);
        switch (p) {
            case phase::idle:
                break;
            case phase::appear:
                animation.adding = time < d;
                animation.alpha = time / d;
                if (time >= d) {
                    animation.added = true;
                }
                break;
            case phase::disappear:
                animation.deleting = time < d;
                animation.alpha = time / d;
                if (time >= d) {
                    animation.deleted = true;
                }
                break;
            case phase::morph:
                animation.interpolation = time / d;
                break;
            case phase::simultaneous:
                animation.adding = time < d;
                animation.deleting = time < d;
                animation.alpha = time / d;
                animation.interpolation = time / d;
                if (time >= d) {
                    animation.deleted = true;
                    animation.added = true;
                }
                break;
        }
    }
};

} // namespace dyng
