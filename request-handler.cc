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
#include <socket++/sockstream.h> // for sockbuf, iosockstream
using namespace std;

//打印信息的类
class Log
{
  public:
   void log_print(string content)
   {
     cout << "this is log content:" << content << endl;
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
  blacklist.addToBlacklist("./blocked-domains.txt");
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
  
  // cout << "处理之前这是获取到的请求" << client_request << endl;
  /*这是获取客户端的请求，为了获取真正的回应要吧这个转给请求的服务器*/
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
  // cout << "处理之后这是获取到的请求" << client_request << endl;
  
  // cout << client_request.getServer() << endl;
}


void getServerResponse(HTTPCache &cache,HTTPBlacklist &blacklist,HTTPRequest & client_request,iosockstream &ss)
{
  // Log log;
  // log.log_print("开始");
  //创建客户端的文件描述符号
  

  // cout << oslock 
  // << client_request.getMethod() << " "
  // << client_request.getPath() << " "
  // << client_request.getProtocol() 
  // << endl << osunlock;

  // cout << "打开成功!!!\n" << endl;
  string host = client_request.getServer();
  unsigned short port = client_request.getPort();
  // log.log_print(host);

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
      cout << "禁止访问" << endl;
      ss  << response404 << endl;
      ss.flush();
      return;
    }


  int serverfd = createClientSocket(host,port);
  
  if(serverfd == -1)
  {
    //服务器打不开应该返回
    HTTPResponse response404;
		response404.setResponseCode(404);
		response404.setProtocol("HTTP/1.0");
		response404.setPayload("Not Found");
    cout << "404" << endl;
    ss  << response404 << endl;
    ss.flush();
    return;

  }
  // cout << "数字是" <<serverfd << endl;
  sockbuf sb(serverfd);
  iosockstream serverSs(&sb);//服务器的文件流
  // log.log_print("创建完成");
    /**
     * 出错日记
     * 目的是想服务器发送请求代码，
     *  失败代码：terminate called after throwing an instance of 'sockerr'
     * 原因：
     *  原因在于上面服务器打开失败
     * 改正：
     *  以后对与各种请求要对错误进行处理
     */
  HTTPResponse response;
  /**
   * 里程碑 2
   * 先检查是否有一个有效的缓存，如果有的
   * 直接将缓存中的回应返回
   */

  // if(cache.containsCacheEntry(client_request,response))
  // {
  //   cout << "存在缓存" << endl;
  //   ss  << response << endl;
  //   ss.flush();
  //   return;
  // }
  

  serverSs << client_request << endl;//这里是导致出错的原因
  
  serverSs.flush();//向服务器发送文件请求。
  // log.log_print("请求发送完成");
  // //接受来自服务器的回应并且准发给客户端
  
  response.ingestResponseHeader(serverSs);

  //如果客户端的请求时HEAD就不应该获取负载
  if(client_request.getMethod() != "HEAD")
    response.ingestPayload(serverSs);

  /**
   * 里程碑2 检查是否可以缓存
   * 如果可以就加入缓存
   */
  // if(cache.shouldCache(client_request,response))
  // {
  //   cout << "可以缓存" << endl;
  //   cache.cacheEntry(client_request,response);
  // }

  // cout << response << endl;
  ss  << response << endl;
  ss.flush();
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


  //尝试将所有的connect全部拦截，现在只支持CONNECT以外的方法。
  if(client_request.getMethod() == "CONNECT")
  {
    cout << oslock 
    << "CONNECT失败" << endl
    << client_request.getMethod() << " "
    << client_request.getURL() << " "
    << client_request.getProtocol() 
    << endl << osunlock;
    HTTPResponse response;
    response.setResponseCode(200);
    response.setProtocol("HTTP/1.0");
    response.setPayload("暂不支持CONNECT方法" );
    ss << response;
    ss.flush();
    return;
  }
  else
  {
    cout << oslock 
    << "其他请求" << endl
    << client_request.getMethod() << " "
    << client_request.getURL() << " "
    << client_request.getProtocol() 
    << endl << osunlock;
  }
  
  /*需要获取服务器的回应，并且发送回客户端**/
  getServerResponse(cache,blacklist,client_request,ss);
}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() {}
void HTTPRequestHandler::setCacheMaxAge(long maxAge) {}
