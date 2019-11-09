// Embedded MicroBenchmarks (EMB) - https://github.com/JoelFilho/EMB
// Benchmark example: 
//   - Creating benchmark functions
//   - Using std::chrono
//   - Creating a benchmark reporter class
//   - Running benchmarks

//------------------------------------------------------------------------
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org/>
//------------------------------------------------------------------------

#include <chrono>
#include <emb/emb.hpp>
#include <iostream>

/// The Benchmarker we'll use:
///    - std::chrono's High Resolution clock
///    - Nanoseconds accumulator, but as double, for better representation of statistics
using Benchmarker =
    emb::Benchmarker<std::chrono::high_resolution_clock, std::chrono::duration<double, std::nano>>;

/// Empty loop Benchmark
/// Note the usage of Benchmarker::State as parameter.
/// This function is useful for measuring the overhead of calling the timer functions,
///  which vary between architectures and implementations.
void benchmark_empty(Benchmarker::State& s) {
  for (auto _ : s) {
  }
}

/// Simple for loop benchmark
/// For cases where we don't have the definition of Benchmarker, we can use a template parameter.
template <typename State>
void benchmark_loop(State& s) {
  for (auto _ : s) {
    for (int i = 0; i < 10000; i++)
      // We need to call dontOptimize() to avoid the loop being optimized out.
      emb::dontOptimize(i);
  }
}

/// Same benchmark as the previous one, but using a double accumulator, as an example.
template <typename State>
void benchmark_loop_double(State& s) {
  for (auto _ : s) {
    for (double i = 0; i < 10000.0; i++)
      emb::dontOptimize(i);
  }
}

/// A benchmark reporting class, using std::cout and printing everything.
struct Reporter {
  template <typename Accumulator>
  static void report(const char* name, size_t iterations, Accumulator mean, Accumulator sd) {
    std::cout << name         << '\t' 
              << iterations   << '\t' 
              << mean.count() << "ns\t" 
              << sd.count()   << "ns\n";
  }
};

/// To provide versatility on embedded systems, EMB does not provide a main function, so we can
/// setup our hardware and only run the benchmarks whenever we need.
int main() {
  // We need a local instance of a benchmarker.
  // We may define a default number of iterations to test. Otherwise, 1000 is used.
  Benchmarker benchmarker(100000);

  // To register a benchmark, we can do it in many ways:

  // 1. Use the registerBenchmark member function and give a name to the benchmark.
  //    When we don't set a number of iterations, the default one is used.
  benchmarker.registerBenchmark("benchmark_empty", benchmark_empty);

  // 2. Use the helper macro, to do the same, but automatically naming the benchmark
  EMB_MAKE_BENCHMARK(benchmarker, benchmark_loop);

  // 3. Use any of the methods above, but also specifying the number of iterations for each case.
  EMB_MAKE_BENCHMARK(benchmarker, benchmark_loop_double, 12000);

  // To run the benchmarks, we just need to call runBenchmarks with the desired Reporter class.
  benchmarker.runBenchmarks<Reporter>();
}