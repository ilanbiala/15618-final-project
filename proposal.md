# Lock-free, Concurrent, Generic Maps Proposal (Ilan Biala and Varun Sharma)

Main Page URL: [https://ilanbiala.github.io/15618-final-project/](https://ilanbiala.github.io/15618-final-project/)
Proposal URL: [https://ilanbiala.github.io/15618-final-project/proposal](https://ilanbiala.github.io/15618-final-project/proposal)

## Summary

We will implement lock-free, concurrent, generic maps using both atomic instructions and transactional memory. We will evaluate both implementations to see which implementation performs better under certain conditions.

## Background

The typical methods supported on a `Map` that we will implement are:

```C++
Map createMap(size)
elem get(key)
void put(key, value)
elem remove(key)
bool containsKey(key)
long size()
bool isEmpty()
```

Concurrent `Map`s can be implemented using locks, atomic instructions, and transactional memory to ensure that only one mutating operation on an element will occur at any time and the state of the `Map` is not corrupted. Multiple threads can operate on distinct, unrelated elements simultaneously, and coarse-grain locks reduce the parallelism by implying false data dependencies. Atomic instructions and transactional memory allow the processor to speculatively execute the operation and then perform conflict detection to determine whether the changes need to be rolled back. We will take advantage of this by wrapping the atomic instructions and transactional memory in retry loops to ensure the operation is performed.

## The Challenge

Implementing concurrent data structures is inherently challenging because of the objective of avoiding race conditions while still allowing multiple processes to make progress. Lock-free programming also poses challenges because of the difficulty in reasoning about the correctness of the implementation. Additionally, atomic instructions do not operate on arbitrary pieces of data, so lock-free data structures typically limit the data type to primitives. Transactional memory avoids this problem by allowing the programmer to specify the start and end of an atomic section of code, with arbitrary instructions within the atomic section. With `Map`s in particular, there could be many processes reading or writing to different elements or even the same element. The number of possible interleavings of operations on the `Map` is countless, and maintaining correctness while also maintaining performance is difficult. There are many typical usage patterns of `Map`s depending on the workload. Most of these usage patterns often involve a high communication-to-computation ratio, otherwise known as low arithmetic intensity. Lock-free programming can reduce contention and strain on the interconnect while still maintaining good performance if implemented correctly. The main constraint with concurrent data structures is maintaining partial ordering, that is to say that within one process two operations on the data structure should not be reordered.

## Resources

We will be implementing the `Map` from scratch. We will use OpenMP, ISPC, and/or pthreads to test and benchmark our implementations. We plan on running our benchmarks on one of CMU's clusters to benchmark our implementation on a larger scale.

## Goals And Deliverables

During the poster session, we will have quantitative performance analysis comparing our implementations to each other and to other concurrent `Map` implementations in C++ such as Folly's [AtomicHashMap](https://github.com/facebook/folly/blob/master/folly/AtomicHashMap.h) and [ConcurrentHashMap](https://github.com/facebook/folly/blob/master/folly/concurrency/ConcurrentHashMap.h).

Our main goal is to ensure correctness and avoid race conditions. Secondarily, we will attempt to maximize performance and be competitive with implementations from large libraries such as Folly and TBB.

### Plan to Achieve

* Implement lock-free, concurrent `Map` using atomic instructions
* Ensure that there are no race conditions and that the `Map` is correct
* Benchmark implementation against existing open-source library implementations and compare performance results

### Hope to Achieve

* Extend implementation to use transactional memory to support arbitrary data types
* Be competitive with existing open-source library implementations in terms of performance and memory consumption

## Platform Choice

We have chosen to implement our `Map` in C++ to maintain consistency with the rest of the projects in this class and to take advantage of the well-developed benchmarking libraries available. Evaluating our implementation on a cluster will allow to us to see how our implementation scales with larger systems that are more likely to have many processes operating on the same data structure.

## Schedule

| Week | Milestone                                                                                                                                |
| ---- | ---------------------------------------------------------------------------------------------------------------------------------------- |
| 1    | Understand different concurrent `Map` implementations and begin writing tests                                                            |
| 2    | Continue writing tests and begin implementing `Map` with locks                                                                           |
| 3    | Finish implementation of `Map` with locks and start implementation of lock-free `Map` using atomic instructions                          |
| 4    | Continue implementation of lock-free `Map` with simple test cases passing                                                                |
| 5    | Finish implementation of lock-free `Map` with atomic instructions and start implementation of lock-free `Map` using transactional memory |
| 6    | Finish implementation of lock-free `Map` using transactional memory, perform performance optimizations, and benchmark performance        |
