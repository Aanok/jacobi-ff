#include <vector>
#include <tuple>
#include <random>
#include <cmath>

// custom types
typedef std::vector<float> vect;
typedef std::vector<vect> matrix;

// function signatures
inline float rand_float(const int low, const int high);
std::tuple<matrix, matrix, vect, vect> init_rand(const unsigned int size,
                                                 const unsigned int low,
                                                 const unsigned int high);
float get_error(const vect &x0, const vect &x1, const unsigned int low, const unsigned int high);