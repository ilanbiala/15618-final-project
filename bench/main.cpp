#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <pthread.h>

#include <algorithm>
#include <random>
#include <vector>

#include <benchmark/benchmark.h>

#include "concurrent_map.h"
#include "sequential_hashmap.h"
#include "concurrent_hashmap.h"
#include "concurrent_tm_hashmap.h"
#include "folly_hashmap.h"
#include "folly_atomic_hashmap.h"

static void SingleReadForHLEArguments(benchmark::internal::Benchmark* b) {
  for (int numBuckets = 128; numBuckets <= 128; numBuckets *= 2) {
    for (int numKeys = 1; numKeys <= 1; numKeys *= 2) {
      for (int mapType = 1; mapType <= 16; mapType *= 2) {
        for (int percentReads = 0; percentReads <= 100; percentReads += 25) {
          b->Args({numBuckets, numKeys, mapType, percentReads, 0});
        }
      }
    }
  }
}

static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int numKeys = 16; numKeys <= 65536; numKeys *= 16) {
    for (int loadFactorInc20 = 1; loadFactorInc20 <= 5; loadFactorInc20++) {
      for (int percentReads = 0; percentReads <= 100; percentReads += 25) {
        for (int mapType = 1; mapType <= 16; mapType *= 2) {
          double loadFactor = (((double)loadFactorInc20) / 5);
          int numBuckets = numKeys / loadFactor;
          b->Args({numBuckets, numKeys, mapType, percentReads, (long)(loadFactor*100)});
        }
      }
    }
  }
}

enum hashmap_type_t {
  SEQUENTIAL=1,
  CONCURRENT=2,
  CONCURRENT_TM=4,
  FOLLY=8,
  FOLLY_ATOMIC=16
};

class HashMapFixture : public benchmark::Fixture {
public:
  ConcurrentMap *map_ptr;
  uint64_t numBuckets;
  uint64_t numKeys;
  uint64_t mapType;
  uint64_t percentReads;
  bool flag = false;

  // 0th arg is numBuckets, 1st arg is numElems, 2nd arg is type of HashMap, 3rd arg is % reads
  void SetUp(benchmark::State& state) {
      // printf("%d setup start, flag= %d\n", state.thread_index, flag);

    if (state.thread_index == 0) {
      numBuckets = state.range(0);
      numKeys = state.range(1);
      mapType = state.range(2);
      percentReads = state.range(3);


      if (mapType == SEQUENTIAL) {
        map_ptr = new SequentialHashMap(numBuckets);
      } else if (mapType == CONCURRENT) {
        map_ptr = new ConcurrentHashMapBucketLock(numBuckets);
      } else if (mapType == CONCURRENT_TM) {
        map_ptr = new ConcurrentHashMapTransactionalMemory(numBuckets);
      } else if (mapType == FOLLY) {
        map_ptr = new FollyHashMap(numBuckets);
      } else if (mapType == FOLLY_ATOMIC) {
        map_ptr = new FollyAtomicHashMap(numBuckets);
      } else {
        printf("ERROR: invalid map type %ld\n", mapType);
        exit(1);
      }

      for (uint64_t i = 0; i < numKeys; i++) {
        uint64_t k = i;
        uint64_t v = rand();
        map_ptr->put(k, v);
      }
      __sync_synchronize();
      flag = true;

    }
    else {
      while(!flag){_mm_pause();}
    }
      // printf("%d setup end, flag= %d\n", state.thread_index, flag);

  }

  void TearDown(benchmark::State& state) {
    if (state.thread_index == 0) {
      // for (uint64_t i = 0; i < numKeys; i++) {
      //   uint64_t k = i;
      //   uint64_t v = rand();
      //   map_ptr->put(k, v);
      // }
    }
  }
};

// BENCHMARK_DEFINE_F(HashMapFixture, RandomRead)(benchmark::State& state) {
//   std::vector<uint64_t> local_keys(state.range(1));
//   auto rng = std::default_random_engine {};
//   // Seed based on thread index so each thread gets a different generator
//   rng.seed(state.thread_index);

//   // Copy and shuffle thread's access pattern
//   std::iota(std::begin(local_keys), std::end(local_keys), 0);
//   std::shuffle(std::begin(local_keys), std::end(local_keys), rng);

//   // printf("size(local_keys)=%lu\n", local_keys.size());

//   for (auto _ : state) {
//     for (uint64_t k : local_keys) {
//       benchmark::DoNotOptimize(map_ptr->get(k));
//     }
//   }
// }

BENCHMARK_DEFINE_F(HashMapFixture, SingleReadForHLE)(benchmark::State& state) {
  std::vector<uint64_t> local_keys(numKeys);
  // uint64_t chance = 0;
  // auto rng = std::default_random_engine {};
  // Seed based on thread index so each thread gets a different generator
  // rng.seed(state.thread_index);
  // std::uniform_int_distribution<uint64_t> readChance(0, 100);

  // Copy and shuffle thread's access pattern
  std::iota(std::begin(local_keys), std::end(local_keys), 0);
  // std::shuffle(std::begin(local_keys), std::end(local_keys), rng);

  // printf("size(local_keys)=%lu\n", local_keys.size());

  for (auto _ : state) {
    for (uint64_t k : local_keys) {
        benchmark::DoNotOptimize(map_ptr->get(k));
      // } else {
        // map_ptr->put(k, state.thread_index);
      // }
    }

    // state.PauseTiming();
    // // Google Benchmark doesn't auto-synchronize at the end of each iteration
    // pthread_barrier_wait(&barrier);
    // state.ResumeTiming();
  }
}

BENCHMARK_REGISTER_F(HashMapFixture, SingleReadForHLE)
  ->Apply(SingleReadForHLEArguments)
  ->UseRealTime()
  ->Threads(8);

BENCHMARK_REGISTER_F(HashMapFixture, SingleReadForHLE)
  ->Apply(SingleReadForHLEArguments)
  ->UseRealTime()
  ->Threads(64);

BENCHMARK_DEFINE_F(HashMapFixture, RandomReadWrite)(benchmark::State& state) {
  std::vector<uint64_t> local_keys(numKeys);
  uint64_t chance = 0;
  uint64_t numReads = 0;
  uint64_t numOps = 0;
  auto rng = std::default_random_engine {};
  // Seed based on thread index so each thread gets a different generator
  rng.seed(state.thread_index);
  std::uniform_int_distribution<uint64_t> readChance(0, 100);

  // Copy and shuffle thread's access pattern
  std::iota(std::begin(local_keys), std::end(local_keys), 0);
  std::shuffle(std::begin(local_keys), std::end(local_keys), rng);

  // printf("size(map)=%lu\n", map_ptr->getSize());
  assert(map_ptr->getSize() == numKeys);

  for (auto _ : state) {
    for (uint64_t k : local_keys) {
      // Pause timing to get as many ops/second as possible
      state.PauseTiming();
      chance = readChance(rng);
      numOps++;

      if (chance <= percentReads) {
        numReads++;
        state.ResumeTiming();
        benchmark::DoNotOptimize(map_ptr->get(k));
      } else {
        state.ResumeTiming();
        map_ptr->put(k, state.thread_index);
      }
    }

    // state.PauseTiming();
    // // Google Benchmark doesn't auto-synchronize at the end of each iteration
    // pthread_barrier_wait(&barrier);
    // state.ResumeTiming();
  }
  flag = false;
  state.counters["NumReads"] = numReads;
  state.counters["NumOps"] = numOps;
}

BENCHMARK_REGISTER_F(HashMapFixture, RandomReadWrite)
  ->Apply(CustomArguments)
  // ->RangeMultiplier(2)
  // ->Ranges({{128, 128, 1}, {128, 8192, 4}})
  ->UseRealTime()
  ->ThreadRange(4,16);
  // ->ThreadPerCpu();  
  // ->DenseThreadRange(1, 4, 1);
  // ->Unit(benchmark::kMicrosecond)
  // ->Repetitions(3);

BENCHMARK_MAIN();
