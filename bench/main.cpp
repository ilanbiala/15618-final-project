#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <pthread.h>

#include <algorithm>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

// #include "../benchmark/include/benchmark/benchmark.h"

#include "concurrent_map.h"
#include "sequential_hashmap.h"
#include "concurrent_hashmap.h"
#include "concurrent_tm_hashmap.h"

static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int i = 128; i <= 128; i *= 2) {
    for (int j = 128; j <= 16384; j *= 2) {
      for (int map_type = 1; map_type <= 4; map_type *= 2) {
        b->Args({i, j, map_type});
      }
    }
  }
}

enum hashmap_type_t {
  SEQUENTIAL=1,
  CONCURRENT=2,
  CONCURRENT_TM=4
};

class HashMapFixture : public benchmark::Fixture {
public:
  ConcurrentMap *map_ptr;

  // 0th arg is numBuckets, 1st arg is numElems, 2nd arg is type of HashMap
  void SetUp(benchmark::State& state) {
    if (state.range(2) == SEQUENTIAL) {
      map_ptr = new SequentialHashMap(state.range(0));
    } else if (state.range(2) == CONCURRENT) {
    // if (state.range(2) == CONCURRENT) {
      map_ptr = new ConcurrentHashMapBucketLock(state.range(0));
    // } else {
    } else if (state.range(2) == CONCURRENT_TM) {
      map_ptr = new ConcurrentHashMapTransactionalMemory(state.range(0));
    // }
    } else {
      printf("ERROR: invalid map type %ld\n", state.range(2));
      exit(1);
    }

    for (int64_t i = 0; i < state.range(1); i++) {
      uint64_t k = i;
      uint64_t v = rand();
      map_ptr->put(k, v);
    }
  }

  void TearDown(benchmark::State& state) {
    (void) state;
    // for (int64_t i = 0; i < state.range(1); i++) {
    //   uint64_t k = i;
    //   map_ptr->remove(k);
    // }
  }
};

BENCHMARK_DEFINE_F(HashMapFixture, RandomRead)(benchmark::State& state) {
  std::vector<uint64_t> local_keys(state.range(1));
  auto rng = std::default_random_engine {};
  // Seed based on thread index so each thread gets a different generator
  rng.seed(state.thread_index);

  // Copy and shuffle thread's access pattern
  std::iota(std::begin(local_keys), std::end(local_keys), 0);
  std::shuffle(std::begin(local_keys), std::end(local_keys), rng);

  for (auto _ : state) {
    for (uint64_t k : local_keys) {
      benchmark::DoNotOptimize(map_ptr->get(k));
    }
  }
}

BENCHMARK_REGISTER_F(HashMapFixture, RandomRead)
  ->Apply(CustomArguments)
  // ->RangeMultiplier(2)
  // ->Ranges({{128, 128, 1}, {128, 8192, 4}})
  ->UseRealTime()
  ->DenseThreadRange(1, 4, 1);

// static void BM_HashMapRandomRead(benchmark::State& state) {
//   // ConcurrentHashMapBucketLock map = ConcurrentHashMapBucketLock(state.range(0));
//   // std::vector<std::vector<uint64_t>> keys(state.threads);
//   std::vector<uint64_t> keys;
//   auto rng = std::default_random_engine {};

//   // Setup code
//   if (state.thread_index == 0) {

//     pthread_mutex_init(&mutex, NULL);
//     map = ConcurrentHashMapBucketLock(state.range(0));
//     for (int64_t i = 0; i < state.range(1); i++) {
//       uint64_t k = rand();
//       uint64_t v = rand();
//       map.put(k, v);
//       keys.push_back(k);
//       // for (int i = 0; i < state.threads; i++) {
//         // keys[i].push_back(k);
//       // }
//     }
//   }

//   printf("mutex_addr=%p\n", (void*) &mutex);

//   // Shuffle every thread's access pattern
//   // for (int i = 0; i < state.threads; i++) {
//     // std::shuffle(std::begin(keys[i]), std::end(keys[i]), rng);
//   // }

//   for (auto _ : state) {
//     pthread_mutex_lock(&mutex);
//     pthread_mutex_unlock(&mutex);
//     // for (uint64_t k : keys) {
//     // for (uint64_t k : keys[state.thread_index]) {
//       // benchmark::DoNotOptimize(map.get(k));
//     // }
//   }

//   // for (int i = 0; i < state.threads; i++) {
//   //   keys[i].clear();
//   // }
// }
// // Register the function as a benchmark
// // numBuckets is 1st arg, numElems is 2nd
// BENCHMARK(BM_HashMapRandomRead)
//   ->RangeMultiplier(2)
//   ->Ranges({{1, 128}, {128, 16384}})
//   ->UseRealTime()
//   // ->Unit(benchmark::kMicrosecond)
//   ->ThreadRange(1, 2);
//   // ->ThreadPerCpu()
//   // ->Repetitions(3);

BENCHMARK_MAIN();
