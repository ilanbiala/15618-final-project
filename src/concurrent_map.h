#ifndef _CONCURRENT_MAP_H
#define _CONCURRENT_MAP_H

#include <stdbool.h>
#include <stdint.h>

class ConcurrentMap
{
private:

public:
  ConcurrentMap() {}
  virtual ~ConcurrentMap() = 0;

  virtual void* get(void *key) = 0;
  virtual void put(void *key, void *value) = 0;
  virtual bool remove(void *key, void *value) = 0;
  virtual bool containsKey(void *key) = 0;
  virtual uint64_t size() = 0;
};

#endif /* _CONCURRENT_MAP_H */
