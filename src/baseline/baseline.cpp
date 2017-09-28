#include <iostream>
#include <vector>
#include <thread>
#include <random>

#define MAX_ITER 100000
#define BATCH_SIZE 5
#define ERROR_THRESH 1.0e-1

using namespace std;

typedef vector<double> row;

vector<row> A; // NB this doesn't affect performance because the computation maps independently to rows
vector<double> x;
vector<double> b;
unsigned int size;


void init_rand(const int seed = 666)
{
  unsigned int i, j;
  default_random_engine generator(seed);
  uniform_int_distribution<> distribution(-100, 100);
  
  A.reserve(size);
  x.reserve(size);
  b.reserve(size);
  for (i = 0; i < size; i++) {
    x.push_back(distribution(generator));
    b.push_back(distribution(generator));
    A[i].reserve(size);
    for (j = 0; j < size; j++) A[i].push_back(distribution(generator));
  }
}

inline double get_error(vector<double>::const_iterator old_start,
                    const vector<double>::const_iterator old_end,
                    vector<double>::const_iterator new_start,
                    const vector<double>::const_iterator new_end)
{
  double error = 0;
  
  for(; old_start <= old_end; old_start++) {
    error += *new_start - *old_start;
    new_start++;
  }
  
  return error;
}

void task(const unsigned int low, const unsigned int high)
{
  unsigned int t, i, j;
  unsigned int iter = 1;
  double sum;
  vector<double> last;
  last.reserve(high - low + 1);
  
  while (iter < MAX_ITER) {
    // store current solution slice
    for (t = 0; t < high - low + 1; t++) last[t] = x[low + t];
    
    // do a batch of iterations
    for (t = 0; t < BATCH_SIZE; t++) {
      // each iteration goes over the assigned values in the stripe
      for (i = low; i <= high; i++) {
        // formula: x_i = (1/A_jj)*(b_i - sum(j=1..i-1,i+1..size, A_ij*x_j))
        sum = b[i];
        for (j = 0; j < i; j++) sum -= A[i][j] * x[j];
        for (j = i+1; j < size; j++) sum -= A[i][j] * x[j];
        x[i] = 1/A[i][i] * sum;
      }
      iter++;
    }
    
    // do one convergence check per batch
    if (get_error(last.cbegin(), last.cend(), x.cbegin()+low, x.cbegin()+high) <= ERROR_THRESH) {
      cerr << "slice (" << low << "," << high << ") converged after " << iter << " iterations" << endl;
      return;
    }
  }

  
  cerr << "slice (" << low << "," << high << ") did not converge" << endl;
}


int main(int argc, char* argv[])
{
  unsigned int nworkers, stripe, i;
  vector<thread> tt;
  
  // Manage command line arguments
  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " [instance size] [workers]" << endl;
    exit(EXIT_FAILURE);
  }
  size = stoi(argv[1]);
  nworkers = stoi(argv[2]);
  
  init_rand(size);
  
  stripe = size / nworkers;
  cerr << "stripe: " << stripe << endl;
  
  // spawn first nworkers-1 threads
  for (i = 0; i < nworkers-1; i++) tt.push_back(thread(task, i*stripe, (i+1)*stripe-1));
  // last thread works on its stripe and on the reminder up to the end of the instance
  tt.push_back(thread(task,(nworkers-1)*stripe, size-1));
  
  for (i = 0; i < nworkers; i++) tt[i].join();
  
  return 0;
}