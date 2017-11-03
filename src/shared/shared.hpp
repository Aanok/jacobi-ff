#include <vector>
#include <tuple>
#include <random>
#include <cmath>
#include <ctime>
#include <iostream>

// universal test parameters
#define ERROR_THRESH 1.0e-10

// custom types
typedef std::vector<double> vect;
typedef std::vector<vect> matrix;

// function signatures
inline double rand_double(const int low, const int high);
std::tuple<matrix, matrix, vect, vect> init_rand(const long size,
                                                 const long low,
                                                 const long high,
                                                 const bool time_seed = true);
void iterate_stripe(const matrix &A,
                    const vect &x_old,
                    vect &x_new,
                    const vect &b,
                    const long low,
                    const long high,
                    const long size);
double error_comp(const vect &x0, const vect &x1, const long low, const long high);
double error_sq(const vect &x0, const vect &x1);
void print(const vect &x);
void print(const matrix &x);