#include <mutex>
#include <condition_variable>

using std::mutex;
using std::condition_variable;
using std::unique_lock;
using std::function;

class barrier
{
  private:
    mutex m_mutex;
    condition_variable m_cv;
    unsigned int m_waiting, m_size, m_generation;
  
  public:
    barrier(const unsigned int size);
    void stop_at();
    void stop_at(function<void()> f);
    void leave();
    void leave(function<void()> f);
};