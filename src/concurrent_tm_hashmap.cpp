#include <stdlib.h>
#include <stdexcept>
#include <immintrin.h>

#include "concurrent_map.h"
#include "concurrent_tm_hashmap.h"

// uint64_t hash(uint64_t key) {
//   uint64_t hashVal = key;

//   hashVal = (hashVal ^ (hashVal >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
//   hashVal = (hashVal ^ (hashVal >> 27)) * UINT64_C(0x94d049bb133111eb);
//   hashVal = hashVal ^ (hashVal >> 31);

//   return hashVal;
// }

ConcurrentHashMapTransactionalMemory::ConcurrentHashMapTransactionalMemory(uint64_t numBuckets)
{
  this->numBuckets = numBuckets;
  this->size = 0;
  buckets = std::vector<Node*>(numBuckets);
}

ConcurrentHashMapTransactionalMemory::~ConcurrentHashMapTransactionalMemory()
{
}

uint64_t ConcurrentHashMapTransactionalMemory::get(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  unsigned status = _XABORT_EXPLICIT;

  for(int i = 0; i < 10; i++) {
    status = _xbegin ();
    if (status == _XBEGIN_STARTED || !(status & _XABORT_RETRY))
      break;
  }
  if (status == _XBEGIN_STARTED) {
    for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
      if (curr->getKey() == key) {
        uint64_t val = curr->getValue();
        _xend();
        return val;
      }
    }

    _xend();
    throw std::out_of_range ("key not found");
  }
  else {
    printf("GET TRANSACTION FAILED: %u\n",status);
    exit(-1);
  }
}

void ConcurrentHashMapTransactionalMemory::put(uint64_t key, uint64_t value) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  unsigned status = _XABORT_EXPLICIT;

  for(int i = 0; i < 10; i++) {
    status = _xbegin ();
    if (status == _XBEGIN_STARTED || !(status & _XABORT_RETRY))
      break;
  }
  if (status == _XBEGIN_STARTED) {
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
        _xend();
        return;
      }
    }

    Node *newItem = new ConcurrentHashMapTransactionalMemory::Node(key, value, buckets[bucketIdx]);
    buckets[bucketIdx] = newItem;
    this->size++;

    _xend();
  }
  else {
    printf("PUT TRANSACTION FAILED: %u\n",status);
    exit(-1);
  }
}

bool ConcurrentHashMapTransactionalMemory::remove(uint64_t key/*, uint64_t value*/) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  unsigned status = _XABORT_EXPLICIT;

  for(int i = 0; i < 10; i++) {
    status = _xbegin ();
    if (status == _XBEGIN_STARTED || !(status & _XABORT_RETRY))
      break;
  }
  if (status == _XBEGIN_STARTED) {

    Node *last = buckets[bucketIdx];
    if (last == nullptr) {
      _xend();
      return false;
    }
    if (last->getKey() == key) {
      buckets[bucketIdx] = last->getNext();
      delete last;
      this->size--;
      _xend();
      return true;
    }

    for (Node *curr = last->getNext(); curr != NULL; curr = curr->getNext()) {
      if (curr->getKey() == key) {
        last->setNext(curr->getNext());
        delete curr;
        this->size--;
        _xend();
        return true;
      }
      last = curr;
    }

    _xend();
    return false;
  }
  else {
    printf("REMOVE TRANSACTION FAILED: %u\n",status);
    exit(-1);
  }
}

bool ConcurrentHashMapTransactionalMemory::containsKey(uint64_t key) {
  uint64_t bucketIdx = hash(key) % this->numBuckets;

  unsigned status = _XABORT_EXPLICIT;

  for(int i = 0; i < 10; i++) {
    status = _xbegin ();
    if (status == _XBEGIN_STARTED || !(status & _XABORT_RETRY))
      break;
  }
  if (status == _XBEGIN_STARTED) {

    for (Node *curr = buckets[bucketIdx]; curr != NULL; curr = curr->getNext()) {
      if (curr->getKey() == key) {
        _xend();
        return true;
      }
    }

    _xend();
    return false;
  }
    else {
    printf("CONTAINSKEY TRANSACTION FAILED: %u\n",status);
    exit(-1);
  }
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
