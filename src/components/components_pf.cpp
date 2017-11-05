#include <iostream>
#include <vector>
#include <ff/parallel_for.hpp>

#include <shared.hpp>

using namespace std;


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
  ff::ParallelFor pf(nworkers);
  
#ifdef _MIC
  // passive scheduling is faster with many cores
  pf.disableScheduler();
#endif
  while (iter < max_iter) {
    pf.parallel_for(0, size, 1, 5, [&](const long i) {
      iterate_stripe(cref(A), cref(x[antiparity]), ref(x[parity]), cref(b), i, i, size);
    }, nworkers);
    // check for convergence
    if (error_sq(cref(x[0]), cref(x[1])) < ERROR_THRESH) return;
    antiparity = parity;
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
  tie(A, x, b, sol) = init_rand(size, -10, 10, false);
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_components(cref(A), ref(x), cref(b), size, max_iter, nworkers);
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
