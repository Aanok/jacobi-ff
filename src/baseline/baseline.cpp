#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <cmath>
#include <mutex>
#include <condition_variable>
#include "barrier.hpp"

#define ERROR_THRESH 1.0e-1

using namespace std;

typedef vector<float> vect;
typedef vector<vect> matrix;

matrix A; // NB this doesn't affect performance because the computation maps independently to rows
vect sol;
vector<vect> x;
vect b;
unsigned int size, max_iter;

barrier *jacobi_barrier;


inline float rand_float(const int low, const int high)
{
  return rand()/RAND_MAX*(high-low) + low;
}


void init_rand(const int seed = 666)
{
  unsigned int i, j;
  int low = -10;
  int high = 10;
  float sum;
  
  A = matrix(size, vect(size));
  x = matrix(2, vect(size));
  sol = vect(size);
  b = vect(size);
  for (i = 0; i < size; i++) {
    x[0][i] = rand_float(low, high);
    sol[i] = rand_float(low, high);
    sum = 0;
    // enforce weak diagonal predominance
    for (j = 0; j < i; j++) {
      A[i][j] = rand_float(low, high);
      sum += abs(A[i][j]);
    }
    for (j = i+1; j < size; j++) {
      A[i][j] = rand_float(low, high);
      sum += abs(A[i][j]);
    }
    // enforce strong diagonal predominance
    A[i][i] = sum + 1;
  }
  
  // generate b as A*sol so the system has a solution for certain
  for (i = 0; i < size; i++) {
    sum = 0;
    for (j = 0; j < size; j++) {
      sum += A[i][j]*sol[j];
    }
    b[i] = sum;
  }
}



inline float get_error(const unsigned int low, const unsigned int high)
// Error is computed as maximum absolute difference between matching components in the stripe
{
  auto x0_it = x[0].cbegin() + low;
  const auto x0_end = x[0].cbegin() + high;
  auto x1_it = x[1].cbegin() + low;
  float error = 0;
  
  while (x0_it <= x0_end) {
    error = max(error, abs(*x1_it - *x0_it));
    x0_it++;
    x1_it++;
  }
  
  return error;
}


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
    if (get_error(low, high) <= ERROR_THRESH) {
      jacobi_barrier->leave();
      return;
    }
  }
}


int main(int argc, char* argv[])
{
  unsigned int nworkers, stripe, i;
  vector<thread> tt;
  chrono::time_point<chrono::high_resolution_clock> start, end;
  chrono::duration<float> t_proc;
  
  // Manage command line arguments
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " [instance size] [max_iter] [workers]" << endl;
    exit(EXIT_FAILURE);
  }
  size = stoi(argv[1]);
  max_iter = stoi(argv[2]);
  nworkers = stoi(argv[3]);
  
  init_rand(size);
  jacobi_barrier = new barrier(nworkers);
  
  stripe = size / nworkers;
  
  start = chrono::high_resolution_clock::now();
  
  // spawn first nworkers-1 threads
  for (i = 0; i < nworkers-1; i++) tt.push_back(thread(task, i*stripe, (i+1)*stripe -1));
  // last thread works on its stripe and on the remainder up to the end of the instance
  tt.push_back(thread(task,(nworkers-1)*stripe, size-1));
  
  for (i = 0; i < nworkers; i++) tt[i].join();
  
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  cout << t_proc.count() << endl;
  
  return 0;
}
