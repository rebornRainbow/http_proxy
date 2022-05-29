@startuml
class HTTPProxy 
{
  
  +HTTPProxy(int argc, char *argv[])这是利用参数配置代理
  +unsigned short getPortNumber()返回正在监听的端口号
  +bool isUsingProxy()检测我们的代理是不是获取所有的请求
  +const std::string& getProxyServer()检测二级代理
  +unsigned short getProxyPortNumber()获取二级代理端口号
  +void acceptAndProxyRequest()接受服务器请求
}

class HTTPRequestHandler
{
  +void serviceRequest(const std::pair<int, std::string>& connection);//处理请求
 +void clearCache();//实现catch的时候实现
+void setCacheMaxAge(long maxAge);//实现catch的时候实现
}

class HTTPRequest {

/'
  提取解析，存储http请求的第一行，
 * 回忆一下任何有效的http请求格式都是
 *
 *   <method> <full-URL> <protocol-and-version>
 * e.g. 
 *   GET http://www.facebook.com/jerry HTTP/1.1
 *   POST http://graph.facebook.com/like?url=www.nytimes.com HTTP/1.1
 '/
  void ingestRequestLine(std::istream& instream) throw (HTTPBadRequestException);//提取，解析存储http请求的第一行

/'
 * Ingests everything beyond the first line up to the first
 * blank line (where all lines, including the visibly blank line,
 * end in either "\r\n" or just a "\n").  Recall that each
 * line is generally a name-value pair, so that the accumulation of
 * all header lines might look like this:
 * 提取请求第一行之后的所有键值对，<key-1>: <value-1>
 *
 *  <key-1>: <value-1>
 *  <key-2>: <value-2>
 *
 * The first ':' character separates the name from the value (except that the
 * "\r\n" or the "\n" at the end of each line is not considered to be part of
 * the value.  In fact, after the split around the ':', the name and value are
 * generally right and left-trimmed.
 *
 * One caveat: if a header line begins with a blank space, then it isn't introducing
 * a new name.  Instead, the line (after being right-trimmed) is providing a continuation 
 * of the previous line's value.
 '/
  void ingestHeader(std::istream& instream, const std::string& clientIPAddress);//提取请求第一行之后的所有键值对，<key-1>: <value-1>

/'
 * Ingests everything after the blank line following the header.
 * As opposed to the header section, the payload isn't necessarily
 * pure text (it might be a photo with pixel bytes that look like
 * text, or newlines, or gobbledygook).  That means that the payload
 * portion isn't read in as C++ string text, but as general character
 * data.
 '/
  void ingestPayload(std::istream& instream);//提取负载


/'
 * The next six methods are all const, inlined accessors.
 * Their behaviors should all be obvious.
 '/

  const std::string& getMethod() 提取所有的有效信息

  const std::string& getURL() 
  const std::string& getServer()
  unsigned short getPort() const 
  const std::string& getPath() 
  const std::string& getProtocol() 
/'
 * Returns true if and only if the supplied, case-insensitive
 * name exists within the collection of (zero or more) name-value
 * pairs.
 '/
  bool containsName(const std::string& name) const;//查看键值对里面是否有这个键
}


class HTTPResponse {

/'
 * Ingests everything up through and including the first
 * blank line of the server's response to an HTTP request.
 '/
  void ingestResponseHeader(std::istream& instream);接收服务器对HTTP请求的响应的所有内容，包括第一个空行。

/'
 * Ingests the payload portion of the server's response
 * to an HTTP request.
 '/
  void ingestPayload(std::istream& instream);接受负载部分

/'
 * Sets the protocol to be the one specified.  The
 * protocol should be "HTTP/1.0" or "HTTP/1.1".
 '/
  void setProtocol(const std::string& protocol);//将协议设置成"HTTP/1.0" or "HTTP/1.1".

/'
 * Returns the current protocol associated 
 * with the response.
 '/
  const std::string& getProtocol() //返回当前与回应相关的协议

/'
 * Installs the provided response code into the receiving
 * HTTPResponse object.
 '/
  void setResponseCode(int code);//设置提供的回应代码

/'
 * Retrieves the response code associated with the
 * HTTPResponse.
 '/
  int getResponseCode() //活的回应代码

/'
 * Manually updates the payload to be the provided
 * string (and updates the response header to be
 * clear about the content length as well).
 '/
  void setPayload(const std::string& payload);//将负载更新为提供的字符串

/'
 * Returns true if and only if the HTTPResponse is
 * cachable and can be returned as is when the same
 * exact request comes through later on.
 '/
  bool permitsCaching() const;//如果回应可以被catch，返回true.

/'
 * Returns the time-to-live, which is the number
 * of remaining seconds for which a cacheable object 
 * is still valid.
 '/
  int getTTL() const;//返回一个catchable 实例有效的时间
}

class HTTPProxyScheduler
{

  void setProxy(const std::string& server, unsigned short port);
  void clearCache() //利用request清理catch
  void setCacheMaxAge(long maxAge) { requestHandler.setCacheMaxAge(maxAge); }
  void scheduleRequest(int clientfd, const std::string& clientIPAddr) throw ();
  
  HTTPRequestHandler requestHandler;
}

class HTTPCache
{

/'
 * Constructs the HTTPCache object.
 '/
  HTTPCache();

/'
 * The following three functions do what you'd expect, except that they 
 * aren't thread safe.  In a MT environment, you should acquire the lock
 * on the relevant request before calling.
 '/
  bool containsCacheEntry(const HTTPRequest& request, HTTPResponse& response) const;
  bool shouldCache(const HTTPRequest& request, const HTTPResponse& response) const;
  void cacheEntry(const HTTPRequest& request, const HTTPResponse& response);

/'
 * Clears the cache of all entries.
 '/
  void clear();
  
/'
 * Sets the maximum number of seconds a cacheable response can live in the cache
 * before it's expunged.  -1 means don't override response's decision on how long a document
 * should be cached, 0 means never cache, any positive number represents how many seconds
 * a cacheable item is allowed to remain in the cache from the time it was placed there.
 '/
  void setMaxAge(long maxAge) { this->maxAge = maxAge; }

}


class HTTPPayload
{
/'
 * Ingests the entire payload from the provided istream, relying
 * on information present in the supplied HTTPHeader to determine
 * the payload size, and whether or not the payload is complete
 * or chunked.
 '/
  void ingestPayload(const HTTPHeader& header, std::istream& instream)//获取负载

/'
 * Sets the payload to be equal to the stream of characters contained
 * in the payload string.
 '/
  void setPayload(HTTPHeader& header, const std::string& payload);//设置负载
}

class HTTPHeader 
{
这个类的作用就是获取头部信息，内部存除键值对。

/'
 * Ingests the entire header of what's assumed to be either an
 * HTTP request or response.
 '/
  void ingestHeader(std::istream& instream);

/'
 * Adds (or updates) the provided name so that it's associated
 * with the string form of the supplied integer.  Note that the name comparison is
 * case-insensitive, so that "Expires" and "EXPIRES" are the considered
 * the same.
 '/
  void addHeader(const std::string& name, int value);
  
/'
 * Adds (or updates) the provided name so that it's associated
 * with the provided value string.  Note that the name comparison is
 * case-insensitive, so that "Expires" and "EXPIRES" are the considered
 * the same.
 '/
  void addHeader(const std::string& name, const std::string& value);

/'
 * Removes the provided name from the request header.
 '/
  void removeHeader(const std::string& name);

/'
 * Returns true if and only if the collection of name-value pairs
 * includes the one provided.  Note that the name comparison is
 * case-insensitive, so that "Expires" and "EXPIRES" are the considered
 * the same.
 '/
  bool containsName(const std::string& name) const;

/'
 * Returns the string form of the value associated with the provided
 * name.  Note, as above, that the name comparison is case-insensitive, 
 * so that "Expires" and "EXPIRES" are the considered the same.  If the
 * key isn't present, then the empty string is returned.
 '/
  const std::string& getValueAsString(const std::string& name) const;

/'
 * Returns the number (as a long) associated with the provided name.
 * Note, as above, that the name comparison is case-insensitive, 
 * so that "Expires" and "EXPIRES" are the considered the same.  If the
 * key isn't present, or if the associated value isn't purely numeric,
 * then 0 is returned.
 '/
  long getValueAsNumber(const std::string& name) const;
  

}


HTTPProxyScheduler -- HTTPRequestHandler
HTTPCache -- HTTPRequest

HTTPRequestHandler -- HTTPResponse

HTTPResponse -- HTTPPayload
HTTPRequest -- HTTPPayload

HTTPResponse -- HTTPHeader
HTTPRequest -- HTTPHeader
HTTPPayload -- HTTPHeader

@enduml
