#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "barrier.hpp"
#include <shared.hpp>

#define ERROR_THRESH 1.0e-3

using namespace std;

matrix A; // NB this doesn't affect performance because the computation maps independently to rows
vect sol;
matrix x;
vect b;
unsigned int size, max_iter;

barrier *jacobi_barrier;


inline void iterate_stripe(const unsigned int low, const unsigned int high, const unsigned int parity)
{
  unsigned int i, j;
  unsigned int antiparity = (parity + 1) % 2;
  float tmp;
  for (i = low; i <= high; i++) {
    // formula: x_i = (1/A_ii)*(b_i - sum(j=0..i-1,i+1..size-1, A_ij*x_j))
    tmp = b[i];
    for (j = 0; j < i; j++) tmp -= A[i][j] * x[antiparity][j];
    for (j = i+1; j < size; j++) tmp -= A[i][j] * x[antiparity][j];
    x[parity][i] = tmp / A[i][i];
  }
}


void task(const unsigned int low, const unsigned int high)
{
  unsigned int t;
  unsigned int iter = 0;
  unsigned int parity = 1;
  
  while (iter <= max_iter) {
    // each iteration goes over the assigned values in the stripe
    iterate_stripe(low, high, parity);
    jacobi_barrier->stop_at();
    parity = (parity + 1) % 2;
    iter++;
    
    // check for convergence
    if (get_error(x[0], x[1], low, high) <= ERROR_THRESH) {
      jacobi_barrier->leave();
      return;
    }
  }
}


inline void jacobi_baseline(const unsigned int nworkers, const unsigned int stripe)
{
  unsigned int i;
  vector<thread> tt;
  
  // spawn first nworkers-1 threads
  for (i = 0; i < nworkers-1; i++) tt.push_back(thread(task, i*stripe, (i+1)*stripe -1));
  // last thread works on its stripe and on the remainder up to the end of the instance
  tt.push_back(thread(task,(nworkers-1)*stripe, size-1));
  
  // wait for join
  for (i = 0; i < nworkers; i++) tt[i].join();
}


int main(int argc, char* argv[])
{
  unsigned int nworkers, stripe;
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
  jacobi_baseline(nworkers, stripe); 
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
