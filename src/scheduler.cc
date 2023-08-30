/**
 * File: scheduler.cc
 * ------------------
 * Presents the implementation of the HTTPProxyScheduler class.
 */

#include "scheduler.h"

#include <utility>
using namespace std;

HTTPProxyScheduler::HTTPProxyScheduler():threadpool(64){};

void HTTPProxyScheduler::scheduleRequest(int clientfd, const string& clientIPAddress) throw () {
  // 让请求处理器处理请求
  threadpool.schedule([this,clientfd,clientIPAddress](){
    requestHandler.serviceRequest(make_pair(clientfd, clientIPAddress));
  });
  // cout << "测试" << endl;
}
