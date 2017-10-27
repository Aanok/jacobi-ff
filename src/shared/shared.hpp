#include <vector>
#include <tuple>
#include <random>
#include <cmath>
#include <iostream>

// universal test parameters
#define ERROR_THRESH 1.0e-4

// custom types
typedef std::vector<double> vect;
typedef std::vector<vect> matrix;

// function signatures
inline double rand_double(const int low, const int high);
std::tuple<matrix, matrix, vect, vect> init_rand(const unsigned int size,
                                                 const unsigned int low,
                                                 const unsigned int high);
void iterate_stripe(const matrix &A,
                    const vect &x_old,
                    vect &x_new,
                    const vect &b,
                    const long low,
                    const long high,
                    const unsigned int size);
double error_comp(const vect &x0, const vect &x1, const unsigned int low, const unsigned int high);
double error_sq(const vect &x0, const vect &x1);
void print(const vect &x);
void print(const matrix &x);