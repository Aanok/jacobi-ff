#include "barrier.hpp"

barrier::barrier(const unsigned int size)
  : m_mutex(), m_cv(), m_waiting(0), m_size(size), m_generation(0) { }

void barrier::stop_at()
{
  unique_lock<mutex> lock(m_mutex);
  unsigned int stop_gen = m_generation;
  if (++m_waiting == m_size) {
    m_generation++;
    m_waiting = 0;
    m_cv.notify_all();
  } else {
    while(stop_gen == m_generation) m_cv.wait(lock);
  }
  lock.unlock();
}


void barrier::stop_at(function<void()> f)
{
  unique_lock<mutex> lock(m_mutex);
  unsigned int stop_gen = m_generation;
  if (++m_waiting == m_size) {
    f();
    m_generation++;
    m_waiting = 0;
    m_cv.notify_all();
  } else {
    while(stop_gen == m_generation) m_cv.wait(lock);
  }
  lock.unlock();
}


void barrier::leave()
{
  unique_lock<mutex> lock(m_mutex);
  if (--m_size == m_waiting) {
    m_generation++;
    m_waiting = 0;
    m_cv.notify_all();
  }
  lock.unlock();
}


void barrier::leave(function<void()> f)
{
  unique_lock<mutex> lock(m_mutex);
  if (--m_size == m_waiting) {
    f();
    m_generation++;
    m_waiting = 0;
    m_cv.notify_all();
  }
  lock.unlock();
}