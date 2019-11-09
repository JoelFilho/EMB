// Embedded MicroBenchmarks (EMB) - https://github.com/JoelFilho/EMB
// emb.hpp - Benchmark defintions

// Copyright Joel P. C. Filho 2019 - 2019
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef EMB_INCLUDED_EMB_HPP
#define EMB_INCLUDED_EMB_HPP

#include <math.h>
#include <stddef.h>

// Default STL-based templates/structures
#ifndef EMB_NO_STL

// You may set a declval implementation in platforms without STL by setting EMB_DECLVAL
#ifndef EMB_DECLVAL
#include <utility>
#define EMB_DECLVAL std::declval
#endif

// You may set a vector implementation in platforms without STL by setting EMB_VECTOR
#ifndef EMB_VECTOR
#include <vector>
#define EMB_VECTOR std::vector
#endif

#endif  // EMB_NO_STL

/// Always inline attribute, compatible with GCC
#define EMB_ALWAYS_INLINE __attribute__((always_inline))

/// Helper Macro: Register a benchmark with name equal to the function's name
#define EMB_MAKE_BENCHMARK(benchmarker, function, ...) \
  benchmarker.registerBenchmark(#function, function, ##__VA_ARGS__);

/// Embedded MicroBenchmarks namespace
namespace emb {

/// Prevents a variable from being optimized out by the compiler
/// The dontOptimize and clobberMemory functions are based on Google Benchmark's
///  DoNotOptimize and ClobberMemory functions, released under the Apache 2.0 License.
/// Refer to https://github.com/google/benchmark for details on the library and
///  http://www.apache.org/licenses/LICENSE-2.0 for the license.
/// Then, if you find out I can't use them this way, tell me. I have no idea how to do it correctly.
template <typename T>
inline EMB_ALWAYS_INLINE void dontOptimize(T& data) {
  asm volatile("" : "+m,r"(data) : : "memory");
}

template <typename T>
inline EMB_ALWAYS_INLINE void dontOptimize(const T& data) {
  asm volatile("" : : "r,m"(data) : "memory");
}

/// Creates a memory barrier for reads/writes
inline EMB_ALWAYS_INLINE void clobberMemory() {
  asm volatile("" : : : "memory");
}

/// EMB Implementation details.
namespace detail {
/// Determines the time point type for a timer class
template <typename Timer>
using default_time_point_t = decltype(Timer::now());

/// Determines the duration type for a time point type
template <typename TimePoint>
using duration_t = decltype(EMB_DECLVAL<TimePoint>() - EMB_DECLVAL<TimePoint>());

/// Determines the default duration type for a timer class, using the default time point.
template <typename Timer>
using default_duration_t = duration_t<default_time_point_t<Timer>>;

/// Basic multiplication for accumulators
template <typename Accumulator>
inline auto multiply(const Accumulator& a1, const Accumulator& a2) -> decltype(a1 * a2) {
  return a1 * a2;
}

/// Multiplication for std::chrono-like accumulator types, where operator * is not defined,
/// but a .count() member function is provided
template <typename Accumulator, decltype(EMB_DECLVAL<Accumulator>().count())* = nullptr>
inline Accumulator multiply(const Accumulator& a1, const Accumulator& a2) {
  return Accumulator(a1.count() * a2.count());
}

/// Default square root function
template <typename Accumulator>
inline auto sqrt(const Accumulator& a) -> decltype(::sqrt(a)) {
  return ::sqrt(a);
}

/// Square root function for std::chrono-like types
template <typename Accumulator>
inline auto sqrt(const Accumulator& a) -> decltype(::sqrt(a.count())) {
  return ::sqrt(a.count());
}

}  // namespace detail

/// The EMB class responsible for benchmarking
/// \tparam Timer         a timer class with a public static `now()` function
/// \tparam Accumulator   an accumulator type
template <typename Timer, typename Accumulator = detail::default_duration_t<Timer>>
class Benchmarker {
 public:
  // Forward declaration of the State type.
  class State;

  /// Type of benchmarked functions
  using EvaluatorFunction = void (*)(State&);

  /// Default constructor
  /// \param default_iterations Number of iterations to execute benchmarks, where not specified.
  Benchmarker(size_t default_iterations = 1000) : default_iterations_{default_iterations} {}

  /// Register a benchmark, specifying a number of iterations
  void registerBenchmark(const char* name, EvaluatorFunction e, size_t iterations) {
    evaluators.push_back({name, e, iterations});
  }

  /// Register a benchmark, using the default number of iterations.
  void registerBenchmark(const char* name, EvaluatorFunction e) {
    evaluators.push_back({name, e, default_iterations_});
  }

  /// Run all benchmarks
  /// \tparam Reporter a class with a static function
  ///         report(name, iterations, mean, standard_deviation), where
  ///         name is a string type (const char*);
  ///         iterations is an unsigned type (size_t);
  ///         mean and standard_deviation have the type of Accumulator.
  ///         Reporter::report(...) is called after each benchmarked function.
  template <typename Reporter>
  void runBenchmarks();

 private:
  /// Internal struct to store the benchmark functions
  struct Evaluator {
    /// Display name of the benchmark
    const char* name;
    /// Function to be benchmarked
    EvaluatorFunction function;
    /// Number of iterations to be performed
    size_t iterations;
  };

  /// Default number of iterations for this benchmark
  size_t default_iterations_;
  /// Collection of benchmarks to execute
  EMB_VECTOR<Evaluator> evaluators;
};

/// Contains the state of a running benchmark.
/// Non-copyable and non-movable type, intended to be used in a range-for loop.
template <typename Timer, typename Accumulator>
class Benchmarker<Timer, Accumulator>::State {
  // Forward declaration of the State::Iterator class
  class Iterator;

  // Friends for accessing this class's data
  friend Iterator;
  friend Benchmarker;

  /// Time Point type
  using time_point = detail::default_time_point_t<Timer>;

  /// Duration type
  using duration = detail::default_duration_t<Timer>;

 public:
  Iterator begin() noexcept;
  Iterator end() noexcept;

 // Everything except for iterator access
 private:
  State(size_t iterations) : iterations_{iterations} {};
  State(const State&) = delete;
  State(State&&) = delete;

  /// Update the statistics after each loop iteration, using Welford's algorithm
  void update(const duration& d) noexcept {
    iteration_++;
    auto value = Accumulator(d);
    auto delta = value - mean_;
    mean_ += delta / iteration_;
    auto delta2 = value - mean_;
    squared_differences_ += detail::multiply(delta, delta2);
  }

  /// Whether benchmark has finished
  bool done() noexcept { return iteration_ >= iterations_; }

  /// Report an individual benchmark.
  /// See Benchmarker::runBenchmarks for a description on Reporter.
  template <typename Reporter>
  void report(const char* name);

  /// Number of iterations to perform
  const size_t iterations_;
  /// Current iteration
  size_t iteration_{0};
  /// Mean time value in the current iteration
  Accumulator mean_{0};
  /// Sum of the squared mean differences, for calculating variance
  Accumulator squared_differences_{0};
};

/// A basic iterator class for a benchmark
template <typename Timer, typename Accumulator>
class Benchmarker<Timer, Accumulator>::State::Iterator {
  /// RAII helper to measure the time of an iteration
  struct IterationTimer {
    /// Constructs with current time
    IterationTimer(State& s) noexcept : state(s), start{Timer::now()} {}

    /// Destroys by loading current time and updating the State
    ~IterationTimer() noexcept {
      time_point now = Timer::now();
      state.update(now - start);
    }

   private:
    State& state;
    time_point start;
  };

  friend State;

 public:
  // Dereference operator, constructs an IterationTimer for this State
  IterationTimer operator*() noexcept { return {*state}; }

  // Increment operator, ends loop if state.done()
  Iterator& operator++() noexcept {
    if (state->done())
      state = nullptr;
    return *this;
  }

  friend bool operator!=(const Iterator& b1, const Iterator& b2) noexcept {
    return b1.state != b2.state;
  }

 private:
  State* state;
  Iterator(State* s) noexcept : state(s) {}
};

//----------------------------------------------------------------------------------
// Implementations that needed declarations
//----------------------------------------------------------------------------------

template <typename Timer, typename Accumulator>
inline typename Benchmarker<Timer, Accumulator>::State::Iterator
Benchmarker<Timer, Accumulator>::State::begin() noexcept {
  return Iterator{this};
}

template <typename Timer, typename Accumulator>
inline typename Benchmarker<Timer, Accumulator>::State::Iterator
Benchmarker<Timer, Accumulator>::State::end() noexcept {
  return Iterator{nullptr};
}

template <typename Timer, typename Accumulator>
template <typename Reporter>
inline void Benchmarker<Timer, Accumulator>::State::report(const char* name) {
  Reporter::report(name, iterations_, mean_,
      Accumulator(detail::sqrt(squared_differences_ / (iterations_ - 1))));
}

template <typename Timer, typename Accumulator>
template <typename Reporter>
inline void Benchmarker<Timer, Accumulator>::runBenchmarks() {
  for (auto& e : evaluators) {
    State s(e.iterations);
    e.function(s);
    s.template report<Reporter>(e.name);
  }
}

}  // namespace emb

#endif