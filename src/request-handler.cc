/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */
#include "request.h"
#include "request-handler.h"
#include "response.h"
#include "ostreamlock.h"
#include "client-socket.h"
#include "watchset.h"

using namespace std;

// #define DEBUG

//打印信息的类
class Log
{
  public:
   void log_print(string content)
   {
#ifdef DEBUG
     cout << oslock << "this is log content:" << content << endl << osunlock;
#endif
   }
};

/**
 * @brief 
 * Construct a new HTTPRequestHandler::HTTPRequestHandler object
 * 配置私有成员 HTTPBlacklist
 */
HTTPRequestHandler::HTTPRequestHandler()
{
  //初始化黑名单
  blacklist.addToBlacklist("../config/blocked-domains.txt");
}


/**
 * @brief Get the Client Request object
 * 这是我用来获取客户端发送的请求的类
 * @param connection 
 * @param client_request 
 * @param ss 
 */
void getClientRequest(const pair<int, string>& connection,
  HTTPRequest & client_request,
  iosockstream &ss)
{
  

  // 在获取客户端的请求行时需要捕获错误  
  try{
		client_request.ingestRequestLine(ss);
	} catch (const HTTPBadRequestException& hpe) {
		cerr << oslock 
    << "HTTP Bad Request: " 
    << hpe.what() 
    << endl 
    << osunlock;
		return;
	}
  client_request.ingestHeader(ss,connection.second);
    //添加一个数据报头 告知服务器转发的协议。
    client_request.addHeader("x-forwarded-proto","http");

    if( !client_request.containsName("x-forwarded-for") )
      //设置他的值是发起请求的客户端的地址
      client_request.addHeader("x-forwarded-for",connection.second);
    else
    {
      string urls = client_request.getValueAsString("x-forwarded-for");
      urls += ","+connection.second;
      client_request.addHeader("x-forwarded-for",urls);
    }

}


void getServerResponse(HTTPCache &cache,HTTPBlacklist &blacklist,
HTTPRequest & client_request,
HTTPResponse &response,iosockstream &ss)
{
  // Log log;
  // log.log_print("开始");
  //创建客户端的文件描述符号
  

  string host = client_request.getServer();
  unsigned short port = client_request.getPort();
  // log.log_print(host);




  int serverfd = createClientSocket(host,port);
  
  if(serverfd == -1)
  {
    //服务器打不开应该返回
    HTTPResponse response404;
		response404.setResponseCode(404);
		response404.setProtocol("HTTP/1.0");
		response404.setPayload("<h1 align=\"center\">Not Found</h1>");

    cout << oslock << "404" << endl << osunlock;
    ss  << response404 << endl;
    ss.flush();
    return;

  }
  sockbuf sb(serverfd);
  iosockstream serverSs(&sb);//服务器的文件流
  /**
   * 里程碑 2
   * 先检查是否有一个有效的缓存，如果有的
   * 直接将缓存中的回应返回
   */

  if(cache.containsCacheEntry(client_request,response))
  {
#ifdef DEBUG
    cout << oslock<< "存在缓存" << endl << osunlock;
#endif
    ss  << response << endl;
    ss.flush();
    return;
  }
  

  serverSs << client_request << endl;//这里是导致出错的原因
  
  serverSs.flush();//向服务器发送文件请求。
  // log.log_print("请求发送完成");
  // //接受来自服务器的回应并且准发给客户端
  
  response.ingestResponseHeader(serverSs);

  //如果客户端的请求时HEAD就不应该获取负载
  if(client_request.getMethod() != "HEAD")
    response.ingestPayload(serverSs);

  ss  << response << endl;
  ss.flush();

  /**
   * 里程碑2 检查是否可以缓存
   * 如果可以就加入缓存
   * 这里过程太长了还是怎么样，这个过程没有回应，客户端就返回了
   * 导致了出错
   */

}


//服务请求，将客户端的请求发送个目的服务器然后转发给
void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
  //这里是对客户端的回应
  //第一个参数是客户端的链接
  
  //对客户端的请求应该是将其传递给请求的服务器，在吧服务器穿回来的内发送回去
  sockbuf sb(connection.first);//这个参数是客户端的文件描述符号。
  HTTPRequest client_request;//客户端发起的请求，应该交给服务器
  //表示客户端文件的文件流
  iosockstream ss(&sb);
  /*这是获取客户端的请求，为了获取真正的回应要吧这个转给请求的服务器*/
  getClientRequest(connection,client_request,ss);




    /**
   * @brief 里程碑2
   * 检查访问的内容是否是黑名单的内容，
   * 如果是返回404
   */
    if(!blacklist.serverIsAllowed(client_request.getURL()))
    {
      //服务器打不开应该返回
      HTTPResponse response404;
      response404.setResponseCode(404);
      response404.setProtocol("HTTP/1.0");
      response404.setPayload("<h1 align=\"center\">Forbidden Content</h1>");
// #ifdef DEBUG
      cout << oslock << "禁止访问" << osunlock <<endl;
// #endif
      ss  << response404 << endl;
      ss.flush();
      return;
    }



  //尝试将所有的connect全部拦截，现在只支持CONNECT以外的方法。
  if(client_request.getMethod() == "CONNECT")
  {
    int severfd = createSocket(client_request);
    sockbuf sfb(severfd);
    iosockstream serverss(&sfb);

#ifdef DEBUG
    cout << oslock 
    << "CONNECT" << endl
    << client_request.getMethod() << " "
    << client_request.getURL() << " "
    << client_request.getProtocol() 
    << endl << osunlock;
#endif
    HTTPResponse response;
    response.setResponseCode(200);
    response.setProtocol("HTTP/1.0");
    ss << response;
    ss.flush();

    manageClientServerBridge(serverss,ss);
#ifdef DEBUG
    cout << oslock << endl << osunlock;
#endif
    return;
  }
  else
  {
#ifdef DEBUG
    cout << oslock 
    << endl
    << "其他请求" << endl
    << client_request.getMethod() << " "
    << client_request.getURL() << " "
    << client_request.getProtocol() 
    << endl << osunlock;
#endif
  }
  HTTPResponse response;
  
  /*需要获取服务器的回应，并且发送回客户端**/
  getServerResponse(cache,blacklist,client_request,response,ss);
  if(cache.shouldCache(client_request,response))
  {
#ifdef DEBUG
    cout << oslock << "可以缓存" << endl << osunlock;
#endif
    cache.cacheEntry(client_request,response);
#ifdef DEBUG
    cout << oslock << "缓存成功" << endl << osunlock;
#endif
  }else
  {
#ifdef DEBUG
    cout << oslock << "不可以缓存" << endl << osunlock;
#endif
  }
}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() {
  cache.clear();
}
void HTTPRequestHandler::setCacheMaxAge(long maxAge) {
  cache.setMaxAge(maxAge);
}

const size_t kTimeout = 5;
const size_t kBridgeBufferSize = 1 << 16;
void HTTPRequestHandler::manageClientServerBridge(iosockstream& client, 
                                                  iosockstream& server) {
  // get embedded descriptors leading to client and origin server
  int clientfd = client.rdbuf()->sd();
  int serverfd = server.rdbuf()->sd();
  
  // monitor both descriptors for any activity 
  ProxyWatchset watchset(kTimeout);
  watchset.add(clientfd);
  watchset.add(serverfd);
  
  // map each descriptor to its surrounding iosockstream and the one
  // surrounding the descriptor on the other side of the bridge we're building 
  map<int, pair<iosockstream *, iosockstream *>> streams;
  streams[clientfd] = make_pair(&client, &server);
  streams[serverfd] = make_pair(&server, &client);
#ifdef DEBUG
  cout << oslock << buildTunnelString(client, server) << "Establishing HTTPS tunnel" << endl << osunlock;
#endif
  
  while (!streams.empty()) {
    int fd = watchset.wait();
    if (fd == -1) break; // return value of -1 means we timed out
    iosockstream& from = *streams[fd].first;
    iosockstream& to = *streams[fd].second;
    char buffer[kBridgeBufferSize];
    from.read(buffer, 1); // attempt to read one byte to see if we have one
    if (from.eof() || from.fail() || from.gcount() == 0) { 
       // in here? that's because the watchset detected EOF instead of an unread byte
       watchset.remove(fd); 
       streams.erase(fd); 
       continue; 
    }
    to.write(buffer, 1);
    to.flush();
    // TODO: additional code that you'll write to read all available bytes from the
    // source and transport them to the other side of the bridge
    while(true)
    {
      size_t readNum = from.readsome(buffer, sizeof(buffer));
      if(readNum == 0) break;
      if (from.eof() || from.fail() || from.gcount() == 0) { 
        // in here? that's because the watchset detected EOF instead of an unread byte
        watchset.remove(fd); 
        streams.erase(fd); 
        break;
      }
      to.write(buffer, readNum);
      to.flush();
#ifdef DEBUG
      cout << oslock << buildTunnelString(from, to) << readNum << endl << osunlock; 
#endif
    }
    to.flush();
  }
#ifdef DEBUG
  cout << oslock << buildTunnelString(client, server) << "Tearing down HTTPS tunnel." << endl << osunlock;
#endif
}

string HTTPRequestHandler::buildTunnelString(iosockstream& from, iosockstream& to) const {
  return "[" + to_string(from.rdbuf()->sd()) + " --> " + to_string(to.rdbuf()->sd()) + "]: ";
}

int HTTPRequestHandler::createSocket(const HTTPRequest& request) {
    int socketD = createClientSocket(request.getServer(), request.getPort());
        
    if (socketD == kClientSocketError) {
        cerr << "Server could not be reached."<<endl;
        cerr << "Abborting"<<endl;

    }

    return socketD;
    
}