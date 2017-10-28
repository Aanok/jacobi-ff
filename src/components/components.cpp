#include <iostream>
#include <vector>
#include <ff/parallel_for.hpp>

#include <shared.hpp>

using namespace std;


/* to be rigorous about it it would warrant testing it out, but:
you could think of doing a parrallelFor over the components and then within each iteration
you could also do a parallelForReduce to compute x[i]; but this would fuck up cache locality,
since accesses to A[i][j]'s happen row-wise for a fixed x[i]. it would likely be slower.
so i'm not doing it. make sure to note it in the report. */


inline void jacobi_components(const matrix &A,
                              matrix &x,
                              const vect &b,
                              const long size,
                              const long max_iter,
                              const long nworkers)
{
  long iter = 0;
  long parity = 1;
  long antiparity = 0;
  long j;
  float tmp;
  ff::ParallelFor pf(nworkers, true, true);
  
#ifdef _MIC
  // passive scheduling is faster with many cores
  pf.disableScheduler();
#endif
  while (iter < max_iter) {
    pf.parallel_for_idx(0, size, 1, 5, [&](const long start, const long stop, const long thid) {
      iterate_stripe(A, ref(x[(parity + 1) % 2]), ref(x[parity]), b, start, stop-1, size);
    }, nworkers);
    // check for convergence
    if (error_sq(ref(x[0]), ref(x[1])) < ERROR_THRESH) return;
    parity = (parity + 1) % 2;
    iter++;
  }
}


int main(int argc, char* argv[])
{
  matrix A, x;
  vect b, sol;
  long nworkers, size, max_iter;
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

  // init random instance
  tie(A, x, b, sol) = init_rand(size, -10, 10);
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_components(ref(A), ref(x), ref(b), size, max_iter, nworkers);
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
