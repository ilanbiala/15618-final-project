#include <stdlib.h>
#include <stdexcept>

#include "concurrent_map.h"
#include "sequential_hashmap.h"

SequentialHashMap::SequentialHashMap(uint64_t numBuckets)
{
  this->numBuckets = numBuckets;
  this->size = 0;
  buckets = std::vector<Node*>(numBuckets);
  pthread_mutex_init(&mux, NULL);
}

SequentialHashMap::~SequentialHashMap()
{
}

uint64_t SequentialHashMap::get(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&mux);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      uint64_t val = curr->getValue();
      pthread_mutex_unlock(&mux);
      return val;
    }
  }

  pthread_mutex_unlock(&mux);
  this->dbg_print();
  printf("ERROR: Not found key=%lu\n", key);
  throw std::out_of_range ("key not found");
}

void SequentialHashMap::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&mux);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      curr->setValue(value);
      pthread_mutex_unlock(&mux);
      return;
    }
  }

  Node *newItem = new SequentialHashMap::Node(key, value, buckets[bucketIdx]);
  buckets[bucketIdx] = newItem;
  this->size++;

  pthread_mutex_unlock(&mux);
}

bool SequentialHashMap::remove(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&mux);

  Node *last = buckets[bucketIdx];
  if (last == nullptr) {
    pthread_mutex_unlock(&mux);
    return false;
  }
  if (last->getKey() == key) {
    buckets[bucketIdx] = last->getNext();
    delete last;
    this->size--;
    pthread_mutex_unlock(&mux);
    return true;
  }

  for (Node *curr = last->getNext(); curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      last->setNext(curr->getNext());
      delete curr;
      this->size--;
      pthread_mutex_unlock(&mux);

      return true;
    }
    last = curr;
  }

  pthread_mutex_unlock(&mux);
  return false;
}

bool SequentialHashMap::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&mux);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      pthread_mutex_unlock(&mux);
      return true;
    }
  }

  pthread_mutex_unlock(&mux);
  return false;
}

uint64_t SequentialHashMap::getSize(void) {
  return size;
}

void SequentialHashMap::dbg_print(void) {
  printf("Printing out current state of HashMap (%lu elements, %lu buckets):\n", this->size, this->numBuckets);
  printf("{\n");
  pthread_mutex_lock(&mux);

  for (unsigned long i = 0; i < buckets.size(); i++) {
      Node* bucket = buckets[i];

    if (bucket != NULL) {
      printf("\tBUCKET%lu: ",i);
      for (Node *curr = bucket; curr != NULL; curr = curr->getNext()) {
        printf("(%lu, %lu);\t", curr->getKey(), curr->getValue());
      }
      printf("\n");
    } else {
      printf("\tBUCKET%lu: EMPTY\n", i);
    }
  }

  pthread_mutex_unlock(&mux);

  printf("}\n");
}
