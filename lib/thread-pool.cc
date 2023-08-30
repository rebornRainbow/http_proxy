/**
 * File: thread-pool.cc
 * --------------------
 * 这是我自己完成的线程池的c++实现
 */

#include <iostream>
#include <unistd.h>
#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) :
wts(numThreads),
worker_task(numThreads),//工人获取任务的工具
tasks_i(0),
numThreads(numThreads)
{
  // std::cout << "创建" << std::endl;
  for(size_t i = 0;i < numThreads; ++i)
  {
    //将所有的工人启动。
    wts[i] = thread([this](int i){//这是工人线程
      wo_a_sh.add_workers_m.lock();
      wo_a_sh.workers.push(i);
      wo_a_sh.add_workers_m.unlock();
      wo_a_sh.freeworkers.signal();//先通知我有空了
      // std::cout << "进入循环了" << std::endl;
      //不断循环
      while(true)
      {
        worker_task[i].hasTask.wait();

        terminate.end_m.lock();
        if(terminate.end) 
        {
          // cout << i << "退出" << endl;
          terminate.end_m.unlock();
          break;
        }
        terminate.end_m.unlock();
        //执行任务
        // cout << i << "开始执行" << endl;
        //cout << worker_task[i].task.size() << endl;
        worker_task[i].task_m.lock();
        auto mytask  = worker_task[i].task;
        worker_task[i].task_m.unlock();
        mytask();
        //通知调度我有空了
        wo_a_sh.add_workers_m.lock();
        wo_a_sh.workers.push(i);
        wo_a_sh.add_workers_m.unlock();
        wo_a_sh.freeworkers.signal();
        finish_tasks_m.lock();
        ++finish_tasks;
        finish_tasks_m.unlock();
        finish_tasks_s.notify_all();
      }
      
    },i);
  }
  dt = thread([this](){//调度器
  //不断循环

  while(true)
  {
    //从任务队列中获取一个任务
    addtask.wait();
    terminate.end_m.lock();
    if(terminate.end) 
    {
      // cout << "调度器退出" << endl;
      terminate.end_m.unlock();
      break;
    }
    terminate.end_m.unlock();
    // cout << "收到一个任务" << endl;
    tasks_m.lock();
    auto task = tasks[tasks_i++];
    tasks_m.unlock();
    //等待一个可以执行的工人，//这里是并发的，因为可能不止一个工人完成。
    wo_a_sh.freeworkers.wait();
    auto worker_i = wo_a_sh.workers.front();
    wo_a_sh.add_workers_m.lock();
    wo_a_sh.workers.pop();
    wo_a_sh.add_workers_m.unlock();
    //将这个任务传递给工人。
    worker_task[worker_i].task_m.lock();
    worker_task[worker_i].task = task;
    worker_task[worker_i].task_m.unlock();
    worker_task[worker_i].hasTask.signal();
  }
  });//这是派发线程


}
void ThreadPool::schedule(const function<void(void)>& thunk) {
  //调度器获取一个任务，将其加入任务队列
  terminate.end_m.lock();
  terminate.end = false;
  terminate.end_m.unlock();
  tasks_m.lock();
  tasks.push_back(thunk);
  tasks_m.unlock();
  //并通知调度器有任务了，可以执行
  addtask.signal();
}
void ThreadPool::wait() {
  //等待所有的任务派发完成，
  //{
  // cout << "开始睡觉" << endl;
  //这里我可以等待让他们完成的任何和队列中的任务数量一致的时候
  finish_tasks_m.lock();
  finish_tasks_s.wait(finish_tasks_m,[this]{
  // cout << "finish tasks:" << finish_tasks << "tasks.size():" << tasks.size() <<endl;
  bool res = (finish_tasks == tasks.size());
  return res;
  });
  finish_tasks_m.unlock();
  // sleep(2);
  // cout << "醒了" << endl; 
  //}
  

  //并且已经执行完毕，所有工人都可以退出的状态
  terminate.end_m.lock();
  terminate.end = true;
  terminate.end_m.unlock();
  
}
ThreadPool::~ThreadPool() {
  wait();
  for(int i = 0;i  < numThreads; ++i)
  {
    worker_task[i].hasTask.signal();
  }
  for(int i = 0;i  < numThreads; ++i)
  {
    wts[i].join();
    // cout << i << "已经成功退出" << endl;
  }
  addtask.signal();
  dt.join();
  // cout << "析构完成" << endl;
}
