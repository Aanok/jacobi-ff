#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "barrier.hpp"
#include <shared.hpp>

using namespace std;


void task(const matrix &A,
          matrix &x,
          const vect &b,
          double *error,
          const long low,
          const long high,
          const long size,
          const long max_iter,
          barrier *jacobi_barrier)
{
  long iter = 0;
  long parity = 1;
  
  while (iter < max_iter && *error > ERROR_THRESH) {
    // each iteration goes over the assigned values in the stripe
    iterate_stripe(A, ref(x[(parity + 1) % 2]), ref(x[parity]), b, low, high, size);
    // sync with barrier; last thread also triggers error update
    jacobi_barrier->stop_at([&] {
      *error = error_sq(ref(x[0]), ref(x[1]));
    });
    // bookkeep
    parity = (parity + 1) % 2;
    iter++;
  }
}


inline void jacobi_baseline(const matrix &A,
                            matrix &x,
                            const vect &b,
                            const long size,
                            const long max_iter,
                            const long nworkers,
                            const long stripe,
                            barrier *jacobi_barrier)
{
  long i;
  double error = ERROR_THRESH + 1.0;
  vector<thread> tt;
  
  // spawn first nworkers-1 threads
  for (i = 0; i < nworkers-1; i++) tt.push_back(thread(task,
                                                       ref(A),
                                                       ref(x),
                                                       ref(b),
                                                       &error,
                                                       i*stripe,
                                                       (i+1)*stripe -1,
                                                       size,
                                                       max_iter,
                                                       jacobi_barrier));
  // last thread works on its stripe and on the remainder up to the end of the instance
  tt.push_back(thread(task,
                      ref(A),
                      ref(x),
                      ref(b),
                      &error,
                      (nworkers-1)*stripe,
                      size-1,
                      size,
                      max_iter,
                      jacobi_barrier));
  
  // wait for join
  for (i = 0; i < nworkers; i++) tt[i].join();
}


int main(int argc, char* argv[])
{
  matrix A, x;
  vect b, sol;
  long nworkers, size, max_iter, stripe;
  barrier *jacobi_barrier;
  chrono::time_point<chrono::high_resolution_clock> start, end;
  chrono::duration<float> t_proc;
  
  // Manage command line arguments
  if (argc < 4) {
    cerr << "Usage: " << argv[0] << " instance_size max_iter workers" << endl;
    exit(EXIT_FAILURE);
  }
  size = stoi(argv[1]);
  max_iter = stoi(argv[2]);
  nworkers = stoi(argv[3]);
  
  // init to random instance
  tie(A, x, sol, b) = init_rand(size, -10, 10);
  jacobi_barrier = new barrier(nworkers);
  
  stripe = size / nworkers;
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_baseline(ref(A), ref(x), ref(b), size, max_iter, nworkers, stripe, jacobi_barrier); 
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
