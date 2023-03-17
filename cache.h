/**
 * File: cache.h
 * -------------
 * Defines a class to help manage an HTTP response cache.
 * 定义一个了来帮助管理一个HTTP回应的cache(临时存储东西的地方)
 */

#ifndef _cache_
#define _cache_

#include <cstdlib>
#include <string>
#include <mutex>
#include <sys/time.h>
#include "request.h"
#include "response.h"

class HTTPCache {
 public:

/**
 * Constructs the HTTPCache object.
 * 构造函数
 */
  HTTPCache();

/**
 * The following three functions do what you'd expect, except that they 
 * aren't thread safe.  In a MT environment, you should acquire the lock
 * on the relevant request before calling.
 * 接下来的三个函数完成了你期待的事情，除了他们不是线程安全的，，在多线程的环境中，
 * 你应该需要在调用前，在相关的请求上加锁
 * 这里之后需要
 */


  /**
   * @brief 
   *  主要检查当前的cache是否含有这个请求
   * @param request 
   * @param response 
   * @return true 
   * @return false 
   */
  bool containsCacheEntry(const HTTPRequest& request, HTTPResponse& response) const;
  /**
   * @brief 
   * 检查这个请求是否可以cache
   * @param request 
   * @param response 
   * @return true 
   * @return false 
   */
  bool shouldCache(const HTTPRequest& request, const HTTPResponse& response) const;
  /**
   * @brief 
   * 将请求添加到cache项
   * @param request 
   * @param response 
   */
  void cacheEntry(const HTTPRequest& request, const HTTPResponse& response);

/**
 * Clears the cache of all entries.
 * 清楚所有的项
 */
  void clear();
  
/**
 * 设置一个缓存的回应在它消失之前可以在cache中保存的最大的秒数。
 * -1表示不推翻默认值
 * 0表示不缓存
 * 任意的正整数表示秒数
 * Sets the maximum number of seconds a cacheable response can live in the cache
 * before it's expunged.  -1 means don't override response's decision on how long a document
 * should be cached, 0 means never cache, any positive number represents how many seconds
 * a cacheable item is allowed to remain in the cache from the time it was placed there.
 */
  void setMaxAge(long maxAge) { this->maxAge = maxAge; }
  
 private:
  std::string getCacheDirectory() const;
  size_t hashRequest(const HTTPRequest& request) const;
  std::string hashRequestAsString(const HTTPRequest& request) const;
  std::string serializeRequest(const HTTPRequest& request) const;
  bool cacheEntryExists(const std::string& filename) const;
  std::string getRequestHashCacheEntryName(const std::string& requestHash) const;
  void ensureDirectoryExists(const std::string& directory, bool empty = false) const;
  std::string getCurrentTime() const;
  std::string getExpirationTime(int ttl) const;
  bool cacheEntryFileNameIsProperlyStructured(const std::string& cachedFileName) const;
  void extractCreateAndExpireTimes(const std::string& cachedFileName, time_t& createTime, time_t& expirationTime) const;
  bool cachedEntryIsValid(const std::string& cachedFileName) const;
  std::string getHostname() const;

  long maxAge;
  std::string cacheDirectory;
};

#endif
