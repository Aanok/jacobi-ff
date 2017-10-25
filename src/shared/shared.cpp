#include "shared.hpp"

inline float rand_float(const int low, const int high)
{
  return std::rand()/RAND_MAX*(high-low) + low;
}


std::tuple<matrix, matrix, vect, vect> init_rand(const unsigned int size,
                                                 const unsigned int low,
                                                 const unsigned int high)
{
  unsigned int i, j;
  float sum;
  matrix A(size, vect(size));
  matrix x(2, vect(size));
  vect sol(size);
  vect b(size);
  
  for (i = 0; i < size; i++) {
    x[0][i] = rand_float(low, high);
    sol[i] = rand_float(low, high);
    sum = 0;
    // setup for weak diagonal predominance
    for (j = 0; j < i; j++) {
      A[i][j] = rand_float(low, high);
      sum += std::abs(A[i][j]);
    }
    for (j = i+1; j < size; j++) {
      A[i][j] = rand_float(low, high);
      sum += std::abs(A[i][j]);
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
  
  return std::make_tuple(A, x, sol, b);
}


float get_error(const vect &x0, const vect &x1, const unsigned int low, const unsigned int high)
// Error is computed as maximum absolute difference between matching components in the stripe
{
  auto x0_it = x0.cbegin() + low;
  const auto x0_end = x0.cbegin() + high;
  auto x1_it = x1.cbegin() + low;
  float error = 0;
  
  while (x0_it <= x0_end) {
    error = std::max(error, std::abs(*x1_it - *x0_it));
    x0_it++;
    x1_it++;
  }
  
  return error;
}