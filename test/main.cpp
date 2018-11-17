#include <cstdio>
#include <stdint.h>

#include "concurrent_hashmap.h"

int main(int argc, char const *argv[])
{
  uint64_t numBuckets = 10;
  if (argc == 2) {
    numBuckets = atoi(argv[1]);
  }

  printf("%d, %s\n", argc, argv[0]);
  printf("numBuckets=%llu\n", numBuckets);

  ConcurrentHashMapBucketLock map = ConcurrentHashMapBucketLock(numBuckets);

  map.dbg_print();

  for (uint64_t i = 0; i < 50; i++) {
    printf("Inserting (%llu, %llu)\n", i, i + 50);
    map.put(i, i + 50);
    printf("Inserted (%llu, %llu)\n", i, i + 50);
    map.dbg_print();
  }

  for (uint64_t i = 0; i < 50; i++) {
    printf("Getting (%llu, value)\n", i);
    uint64_t value = map.get(i);
    printf("Got (%llu, %llu)\n", i, value);
  }

  return 0;
}
