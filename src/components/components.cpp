#include <iostream>
#include <vector>
#include <ff/parallel_for.hpp>

#include <shared.hpp>

#define ERROR_THRESH 1.0e-3

using namespace std;


/* to be rigorous about it it would warrant testing it out, but:
you could think of doing a parrallelFor over the components and then within each iteration
you could also do a parallelForReduce to compute x[i]; but this would fuck up cache locality,
since accesses to A[i][j]'s happen row-wise for a fixed x[i]. it would likely be slower.
so i'm not doing it. make sure to note it in the report. */


matrix A; // NB this doesn't affect performance because the computation maps independently to rows
vect sol;
matrix x;
vect b;
unsigned int size, max_iter;


inline void jacobi_components(const unsigned int nworkers)
{
  unsigned int iter = 0;
  unsigned int parity = 1;
  unsigned int antiparity = 0;
  long j;
  float tmp;
  ff::ParallelFor pr(nworkers, false);
  
#ifdef _MIC
  // passive scheduling should be faster on manycore architectures
  pr.disableScheduler();
#endif
  while (iter <= max_iter) {
    pr.parallel_for_static(0, size, 1, 0, [&](const long i) {
      // formula: x_i = (1/A_ii)*(b_i - sum(j=0..i-1,i+1..size-1, A_ij*x_j))
      tmp = b[i];
      for (j = 0; j < i; j++) tmp -= A[i][j] * x[antiparity][j];
      for (j = i+1; j < size; j++) tmp -= A[i][j] * x[antiparity][j];
      x[parity][i] = tmp / A[i][i];
    }, nworkers);
    // check for convergence
    if (get_error(x[0], x[1], 0, size-1) <= ERROR_THRESH) return;
    antiparity = parity;
    parity = (parity + 1) % 2;
    iter++;
  }
}


int main(int argc, char* argv[])
{
  unsigned int nworkers;
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
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_components(nworkers);
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
