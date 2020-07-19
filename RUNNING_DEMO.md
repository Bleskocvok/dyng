# How to compile the demo
You will need the **SDL2** library. If you are using Debian/Ubuntu it can be installed by entering these commands in the terminal:
```bash
sudo apt install libsdl2-dev
```

I recommend using **Clang** for compilation. If **Clang** is installed, it can be used by entering these commands before the next step:
```bash
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++
```

Then to compile execute these commands from the root folder of the project (the same folder where `CMakeLists.txt` is located):  

```bash
mkdir bin
cd bin
cmake .. && make demo -j4
```

Then you can start the demo by entering:

```bash
./demo example_tree_1
```

The animation being displayed can be controlled using keyboard. Hold `left` or `right` arrow key to rewind or to fast forward, press `spacebar` to pause/unpause and `escape` to exit.

## Other executables

The folder `demo/` also contains other targets. These can be compiled similarly by substituting `demo` for the target you want. All the targets can be build at once by entering:

```bash
cmake .. && make -j4
```

These are the phases of `demo` but split into multiple executables:

- `generate` -- generates and outputs an example dynamic graph, uses the same input arguments as `demo` does
- `layout` (or `parallel_layout`) -- takes three (or four) parameters -- (thread count) tolerance, width and height -- and produces a layout from text input it receives
- `draw` -- takes width and height and displays an animation

These can  be chained together using pipes:

```bash
./generate example_tree_1 | ./layout 0.04 1024 640 | ./draw 1280 720
```

## Example executable

The target `example` is built from the file `demo/example.cpp` which creates a simple dynamic graph layout and outputs it. To see an animation of the layout you can enter:  

```bash
./example | ./draw 1280 720
```

## Benchmarking

The folder also contains an executable named `benchmark`, which runs a given number of iterations and produces a table comparing performance for different sizes of an example dynamic graph.

```bash
./benchmark
./benchmark 10
```

Produced table compares three cases: tolerance = 0, tolerance > 0 and parallel implementation with tolerance > 0.

The number of threads to use can also be specified. For example, to use 8 threads you can enter:

```bash
./benchmark 10 8
```

## Taking screenshots

Executable `draw_states` draws individual states as a series of `.bmp` images. It can be used the aforementioned way:

```bash
./example | ./draw_states  1280  720
```

## Examples

As mentioned earlier, the two executables `demo` and `generate` accept the same arguments.
You can use a predefined example, or you can use one of the generators provided.

```bash
./demo example_simple
./demo example_tree_2
./demo example_dense
./demo example_long
./demo gen_grid 8
./demo help # prints a list of accepted arguments
```
