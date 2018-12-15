#ifndef _FOLLY_ATOMIC_HASHMAP_H
#define _FOLLY_ATOMIC_HASHMAP_H

#include <pthread.h>
#include <stdint.h>
#include <vector>
#include <folly/AtomicHashMap.h>

#include "concurrent_map.h"

class FollyAtomicHashMap : public ConcurrentMap
{
private:

folly::AtomicHashMap<uint64_t, uint64_t>* map;

public:
  FollyAtomicHashMap(uint64_t numBuckets) {
    map = new folly::AtomicHashMap<uint64_t, uint64_t>(numBuckets);
  }

  ~FollyAtomicHashMap() {
  }

  uint64_t get(uint64_t key) {
    auto ret = map->find(key);
    if (ret == map->end()) {
      throw std::out_of_range ("key not found");
    } 
    return ret->second;
  }

  void put(uint64_t key, uint64_t value) {
    auto ret = map->insert(key,value);
    if (!ret.second) {
      ret.first->second = value;
    }
  }
  bool remove(uint64_t key) {
    map->erase(key);
    return true;
  }
  bool containsKey(uint64_t key) {
    return map->find(key) != map->end();
  }
  uint64_t getSize(void) {
    return map->size();
  }
  void dbg_print(void) {
    
  }
};

#endif /* _FOLLY_ATOMIC_HASHMAP_H */
