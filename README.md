# Embedded MicroBenchmarks (EMB)

EMB is a lightweight library for benchmarking embedded software, written in C++11 and compatible with GCC compilers. 

**This project is still Work In Progress**, so it may have API changes, incomplete functionality, performance issues and bugs. 
Contact me [via Twitter](https://twitter.com/_JoelFilho) or [open an Issue](https://github.com/JoelFilho/EMB/issues/new) for bugs or feature requests.

## Using the library

EMB is a generic library, built for compatibility with multiple embedded platforms.

As different platforms have different interfaces for timing and I/O, EMB provides multiple customization points, and you need to provide to use it:

1. Import the [`emb/emb.hpp`](include/emb/emb.hpp) file into your project. EMB is header-only, so it's that easy!
2. Provide a reporter class for you system. This class can store the benchmark results, print them to your screen, send via serial port, etc.
   * See [this example](examples/stl_chrono/main.cpp) for details on how to do it, as well as how to instantiate the benchmarks.
3. Define a timer class for the benchmark. The library is compatible with the `std::chrono`'s interface.
   * See [this example](examples/stl_ctime/main.cpp) for a basic implementation
4. If you're not using the STL, set the `EMB_DECLVAL` and `EMB_VECTOR` macros, with compatible interfaces.
   * `EMB_DECLVAL` should have similar functionality to `std::declval`. 
[Example implementation](https://github.com/JoelFilho/JTC/blob/master/include/jtc/templates/declval.hpp).
   * `EMB_VECTOR` should be a class template similar to `std::vector`, with a `push_back` member function and `begin` and `end` iterator functions for range-based `for` loop.

## Copyright / License

Copyright © 2019 Joel P. C. Filho

This software is released under the Boost Software License - Version 1.0. Refer to [the License file](LICENSE.md) for details. 