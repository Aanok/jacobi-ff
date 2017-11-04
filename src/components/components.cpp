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

struct jacobi_task {
  jacobi_task(const long parity, const long low, const long high)
  : m_parity(parity), m_low(low), m_high(high) { }
  
  const long m_parity, m_low, m_high;
};


struct jacobi_node: ff_node_t<jacobi_task > {
  jacobi_node(const matrix &A, matrix &x, const vect &b, const long size)
    : m_A(cref(A)), m_x(ref(x)), m_b(cref(b)), m_size(size) { }
  
  jacobi_task* svc(jacobi_task *task) {
    iterate_stripe(cref(m_A), m_x[(task->m_parity + 1) % 2], m_x[task->m_parity], cref(m_b), task->m_low, task->m_high, m_size);
    return task;
  }
  
  const matrix &m_A;
  matrix &m_x;
  const vect &m_b;
  const long m_size;
};


struct jacobi_collector: ff_node_t<jacobi_task > {
  jacobi_collector(const matrix &x, const long max_iter, const long size, const long grain)
    : m_x(cref(x)),
      m_max_iter(max_iter),
      m_iter(0),
      m_collected(0),
      m_tasks(size / grain + (size % grain != 0)) { }
   
  jacobi_task* svc(jacobi_task *task) {
    if (++m_collected >= m_tasks) {
      // workers are done for the current iteration: time for a convergence check
      m_collected = 0;
      if (++m_iter >= m_max_iter || error_sq(cref(m_x[0]), cref(m_x[1])) < ERROR_THRESH) {
        // done: kill everything
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
  const long m_max_iter, m_tasks;
  long m_iter, m_collected;
};


struct jacobi_emitter: ff_node_t<jacobi_task > {
  jacobi_emitter(const long grain, const long size)
    : m_grain(grain), m_size(size), m_parity(1) { }
  
  jacobi_task* svc(jacobi_task *task) {
    long i;
    
    // prevent memory leaks from wrap_around
    if (task != nullptr) delete task;
    
    // generate bulk of tasks
    for (i = 0; i < m_size/m_grain; i++) ff_send_out(new jacobi_task(m_parity, i*m_grain, (i+1)*m_grain -1));
    // take care of remainder, if any
    if (m_size % m_grain != 0) ff_send_out(new jacobi_task(m_parity, (m_size/m_grain)*m_grain, m_size-1));
    
    m_parity = (m_parity + 1) % 2;
    return GO_ON;
  }
  
  const long m_grain, m_size;
  long m_parity;
};

inline void jacobi_components(const matrix &A,
                              matrix &x,
                              const vect &b,
                              const long size,
                              const long max_iter,
                              const long nworkers)
{
  long grain = size/1000;
  vector<unique_ptr<ff_node> > workers;
  
  for (int i = 0; i < nworkers; i++)
    workers.push_back(unique_ptr<jacobi_node>(new jacobi_node(cref(A), ref(x), cref(b), size)));
  
  ff_Farm<jacobi_task> jacobi_farm(move(workers),
                                   unique_ptr<jacobi_emitter>(new jacobi_emitter(grain, size)),
                                   unique_ptr<jacobi_collector>(new jacobi_collector(cref(x), max_iter, size, grain)));
  
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
