/**
 * File: request-handler.h
 * -----------------------
 * Defines the HTTPRequestHandler class, which fully proxies and
 * services a single client request.  
 */

#ifndef _request_handler_
#define _request_handler_

#include <utility>
#include <string>
#include "blacklist.h"
#include "cache.h"

class HTTPRequestHandler {
public:
  HTTPRequestHandler();
  void serviceRequest(const std::pair<int, std::string>& connection) throw();
  void clearCache();
  void setCacheMaxAge(long maxAge);

  /**
   * @brief 里程碑2
   * 添加一个黑名单的功能
   */
  private:
    HTTPBlacklist blacklist;
    HTTPCache cache;
};

#endif
