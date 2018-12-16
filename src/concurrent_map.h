#ifndef _CONCURRENT_MAP_H
#define _CONCURRENT_MAP_H

#include <stdbool.h>
#include <stdint.h>

class ConcurrentMap
{
private:

public:
  ConcurrentMap() {}

  uint64_t hash(uint64_t key) {
    uint64_t hashVal = key;

    hashVal = (hashVal ^ (hashVal >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    hashVal = (hashVal ^ (hashVal >> 27)) * UINT64_C(0x94d049bb133111eb);
    hashVal = hashVal ^ (hashVal >> 31);

    return hashVal;
  }

  virtual uint64_t get(uint64_t key) = 0;
  virtual void put(uint64_t key, uint64_t value) = 0;
  virtual bool remove(uint64_t key) = 0;
  virtual bool containsKey(uint64_t key) = 0;
  virtual uint64_t getSize() = 0;
  virtual void dbg_print(void) = 0;
};

#endif /* _CONCURRENT_MAP_H */
