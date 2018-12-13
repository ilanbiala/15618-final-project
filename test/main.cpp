#include <cstdio>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "sequential_hashmap.h"
#include "concurrent_hashmap.h"
#include "concurrent_tm_hashmap.h"

// #define DEBUG // uncomment this line to enable debugging

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#endif

#define TEST_SIZE 10000

void test_distinct_put_get(ConcurrentMap& map) {
  // map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < TEST_SIZE; i++) {
      dbg_printf("Inserting (%lu, %lu)\n", i, i + TEST_SIZE);
      map.put(i, i + TEST_SIZE);
      assert(i + TEST_SIZE == map.get(i));
      dbg_printf("Inserted (%lu, %lu)\n", i, i + TEST_SIZE);
      assert(map.getSize() == i + 1);
    }

    // map.dbg_print();

    for (uint64_t i = 0; i < TEST_SIZE; i++) {
      assert(map.getSize() == TEST_SIZE - i);
      dbg_printf("Getting (%lu, value)\n", i);
      assert(map.containsKey(i));
      uint64_t value = map.get(i);
      assert(value == i + TEST_SIZE);
      dbg_printf("Got (%lu, %lu)\n", i, value);
      assert(map.remove(i) == true);
      dbg_printf("Removed (%lu, %lu)\n", i, value);
      assert(!map.containsKey(i));
    }
  }

  printf("%s passed!\n", __FUNCTION__);
}

void test_same_put_get(ConcurrentMap& map) {

  // map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < TEST_SIZE; i++) {
      dbg_printf("Inserting (%lu, %lu)\n", 0UL, i);
      map.put(0, i);
      dbg_printf("Inserted (%lu, %lu)\n", 0UL, i);
      assert(map.getSize() == 1);
    }

    // map.dbg_print();

    for (uint64_t i = 0; i < TEST_SIZE; i++) {
      if (i == 0) {
        assert(map.getSize() == 1);
        dbg_printf("Getting (%lu, value)\n", i);
        assert(map.containsKey(i));
        uint64_t value = map.get(i);
        assert(value == TEST_SIZE-1);
        dbg_printf("Got (%lu, %lu)\n", i, value);
        assert(map.remove(i) == true);
        dbg_printf("Removed (%lu, %lu)\n", i, value);
      }

      assert(map.getSize() == 0);
      assert(!map.containsKey(i));
    }
  }

  printf("%s passed!\n", __FUNCTION__);
}

int main(int argc, char const *argv[])
{
  uint64_t numBuckets = 10;
  if (argc == 2) {
    numBuckets = atoi(argv[1]);
  }

  dbg_printf("%d, %s\n", argc, argv[0]);
  dbg_printf("numBuckets=%lu\n", numBuckets);

  SequentialHashMap seq_map1 = SequentialHashMap(numBuckets);
  SequentialHashMap seq_map2 = SequentialHashMap(numBuckets);
  ConcurrentHashMapBucketLock map1 = ConcurrentHashMapBucketLock(numBuckets);
  ConcurrentHashMapBucketLock map2 = ConcurrentHashMapBucketLock(numBuckets);
  ConcurrentHashMapTransactionalMemory map3 = ConcurrentHashMapTransactionalMemory(numBuckets);
  ConcurrentHashMapTransactionalMemory map4 = ConcurrentHashMapTransactionalMemory(numBuckets);

  test_distinct_put_get(seq_map1);
  test_same_put_get(seq_map2);

  test_distinct_put_get(map1);
  test_same_put_get(map2);

  test_distinct_put_get(map3);
  test_same_put_get(map4);


  return 0;
}
