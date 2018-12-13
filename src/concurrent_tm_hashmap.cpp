#include <stdlib.h>
#include <stdexcept>

#include "concurrent_map.h"
#include "concurrent_tm_hashmap.h"

ConcurrentHashMapTransactionalMemory::ConcurrentHashMapTransactionalMemory(uint64_t numBuckets)
{
  this->numBuckets = numBuckets;
  this->size = 0;
  buckets = std::vector<Node*>(numBuckets);
  bucketMutexes = std::vector<int>(numBuckets);
  for (size_t i = 0; i < numBuckets; i++) {
    bucketMutexes[i] = 0;
  }
  pthread_mutex_init(&mux, NULL);
}

ConcurrentHashMapTransactionalMemory::~ConcurrentHashMapTransactionalMemory()
{
}

uint64_t ConcurrentHashMapTransactionalMemory::get(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  hle_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      uint64_t val = curr->getValue();
      hle_unlock(&bucketMutexes[bucketIdx]);
      return val;
    }
  }

  hle_unlock(&bucketMutexes[bucketIdx]);
  throw std::out_of_range ("key not found");
}

void ConcurrentHashMapTransactionalMemory::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  hle_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      curr->setValue(value);
      hle_unlock(&bucketMutexes[bucketIdx]);
      return;
    }
  }

  Node *newItem = new ConcurrentHashMapTransactionalMemory::Node(key, value, buckets[bucketIdx]);
  buckets[bucketIdx] = newItem;
  hle_unlock(&bucketMutexes[bucketIdx]);
  pthread_mutex_lock(&mux);
  this->size++;
  pthread_mutex_unlock(&mux);
}

bool ConcurrentHashMapTransactionalMemory::remove(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  hle_lock(&bucketMutexes[bucketIdx]);

  Node *last = buckets[bucketIdx];
  if (last == nullptr) {
    hle_unlock(&bucketMutexes[bucketIdx]);
    return false;
  }
  if (last->getKey() == key) {
    buckets[bucketIdx] = last->getNext();
    hle_unlock(&bucketMutexes[bucketIdx]);
    delete last;
    pthread_mutex_lock(&mux);
    this->size--;
    pthread_mutex_unlock(&mux);

    return true;
  }

  for (Node *curr = last->getNext(); curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      last->setNext(curr->getNext());
      hle_unlock(&bucketMutexes[bucketIdx]);
      delete curr;
      pthread_mutex_lock(&mux);
      this->size--;
      pthread_mutex_unlock(&mux);
      return true;
    }
    last = curr;
  }

  hle_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

bool ConcurrentHashMapTransactionalMemory::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  hle_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      hle_unlock(&bucketMutexes[bucketIdx]);
      return true;
    }
  }

  hle_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

uint64_t ConcurrentHashMapTransactionalMemory::getSize(void) {
  return size;
}

void ConcurrentHashMapTransactionalMemory::dbg_print(void) {
  printf("Printing out current state of HashMap (%lu elements, %lu buckets):\n", this->size, this->numBuckets);
  printf("{\n");

  for (auto const& bucket: buckets) {
    if (bucket != NULL) {
      printf("\tBUCKET: ");
      for (Node *curr = bucket; curr != NULL; curr = curr->getNext()) {
        printf("(%lu, %lu);\t", curr->getKey(), curr->getValue());
      }
      printf("\n");
    } else {
      printf("\tBUCKET: EMPTY\n");
    }
  }

  printf("}\n");
}
