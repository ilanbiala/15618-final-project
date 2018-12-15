#ifndef _FOLLY_HASHMAP_H
#define _FOLLY_HASHMAP_H

#include <pthread.h>
#include <stdint.h>
#include <vector>
#include <folly/concurrency/ConcurrentHashMap.h>

#include "concurrent_map.h"

class FollyHashMap : public ConcurrentMap
{
private:

folly::ConcurrentHashMap<uint64_t, uint64_t> map;

public:
  FollyHashMap(uint64_t numBuckets) {
    map = folly::ConcurrentHashMap<uint64_t, uint64_t>(numBuckets);
  }

  ~FollyHashMap() {
  }

  uint64_t get(uint64_t key) {
    return map[key];
  }
  void put(uint64_t key, uint64_t value) {
    map.insert_or_assign(key,value);
  }
  bool remove(uint64_t key) {
    map.erase(key);
    return true;
  }
  bool containsKey(uint64_t key) {
    return map.find(key) != map.end();
  }
  uint64_t getSize(void) {
    return map.size();
  }
  void dbg_print(void) {
    
  }
};

#endif /* _FOLLY_HASHMAP_H */
