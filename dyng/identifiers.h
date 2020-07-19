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

#include <type_traits> // std::enable_if, std::is_same, std::integral_constant
#include <functional> // std::hash
#include <ostream>

namespace dyng {

// distinctive types for identifying nodes and edges.
struct node_id;
struct edge_id;

template<typename T>
using is_id_type = std::integral_constant<bool,
        std::is_same<T, edge_id>::value || std::is_same<T, node_id>::value>;

// enables if T is either node_id or edge_id
template<typename T, typename Result>
using enable_if_id = typename std::enable_if<dyng::is_id_type<T>::value, Result>::type;

// operators for comparision and ostream function overload for edge_id and node_id

template<typename T>
enable_if_id<T, bool> operator==(const T& a, const T& b) {
    return a.value == b.value;
}

template<typename T>
enable_if_id<T, bool> operator!=(const T& a, const T& b) {
    return !(a == b);
}

template<typename T>
enable_if_id<T, bool> operator<(const T& a, const T& b) {
    return a.value < b.value;
}

template<typename T>
enable_if_id<T, bool> operator>(const T& a, const T& b) {
    return a.value > b.value;
}

template<typename T>
enable_if_id<T, bool> operator<=(const T& a, const T& b) {
    return !(a > b);
}

template<typename T>
enable_if_id<T, bool> operator>=(const T& a, const T& b) {
    return !(a < b);
}

template <typename T>
enable_if_id<T, std::ostream&> operator<<(std::ostream& out, const T& id) {
    return out << id.value;
}

/// Type used as an identifier for nodes.
/**
 * (Described in section 6.1.3)
 */
struct node_id {
    unsigned value;
    node_id(unsigned value)
            : value(value) {}
};

/// Type used as an identifier for edges.
/**
 * (Described in section 6.1.3)
 */
struct edge_id {
    unsigned value;
    edge_id(unsigned value)
            : value(value) {}
};

} // namespace dyng

namespace std {

template <>
struct hash<dyng::node_id>{
    size_t operator()(const dyng::node_id& id) const {
        return std::hash<unsigned>()(id.value);
    }
};

template <>
struct hash<dyng::edge_id>{
    size_t operator()(const dyng::edge_id& id) const {
        return std::hash<unsigned>()(id.value);
    }
};

} // namespace std
