#include <stdlib.h>

#include "concurrent_map.h"
#include "concurrent_hashmap.h"

uint64_t hash(uint64_t key) {
  uint64_t hashVal = key;

  hashVal = (hashVal ^ (hashVal >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
  hashVal = (hashVal ^ (hashVal >> 27)) * UINT64_C(0x94d049bb133111eb);
  hashVal = hashVal ^ (hashVal >> 31);

  return hashVal;
}

ConcurrentHashMapBucketLock::ConcurrentHashMapBucketLock(uint64_t numBuckets)
{
  this->numBuckets = numBuckets;
  this->size = 0;
  buckets = std::vector<Node*>(numBuckets);
  bucketMutexes = std::vector<pthread_mutex_t>(numBuckets);
}

ConcurrentHashMapBucketLock::~ConcurrentHashMapBucketLock()
{
}

uint64_t ConcurrentHashMapBucketLock::get(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      uint64_t val = curr->getValue();
      pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
      return val;
    }
  }

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
  throw std::out_of_range ("key not found");
}

void ConcurrentHashMapBucketLock::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

// need to check existance of key and overwrite if exists
// we should change api to return old value if overwritten

  // if (buckets[bucketIdx] == NULL) {
  //   buckets[bucketIdx] = &Node(value, NULL);
  // } else {
  //   Node newItem = Node(value, buckets[bucketIdx]);
  //   buckets[bucketIdx] = &newItem;
  // }
  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      curr->setValue(value);
      pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
      return;
    }
  }

  Node *newItem = new ConcurrentHashMapBucketLock::Node(key, value, buckets[bucketIdx]);
  buckets[bucketIdx] = newItem;
  this->size++;

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
}

bool ConcurrentHashMapBucketLock::remove(uint64_t key/*, uint64_t value*/) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

  Node *last = buckets[bucketIdx];
  if (last == nullptr) {
    pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
    return false;
  }
  if (last->getKey() == key) {
    buckets[bucketIdx] = last->getNext();
    pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
    delete last;
    this->size--;
    return true;
  }

  for (Node *curr = last->getNext(); curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      last->setNext(curr->getNext());
      pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
      delete curr;
      this->size--;
      return true;
    }
    last = curr;
  }

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

bool ConcurrentHashMapBucketLock::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
      return true;
    }
  }

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

uint64_t ConcurrentHashMapBucketLock::getSize(void) {
  return size;
}

void ConcurrentHashMapBucketLock::dbg_print(void) {
  printf("Printing out current state of HashMap (%llu elements, %llu buckets):\n", this->size, this->numBuckets);
  printf("{\n");

  for (auto const& bucket: buckets) {
    if (bucket != NULL) {
      printf("\tBUCKET: ");
      for (Node *curr = bucket; curr != NULL; curr = curr->getNext()) {
        printf("(%llu, %llu);\t", curr->getKey(), curr->getValue());
      }
      printf("\n");
    } else {
      printf("\tBUCKET: EMPTY\n");
    }
  }

  printf("}\n");
}
