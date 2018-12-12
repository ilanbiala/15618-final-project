#include <cstdio>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "concurrent_hashmap.h"
#include "concurrent_tm_hashmap.h"


void test_distinct_put_get(ConcurrentMap& map) {
  map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < 50; i++) {
      printf("Inserting (%lu, %lu)\n", i, i + 50);
      map.put(i, i + 50);
      assert(i + 50 == map.get(i));
      printf("Inserted (%lu, %lu)\n", i, i + 50);
      assert(map.getSize() == i + 1);
    }

    map.dbg_print();

    for (uint64_t i = 0; i < 50; i++) {
      assert(map.getSize() == 50 - i);
      printf("Getting (%lu, value)\n", i);
      assert(map.containsKey(i));
      uint64_t value = map.get(i);
      assert(value == i + 50);
      printf("Got (%lu, %lu)\n", i, value);
      assert(map.remove(i) == true);
      printf("Removed (%lu, %lu)\n", i, value);
      assert(!map.containsKey(i));
    }
  }

  printf("%s passed!\n", __FUNCTION__);
}

void test_same_put_get(ConcurrentMap& map) {

  map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < 50; i++) {
      printf("Inserting (%lu, %lu)\n", 0UL, i);
      map.put(0, i);
      printf("Inserted (%lu, %lu)\n", 0UL, i);
      assert(map.getSize() == 1);
    }

    map.dbg_print();

    for (uint64_t i = 0; i < 50; i++) {
      if (i == 0) {
        assert(map.getSize() == 1);
        printf("Getting (%lu, value)\n", i);
        assert(map.containsKey(i));
        uint64_t value = map.get(i);
        assert(value == 49);
        printf("Got (%lu, %lu)\n", i, value);
        assert(map.remove(i) == true);
        printf("Removed (%lu, %lu)\n", i, value);
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

  printf("%d, %s\n", argc, argv[0]);
  printf("numBuckets=%lu\n", numBuckets);

  ConcurrentHashMapBucketLock map1 = ConcurrentHashMapBucketLock(numBuckets);
  ConcurrentHashMapBucketLock map2 = ConcurrentHashMapBucketLock(numBuckets);
  ConcurrentHashMapTransactionalMemory map3 = ConcurrentHashMapTransactionalMemory(numBuckets);
  ConcurrentHashMapTransactionalMemory map4 = ConcurrentHashMapTransactionalMemory(numBuckets);


  // test_distinct_put_get(map1);
  // test_same_put_get(map2);

  test_distinct_put_get(map3);
  test_same_put_get(map4);


  return 0;
}
