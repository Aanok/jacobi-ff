#include <vector>
#include <tuple>
#include <random>
#include <cmath>
#include <iostream>

// custom types
typedef std::vector<double> vect;
typedef std::vector<vect> matrix;

// function signatures
inline double rand_double(const int low, const int high);
std::tuple<matrix, matrix, vect, vect> init_rand(const unsigned int size,
                                                 const unsigned int low,
                                                 const unsigned int high);
double error_comp(const vect &x0, const vect &x1, const unsigned int low, const unsigned int high);
double error_sq(const vect &x0, const vect &x1);
void print(const vect &x);
void print(const matrix &x);