#include <iostream>
#include <vector>
#include <chrono>
#include <functional>

#include <shared.hpp>

using namespace std;




void jacobi_sequential(const matrix &A,
                       matrix &x,
                      const vect &b,
                      const long size,
                      const long max_iter)
{
  long iter = 0;
  long parity = 1;
  long antiparity = 0;
  double error = ERROR_THRESH + 1.0;
  
  while (iter < max_iter && error > ERROR_THRESH) {
    // here the whole matrix is a monolithic stripe
    iterate_stripe(cref(A), cref(x[antiparity]), ref(x[parity]), cref(b), 0, size-1, size);
    error = error_sq(cref(x[0]), cref(x[1]));
    // bookkeep
    antiparity = parity;
    parity = (parity + 1) % 2;
    iter++;
  }
}


int main(int argc, char* argv[])
{
  matrix A, x;
  vect b, sol;
  long size, max_iter;
  chrono::time_point<chrono::high_resolution_clock> start, end;
  chrono::duration<float> t_proc;
  
  // Manage command line arguments
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " instance_size max_iter" << endl;
    exit(EXIT_FAILURE);
  }
  size = stoi(argv[1]);
  max_iter = stoi(argv[2]);
  
  // init to random instance
  tie(A, x, sol, b) = init_rand(size, -10, 10, false);
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_sequential(cref(A), ref(x), cref(b), size, max_iter); 
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}