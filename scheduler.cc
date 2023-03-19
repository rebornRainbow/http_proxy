/**
 * File: scheduler.cc
 * ------------------
 * Presents the implementation of the HTTPProxyScheduler class.
 */

#include "scheduler.h"
#include "thread-pool.h"
#include <utility>
using namespace std;

void HTTPProxyScheduler::scheduleRequest(int clientfd, const string& clientIPAddress) throw () {
  // 让请求处理器处理请求
  ThreadPool threadpool(64);
  threadpool.schedule([this,clientfd,clientIPAddress](){
    requestHandler.serviceRequest(make_pair(clientfd, clientIPAddress));
  });
  // cout << "测试" << endl;
}
