# Lock-free, Concurrent, Generic Maps Progress Update (Ilan Biala and Varun Sharma)

URL: [https://ilanbiala.github.io/15618-final-project/](https://ilanbiala.github.io/15618-final-project/)


Original Proposal URL: [https://ilanbiala.github.io/15618-final-project/proposal](https://ilanbiala.github.io/15618-final-project/proposal)

## Progress Update Summary

We have completed the implementation of the concurrent map with a per-bucket locking scheme. We have also created tests that test the correctness of the implementation under single-threaded usage. We have also investigated how we are going to implement the lock-free version of the map. We will implement a hand-over-hand locking scheme, which will improve concurrent performance but introduce more overhead at the same time for any given operation. We have begun to think about the performance benchmarks we would like to write, but we have not definitively decided on specific benchmarks and use cases yet.

### Goals and Deliverables Update

During the poster session, we will have quantitative performance analysis comparing our implementations to each other. As a nice-to-have, we will also compare our implementations to other concurrent `Map` implementations in C++ such as Folly's [AtomicHashMap](https://github.com/facebook/folly/blob/master/folly/AtomicHashMap.h) and [ConcurrentHashMap](https://github.com/facebook/folly/blob/master/folly/concurrency/ConcurrentHashMap.h).

Our main goal is to ensure correctness and avoid race conditions. Secondarily, we will attempt to maximize performance and be as competitive as we can with implementations from large libraries such as Folly and TBB.

### Poster Session Plans

We will show graphs and other results at the poster session, since a demo of our implemented map is not going to be illustrative of its performance.

### Preliminary Results

At this time, we have no preliminary results to demonstrate regarding the performance of our implementations. We have developed tests that focus on functionality, and once we have our atomic implementation finished, we will develop performance benchmarks.

### Current Issues

Currently, we have no major issues. We are still working on ways to test the correctness of our implementations under concurrent usage, but we don't see this as a major issue and proper research will help us understand how to do this. We are also thinking about the performance tests we will use and what those will test and how we will implement those benchmarks.

## Updated Schedule

| Week | Milestone                                                                                                                                | Status |
| ---- | ---------------------------------------------------------------------------------------------------------------------------------------- | ------ |
| 1    | Understand different concurrent `Map` implementations and begin writing tests                                                            |   ✔    |
| 2    | Continue writing tests and begin implementing `Map` with locks                                                                           |   ✔    |
| 3    | Finish implementation of `Map` with locks                                                                                                |   ✔    |
| 4    | Add performance benchmarks and run preliminary benchmarks against current `Map` implementation (Ilan)                                    |        |
| 4    | Implement concurrent tests against current `Map` implementation (Varun)                                                                  |        |
| 5    | Implement and test lock-free `Map` (Ilan, Varun)                                                                                         |        |
| 5.5  | Implement and test transactional memory `Map` (Ilan, Varun)                                                                              |        |
| 6    | Add more performance benchmarks and perform performance optimizations (Ilan, Varun)                                                      |        |
| 6.5  | Evaluate and compare results against other library implementations (Ilan, Varun)                                                         |        |
