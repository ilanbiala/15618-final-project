#ifndef _CONCURRENT_MAP_H
#define _CONCURRENT_MAP_H

#include <stdbool.h>
#include <stdint.h>

class ConcurrentMap
{
private:

public:
  ConcurrentMap() {}
  // virtual ~ConcurrentMap() = 0 {};

  virtual uint64_t get(uint64_t key) = 0;
  virtual void put(uint64_t key, uint64_t value) = 0;
  virtual bool remove(uint64_t key/*, uint64_t value*/) = 0;
  virtual bool containsKey(uint64_t key) = 0;
  virtual uint64_t getSize() = 0;
};

#endif /* _CONCURRENT_MAP_H */
