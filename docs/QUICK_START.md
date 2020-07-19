# How to use the dyng library
To include the library in a project, you only need to put the folder `dyng/` in the project directory and include the header `dyng/dyng.h`.

```cpp
#include "dyng/dyng.h"
```

However, the project needs to be using at least the version C++14 for the **dyng** library to compile.

## Simple example
Here is a simple code example showing how to create a dynamic graph. For full example code that can be compiled refer to `demo/example.cpp`.

```cpp
dyng::dynamic_graph dgraph;

auto a = dgraph.add_node(0);
auto b = dgraph.add_node(1);
auto ab = dgraph.add_edge(2, a, b);
// possibly more modifications ...

dgraph.build();
```

After calling the `build` method, individual graph states can be accessed. However, they don't contain any layout information yet. Class `default_layout` serves the purpose of calculating that information.

```cpp
dyng::default_layout layout;
layout.set_tolerance(0.05);

layout(dgraph);

// now you can access individual layouts using:
dgraph.states();
// this way you can access specific node coordinates
// for instance:
dgraph.states()[0].nodes()[0].pos().x;
```

## Canvas size

A different constructor can be used to set canvas dimensions and the center point of the coordinate system. For instance:

```cpp
float w = 1920;
float h = 1080;
float tolerance = 0.04;
dyng::default_layout layout(tolerance, w, h);
```

Which is equivalent to:

```cpp
float w = 1920;
float h = 1080;
float x = 0;
float y = 0;
float tol = 0.04;
dyng::default_layout layout(tol, w, h, { x, y });
```

## Parallel implementation

You can use `default_layout_parallel` instead of `default_layout`, which uses a given number of threads to speed up the layout computation process.

The interface is the same as `default_layout`, except the constructor takes one more parameter -- the number of threads to use. It also contains the method `set_threads` which serves the same purpose.

```cpp
dyng::default_layout_parallel layout;
layout.set_threads(4);
layout.set_tolerance(0.04);
// equivalent to
// dyng::default_layout_parallel layout(4, 0.04);
```

## Using `interpolator`
After you have built a dynamic graph and computed a layout, a smooth animation can be created by generating a series of graph states at given timeslices.

```cpp
dyng::interpolator inter;
float timeslice = 0.016; // 16 milliseconds
for (float t = 0; t <= inter.length(dgraph); t += timeslice) {
    dyng::graph_state frame = inter(dgraph, t);
    // here you can display 'frame'
}
```

To draw a single `graph_state`, individual nodes and edges can be accessed along with their coordinates. The class `interpolator` sets the `alpha` values of nodes and edges according to the current phase. You can also use the methods `is_new` and `is_old` that have been set appropriately when the method `dynamic_graph::build` was called.

Using two hypothetical functions `draw_line` and `draw_dot`, nodes and edges can be rendered the following way:

```cpp
for (const auto& e : frame.edges()) {
    draw_line(e.node_one().pos().x, e.node_one().pos().y, e.node_two().pos().x, e.node_two().pos().y);
}
for (const auto& n : frame.nodes()) {
    draw_dot(n.pos().x, n.pos().y);
}
```

You can use `e.alpha()` to draw the edge with appropriate levels of transparency in order to display it appearing/disappearing. You can also use `e.is_old()` or `e.is_new()` to differentiate appearing/disappearing edges (by using different colors for example). The same can be used for nodes.

To set the order of phases during individual transitions between states (`dynamic_graph::states`) the method `interpolator::set_phases` can be used. Additionally, the method `duration` can be used to read or change the duration of each phase. Refer to the full documentation for more information on how phases work.

The library provides two default presets for phases. They can be selected by calling the appropriate constructor using one of two object tags: `phased` or `simultaneous`.
To create an animation that is split into phases you can use the following constructor, but you are free to use the default one as well:

```cpp
dyng::interpolator inter(dyng::phased{});
// equivalent to dyng::interpolator inter;
```

To create a simpler animation use this constructor, which sets up the object in order to make a simultaneous transition that simply interpolates between the current state and the next one:

```cpp
dyng::interpolator inter(dyng::simultaneous{});
```