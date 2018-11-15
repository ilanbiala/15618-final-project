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
  uint64_t bucketIdx = hash(key) % this->size;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

  for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
    if (curr->getKey() == key) {
      pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
      return curr->getValue();
    }
  }

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
  return false;
}

void ConcurrentHashMapBucketLock::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->size;

  pthread_mutex_lock(&bucketMutexes[bucketIdx]);

  if (buckets[bucketIdx] == NULL) {
    buckets[bucketIdx] = &Node(value, NULL);
  } else {
    Node newItem = Node(value, buckets[bucketIdx]);
    buckets[bucketIdx] = &newItem;
  }

  pthread_mutex_unlock(&bucketMutexes[bucketIdx]);
}

bool ConcurrentHashMapBucketLock::remove(uint64_t key, uint64_t value) {

}

bool ConcurrentHashMapBucketLock::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->size;

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

uint64_t ConcurrentHashMapBucketLock::getSize() {
  return size;
}
