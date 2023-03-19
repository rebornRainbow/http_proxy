/**
 * File: thread-pool.h
 * -------------------
 * This class defines the ThreadPool class, which accepts a collection
 * of thunks (which are zero-argument functions that don't return a value)
 * and schedules them in a FIFO manner to be executed by a constant number
 * of child threads that exist solely to invoke previously scheduled thunks.
 */

#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>     // for size_t
#include <functional>  // for the function template used in the schedule signature
#include <thread>      // for thread
#include <vector>      // for vector
#include <atomic>      // for atomic
#include <queue>       // for tasks
#include <mutex>       // 这是锁

#include "semaphore.h"



class ThreadPool {
 public:

/**
 * Constructs a ThreadPool configured to spawn up to the specified
 * number of threads.
 */
  ThreadPool(size_t numThreads);

/**
 * Schedules the provided thunk (which is something that can
 * be invoked as a zero-argument function without a return value)
 * to be executed by one of the ThreadPool's threads as soon as
 * all previously scheduled thunks have been handled.
 */
  void schedule(const std::function<void(void)>& thunk);

/**
 * Blocks and waits until all previously scheduled thunks
 * have been executed in full.
 */
  void wait();

/**
 * Waits for all previously scheduled thunks to execute, and then
 * properly brings down the ThreadPool and any resources tapped
 * over the course of its lifetime.
 */
  ~ThreadPool();
  
 private:
  std::thread dt;                // dispatcher thread handle
  std::vector<std::thread> wts;  // worker thread handles

  struct workerTask{//这是每个工人得到自己工作的一个工具，表示线程的位置
    semaphore hasTask;
    std::mutex task_m;
    std::function<void(void)> task;//这里我打算用来装一个re
  };

  std::vector<workerTask> worker_task;

  struct Worker_and_schedule
  {
    semaphore freeworkers;
    std::mutex add_task;
    std::queue<std::function<void(void)> >worktasks;
    std::mutex add_workers_m;
    std::queue<int> workers;
  } wo_a_sh;
               // 表示可用工人的数量
  std::mutex tasks_m;
  std::vector<std::function<void(void)> >tasks;
  semaphore addtask;

  std::mutex tasks_i_m;
  size_t tasks_i;
  size_t numThreads;

  //用来判断所有工人是否结束
  std::mutex finish_tasks_m;
  size_t finish_tasks = 0;
  std::condition_variable_any finish_tasks_s;
  struct ISEND{
    std::mutex end_m;
    bool end = false;
  } terminate;


/**
 * ThreadPools are the type of thing that shouldn't be cloneable, since it's
 * not clear what it means to clone a ThreadPool (should copies of all outstanding
 * functions to be executed be copied?).
 *
 * In order to prevent cloning, we remove the copy constructor and the
 * assignment operator.  By doing so, the compiler will ensure we never clone
 * a ThreadPool.
 */
  ThreadPool(const ThreadPool& original) = delete;
  ThreadPool& operator=(const ThreadPool& rhs) = delete;
};

#endif
