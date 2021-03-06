#ifndef _SEQUENTIAL_HASHMAP_H
#define _SEQUENTIAL_HASHMAP_H

#include <pthread.h>
#include <stdint.h>
#include <vector>

#include "concurrent_map.h"

class SequentialHashMap : public ConcurrentMap
{
private:

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
  pthread_mutex_t mux;

public:
  SequentialHashMap(uint64_t numBuckets);
  ~SequentialHashMap();

  uint64_t get(uint64_t key);
  void put(uint64_t key, uint64_t value);
  bool remove(uint64_t key);
  bool containsKey(uint64_t key);
  uint64_t getSize(void);
  void dbg_print(void);
};

#endif /* _SEQUENTIAL_HASHMAP_H */
