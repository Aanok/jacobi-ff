#include <iostream>
#include <vector>
#include <memory>
#include <ff/farm.hpp>

#include <shared.hpp>

using namespace std;
using ff::ff_node_t;
using ff::ff_node;
using ff::ff_Farm;

/* to be rigorous about it it would warrant testing it out, but:
you could think of doing a parrallelFor over the components and then within each iteration
you could also do a parallelForReduce to compute x[i]; but this would fuck up cache locality,
since accesses to A[i][j]'s happen row-wise for a fixed x[i]. it would likely be slower.
so i'm not doing it. make sure to note it in the report. */


struct jacobi_node: ff_node_t<pair<long,long> > {
  jacobi_node(const matrix &A, matrix &x, const vect &b, const long size)
    : m_A(cref(A)), m_x(ref(x)), m_b(cref(b)), m_size(size), m_parity(1) { }
  
  pair<long,long>* svc(pair<long,long> *task) {
    iterate_stripe(cref(m_A), cref(m_x[(m_parity + 1) % 2]), ref(m_x[m_parity]), cref(m_b), task->first, task->second, m_size);
    m_parity = (m_parity + 1) % 2;
    return task;
  }
  
  const matrix &m_A;
  matrix &m_x;
  const vect &m_b;
  const long m_size;
  long m_parity;
};


struct jacobi_collector: ff_node_t<pair<long,long> > {
  jacobi_collector(const matrix &x, const long max_iter, const long nworkers)
    : m_x(cref(x)), m_max_iter(max_iter), m_iter(0), m_nworkers(nworkers), m_collected(0) { }
   
  pair<long,long>* svc(pair<long,long> *task) {
    if (++m_collected >= m_nworkers - 1) {
      // workers are done for the current iteration: time for a convergence check
      m_collected = 0;
      if (++m_iter >= m_max_iter || error_sq(cref(m_x[0]), cref(m_x[1])) < ERROR_THRESH) {
        // done: kill everything
        cerr << m_iter - 1 << endl;
        delete task;
        return EOS;
      }
      // more work to do: propagate bogus task
      return task;
    }
    // too early for convergence check, just consume task
    delete task;
    return GO_ON;
  }
  
  const matrix &m_x;
  const long m_max_iter, m_nworkers;
  long m_iter, m_collected;
};


struct jacobi_emitter: ff_node_t<pair<long,long> > {
  jacobi_emitter(const long nworkers, const long size)
    : m_nworkers(nworkers), m_size(size) { }
  
  pair<long,long>* svc(pair<long,long> *task) {
    long i;
    long stripe = m_size / m_nworkers;
    
    for (i = 0; i < m_nworkers - 1; i++) ff_send_out(new pair<long,long>(i*stripe,(i+1)*stripe -1));
    ff_send_out(new pair<long,long>((m_nworkers-1)*stripe,m_size-1));
    return GO_ON;
  }
  
  const long m_nworkers, m_size;
};

inline void jacobi_components(const matrix &A,
                              matrix &x,
                              const vect &b,
                              const long size,
                              const long max_iter,
                              const long nworkers)
{
  vector<unique_ptr<ff_node> > workers;
  for (int i = 0; i < nworkers; i++)
    workers.push_back(unique_ptr<jacobi_node>(new jacobi_node(cref(A), ref(x), cref(b), size)));
  
  ff_Farm<pair<long,long> > jacobi_farm(move(workers),
                                        unique_ptr<jacobi_emitter>(new jacobi_emitter(nworkers, size)),
                                        unique_ptr<jacobi_collector>(new jacobi_collector(cref(x), max_iter, nworkers)));
  
  jacobi_farm.wrap_around();
  jacobi_farm.run_and_wait_end();
}


int main(int argc, char* argv[])
{
  matrix A, x;
  vect b, sol;
  long nworkers, size, max_iter;
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

  // init random instance
  tie(A, x, b, sol) = init_rand(size, -10, 10, false);
  
  // time and run algorithm
  start = chrono::high_resolution_clock::now();
  jacobi_components(cref(A), ref(x), cref(b), size, max_iter, nworkers);
  end = chrono::high_resolution_clock::now();
  t_proc = end - start;
  
  cout << t_proc.count() << endl;
  
  return 0;
}
