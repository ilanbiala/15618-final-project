#include <cstdio>
#include <stdint.h>

#include "concurrent_hashmap.h"

void test_distinct_put_get(uint64_t numBuckets) {
  ConcurrentHashMapBucketLock map = ConcurrentHashMapBucketLock(numBuckets);

  map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < 50; i++) {
      printf("Inserting (%llu, %llu)\n", i, i + 50);
      map.put(i, i + 50);
      assert(i + 50 == map.get(i));
      printf("Inserted (%llu, %llu)\n", i, i + 50);
      assert(map.getSize() == i + 1);
    }

    map.dbg_print();

    for (uint64_t i = 0; i < 50; i++) {
      assert(map.getSize() == 50 - i);
      printf("Getting (%llu, value)\n", i);
      assert(map.containsKey(i));
      uint64_t value = map.get(i);
      assert(value == i + 50);
      printf("Got (%llu, %llu)\n", i, value);
      assert(map.remove(i) == true);
      printf("Removed (%llu, %llu)\n", i, value);
      assert(!map.containsKey(i));
    }
  }

  printf("%s passed!\n", __FUNCTION__);
}

void test_same_put_get(uint64_t numBuckets) {
  ConcurrentHashMapBucketLock map = ConcurrentHashMapBucketLock(numBuckets);

  map.dbg_print();

  for (int iter = 0; iter < 2; iter++) {
    for (uint64_t i = 0; i < 50; i++) {
      printf("Inserting (%llu, %llu)\n", 0ULL, i);
      map.put(0ULL, i);
      printf("Inserted (%llu, %llu)\n", 0ULL, i);
      assert(map.getSize() == 1);
    }

    map.dbg_print();

    for (uint64_t i = 0; i < 50; i++) {
      if (i == 0) {
        assert(map.getSize() == 1);
        printf("Getting (%llu, value)\n", i);
        assert(map.containsKey(i));
        uint64_t value = map.get(i);
        assert(value == 49ULL);
        printf("Got (%llu, %llu)\n", i, value);
        assert(map.remove(i) == true);
        printf("Removed (%llu, %llu)\n", i, value);
      }

      assert(map.getSize() == 0);
      assert(!map.containsKey(i));
    }
  }

  printf("%s passed!", __FUNCTION__);
}

int main(int argc, char const *argv[])
{
  uint64_t numBuckets = 10;
  if (argc == 2) {
    numBuckets = atoi(argv[1]);
  }

  printf("%d, %s\n", argc, argv[0]);
  printf("numBuckets=%llu\n", numBuckets);

  test_distinct_put_get(numBuckets);
  test_same_put_get(numBuckets);

  return 0;
}
