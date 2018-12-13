#ifndef _CONCURRENT_TM_HASHMAP_H
#define _CONCURRENT_TM_HASHMAP_H

#include <stdint.h>
#include <immintrin.h>
#include <vector>

#include "concurrent_map.h"

class ConcurrentHashMapTransactionalMemory : public ConcurrentMap
{
private:
  void hle_lock(int* lock) {
    while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE|__ATOMIC_HLE_ACQUIRE))
    _mm_pause(); /* Abort failed transaction */
  }
  void hle_unlock(int* lock) {
    __atomic_store_n(lock, 0, __ATOMIC_RELEASE|__ATOMIC_HLE_RELEASE);
  }

  class Node
  {
  private:
    uint64_t key;
    uint64_t value;
    Node *next;

  public:
    Node(uint64_t key, uint64_t value, Node *next) {
      this->key = key;
      this->value = value;
      this->next = next;
    }

    ~Node() {}

    uint64_t getKey() {
      return this->key;
    }

    void setKey(uint64_t key) {
      this->key = key;
    }

    uint64_t getValue() {
      return this->value;
    }

    void setValue(uint64_t value) {
      this->value = value;
    }

    Node* getNext() {
      return this->next;
    }

    void setNext(Node *next) {
      this->next = next;
    }
  };

  uint64_t numBuckets;
  uint64_t size;
  std::vector<Node*> buckets;
  std::vector<int> bucketMutexes;
  pthread_mutex_t mux;

public:
  ConcurrentHashMapTransactionalMemory(uint64_t numBuckets);
  ~ConcurrentHashMapTransactionalMemory();

  uint64_t get(uint64_t key);
  void put(uint64_t key, uint64_t value);
  bool remove(uint64_t key);
  bool containsKey(uint64_t key);
  uint64_t getSize(void);
  void dbg_print(void);
};

#endif /* _CONCURRENT_TM_HASHMAP_H */
