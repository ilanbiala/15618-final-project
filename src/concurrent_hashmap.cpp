#include <stdlib.h>
#include <stdexcept>

#include "concurrent_map.h"
#include "concurrent_hashmap.h"

ConcurrentHashMapBucketLock::ConcurrentHashMapBucketLock(uint64_t numBuckets)
{
  this->numBuckets = numBuckets;
  this->size = 0;
  buckets = std::vector<Node*>(numBuckets);
  bucketMutexes = std::vector<pthread_rwlock_t>(numBuckets);
  for (size_t i = 0; i < numBuckets; i++) {
    pthread_rwlock_init(&bucketMutexes[i], NULL);
  }
}

ConcurrentHashMapBucketLock::~ConcurrentHashMapBucketLock()
{
}

uint64_t ConcurrentHashMapBucketLock::get(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_rwlock_rdlock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      uint64_t val = curr->getValue();
      pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
      return val;
    }
  }

  pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
  printf("ERROR: Not found key=%lu\n", key);
  throw std::out_of_range ("key not found");
}

void ConcurrentHashMapBucketLock::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_rwlock_wrlock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      curr->setValue(value);
      pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
      return;
    }
  }

  Node *newItem = new ConcurrentHashMapBucketLock::Node(key, value, buckets[bucketIdx]);
  buckets[bucketIdx] = newItem;
  pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);

  this->size++;
}

bool ConcurrentHashMapBucketLock::remove(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_rwlock_wrlock(&bucketMutexes[bucketIdx]);

  Node *last = buckets[bucketIdx];
  if (last == nullptr) {
    pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
    return false;
  }
  if (last->getKey() == key) {
    buckets[bucketIdx] = last->getNext();
    pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
    delete last;
    this->size--;
    return true;
  }

  for (Node *curr = last->getNext(); curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      last->setNext(curr->getNext());
      pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
      delete curr;
      this->size--;
      return true;
    }
    last = curr;
  }

  pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

bool ConcurrentHashMapBucketLock::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_rwlock_rdlock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
      return true;
    }
  }

  pthread_rwlock_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

uint64_t ConcurrentHashMapBucketLock::getSize(void) {
  return size;
}

void ConcurrentHashMapBucketLock::dbg_print(void) {
  printf("Printing out current state of HashMap (%lu elements, %lu buckets):\n", (uint64_t)this->size, this->numBuckets);
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
