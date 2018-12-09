#pragma once

// cilkplus
#if defined(CILK)
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <sstream>
#define PAR_GRANULARITY 5000

int num_workers() {return __cilkrts_get_nworkers();}
int worker_id() {return __cilkrts_get_worker_number();}
void set_num_workers(int n) {
  __cilkrts_end_cilk();
  std::stringstream ss; ss << n;
  if (0 != __cilkrts_set_param("nworkers", ss.str().c_str())) {
    std::cerr << "failed to set worker count!" << std::endl;
    std::abort();
  }
}

template <class F>
inline void parallel_for(size_t start, size_t end, F f, size_t granularity) {
  cilk_for(size_t i=start; i<end; i++) {
    f(i);
  }
}

template <typename Lf, typename Rf>
inline void par_do_(Lf left, Rf right) {
    cilk_spawn right();
    left();
    cilk_sync;
}

template <typename Lf, typename Mf, typename Rf >
inline void par_do3_(Lf left, Mf mid, Rf right) {
    cilk_spawn mid();
    cilk_spawn right();
    left();
    cilk_sync;
}

// openmp
#elif defined(OPENMP)
#include <omp.h>
#define PAR_GRANULARITY 200000

int num_workers() { return omp_get_max_threads(); }
int worker_id() { return omp_get_thread_num(); }
void set_num_workers(int n) { omp_set_num_threads(n); }

template <class F>
inline void parallel_for(size_t start, size_t end, F f, size_t granularity) {
  _Pragma("omp parallel for") for(size_t i=start; i<end; i++) {
    f(i);
  }
}

template <typename Lf, typename Rf>
static void par_do_(Lf left, Rf right) {
#pragma omp task
    left();
#pragma omp task
    right();
#pragma omp taskwait
}

template <typename Lf, typename Mf, typename Rf>
static void par_do3_(Lf left, Mf mid, Rf right) {
#pragma omp task
    left();
#pragma omp task
    mid();
#pragma omp task
    right();
#pragma omp taskwait
}

// Guy's scheduler (ABP)
#elif defined(HOMEGROWN)
#include "scheduler.h"
fork_join_scheduler fj;

#define PAR_GRANULARITY 2000

int num_workers() {
  return fj.num_workers();
}

int worker_id() {
  return fj.worker_id();
}

void set_num_workers(int n) {
  fj.set_num_workers(n);
}

template <class F>
inline void parallel_for(size_t start, size_t end, F f, size_t granularity) {
  fj.parfor(start, end, f, granularity);
}

template <typename Lf, typename Rf>
static void par_do_(Lf left, Rf right) {
  return fj.pardo(left, right);
}

template <typename Lf, typename Mf, typename Rf>
static void par_do3_(Lf left, Mf mid, Rf right) {
//  return fj.pardo(left, [&] () { fj.pardo(mid, right); });
  return fj.pardo([&] () { fj.pardo(left, mid); }, right);
}

template <typename Job>
static void parallel_run(Job job) {
  fj.run(job);
}

// c++
#else

int num_workers() { return 1;}
int worker_id() { return 0;}
void set_num_workers(int n) { ; }
#define PAR_GRANULARITY 1000

template <class F>
inline void parallel_for(size_t start, size_t end, F f, size_t granularity) {
  for (size_t i=start; i<end; i++) {
    f(i);
  }
}

template <typename Lf, typename Rf>
static void par_do_(Lf left, Rf right) {
  left(); right();
}

template <typename Lf, typename Mf, typename Rf>
static void par_do3_(Lf left, Mf mid, Rf right) {
  left(); mid(); right();
}
#endif
