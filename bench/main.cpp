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
          // Static load factor of 0 for the data processing pipeline
          b->Args({numBuckets, numKeys, mapType, percentReads, 0});
        }
      }
    }
  }
}

static void CustomArguments(benchmark::internal::Benchmark* b) {
  for (int numKeys = 1; numKeys <= 65536; numKeys *= 16) {
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
  ConcurrentMap *map_ptr = NULL;
  uint64_t numBuckets;
  uint64_t numKeys;
  uint64_t mapType = 0;
  uint64_t percentReads;
  bool flag = false;

  // 0th arg is numBuckets, 1st arg is numElems, 2nd arg is type of HashMap,
  // 3rd arg is % reads, 4th arg is loadFactor and is just for data processing pipeline
  void SetUp(benchmark::State& state) {
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
  }

  void TearDown(benchmark::State& state) {
    if (state.thread_index == 0) {
      // TODO add proper teardown code to avoid memory leaks.
      // This requires destructors for each implementations.
    }
  }
};

// Old benchmark that does random reads
// Most likely doesn't work without modification
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

BENCHMARK_DEFINE_F(HashMapFixture, SingleReadForHLE)(benchmark::State& state) {
  // Initialize local_keys to have 1 key
  std::vector<uint64_t> local_keys(numKeys);
  std::iota(std::begin(local_keys), std::end(local_keys), 0);

  for (auto _ : state) {
    for (uint64_t k : local_keys) {
      benchmark::DoNotOptimize(map_ptr->get(k));
    }
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
  uint64_t chance = 0;
  auto rng = std::default_random_engine {};
  // Seed based on thread index so each thread gets a different generator
  rng.seed(state.thread_index);
  std::uniform_int_distribution<uint64_t> readChance(0, 100);
  std::uniform_int_distribution<uint64_t> randKey(0, numKeys - 1);

  assert(map_ptr->getSize() == numKeys);

  for (auto _ : state) {
      uint64_t k = randKey(rng);
      chance = readChance(rng);

      if (chance <= percentReads) {
        benchmark::DoNotOptimize(map_ptr->get(k));
      } else {
        map_ptr->put(k, state.thread_index);
      }
  }
  // TODO fix this hack to get threads resynchronized in SetUp()
  flag = false;
}

BENCHMARK_REGISTER_F(HashMapFixture, RandomReadWrite)
  ->Apply(CustomArguments)
  ->UseRealTime()
  ->ThreadRange(1, 64);

BENCHMARK_MAIN();
