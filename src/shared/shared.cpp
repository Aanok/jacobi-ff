#include "shared.hpp"

inline double rand_double(const int low, const int high)
{
  return double(std::rand())/RAND_MAX *(high-low) + low;
}


std::tuple<matrix, matrix, vect, vect> init_rand(const unsigned int size,
                                                 const unsigned int low,
                                                 const unsigned int high)
{
  unsigned int i, j;
  double sum;
  matrix A(size, vect(size));
  matrix x(2, vect(size));
  vect sol(size);
  vect b(size);
  
  for (i = 0; i < size; i++) {
    x[0][i] = rand_double(low, high);
    sol[i] = rand_double(low, high);
    sum = 0;
    // setup for weak diagonal predominance
    for (j = 0; j < i; j++) {
      A[i][j] = rand_double(low, high);
      sum += std::abs(A[i][j]);
    }
    for (j = i+1; j < size; j++) {
      A[i][j] = rand_double(low, high);
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


void iterate_stripe(const matrix &A,
                    const vect &x_old,
                    vect &x_new,
                    const vect &b,
                    const long low,
                    const long high,
                    const unsigned int size)
{
  long i, j;
  double tmp;
  for (i = low; i <= high; i++) {
    // formula: x_i = (1/A_ii)*(b_i - sum(j=0..i-1,i+1..size-1, A_ij*x_j))
    tmp = b[i];
    for (j = 0; j < i; j++) tmp -= A[i][j] * x_old[j];
    for (j = i+1; j < size; j++) tmp -= A[i][j] * x_old[j];
    x_new[i] = tmp / A[i][i];
  }
}


double error_comp(const vect &x0, const vect &x1, const unsigned int low, const unsigned int high)
// Error is computed as maximum absolute difference between matching components in the stripe
{
  auto x0_it = x0.cbegin() + low;
  const auto x0_end = x0.cbegin() + high;
  auto x1_it = x1.cbegin() + low;
  double error = 0;
  
  while (x0_it <= x0_end) {
    error = std::max(error, std::abs(*x1_it - *x0_it));
    x0_it++;
    x1_it++;
  }
  
  return error;
}


double error_sq(const vect &x0, const vect &x1)
// Two-norm of difference between vectors
{
  auto x0_it = x0.cbegin();
  auto x1_it = x1.cbegin();
  double error = 0;
  
  while (x0_it <= x0.cend()) {
    error += (*x0_it - *x1_it) * (*x0_it - *x1_it);
    x0_it++;
    x1_it++;
  }
  
  return std::sqrt(error);
}


void print(const vect &x)
{
  for (auto it = x.cbegin(); it < x.cend(); it++)
    std::cout << *it << " ";
  std::cout << std::endl;  
}


void print(const matrix &x)
{
  for (auto it = x.cbegin(); it < x.cend(); it++) print(x);
}