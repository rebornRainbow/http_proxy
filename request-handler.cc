/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */
#include "request.h"
#include "request-handler.h"
#include "response.h"
#include "client-socket.h"
#include <socket++/sockstream.h> // for sockbuf, iosockstream
using namespace std;


class Log
{
  public:
   void log_print(string content)
   {
     cout << "this is log content:" << content << endl;
   }
};


void getClientRequest(const pair<int, string>& connection,
  HTTPRequest & client_request,
  iosockstream &ss)
{
  /*这是获取客户端的请求，为了获取真正的回应要吧这个转给请求的服务器*/
  client_request.ingestRequestLine(ss);
  client_request.ingestHeader(ss,connection.second);
    //对请求的处理
    client_request.addHeader("x-forwarded-proto","http");

    if( !client_request.containsName("x-forwarded-for") )
      client_request.addHeader("x-forwarded-for",connection.second);//设置他的值是发起请求的客户端的地址
    else
    {
      string urls = client_request.getValueAsString("x-forwarded-for");
      urls += ","+connection.second;
      client_request.addHeader("x-forwarded-for",urls);
    }
  cout << client_request << endl;
  cout << client_request.getServer() << endl;
}


void getServerResponse(HTTPRequest & client_request,iosockstream &ss)
{
  Log log;
  log.log_print("开始");
  //创建客户端的文件描述符号
  //
  string host = client_request.getServer();
  unsigned short port = client_request.getPort();
  log.log_print(host);
  int serverfd = createClientSocket(host,port);
  if(serverfd == -1)
  {
    cerr << host << ":" << port << "服务器打开失败" << endl; 
    exit(-1);
  }
  cout << "数字是" <<serverfd << endl;
  sockbuf sb(serverfd);
  iosockstream serverSs(&sb);//服务器的文件流
  log.log_print("创建完成");
    /**
     * 出错日记
     * 目的是想服务器发送请求代码，
     *  失败代码：terminate called after throwing an instance of 'sockerr'
     * 原因：
     *  原因在于上面服务器打开失败
     * 改正：
     *  以后对与各种请求要对错误进行处理
     */
  serverSs << client_request ;//这里是导致出错的原因
  log.log_print("请求发送完成");
  serverSs.flush();//向服务器发送文件请求。
  // //接受来自服务器的回应并且准发给客户端
  HTTPResponse response;
  response.ingestResponseHeader(serverSs);
  response.ingestPayload(serverSs);

  ss  << response << endl;
  ss.flush();
}

void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection) throw() {
  //这里是对客户端的回应
  //第一个参数是客户端的链接
    //对客户端的请求应该是将其传递给请求的服务器，在吧服务器穿回来的内容吗发挥起
  sockbuf sb(connection.first);//这个参数是客户端的文件描述符号。
  HTTPRequest client_request;//客户端发起的请求，应该交给服务器

  iosockstream ss(&sb);

  /*这是获取客户端的请求，为了获取真正的回应要吧这个转给请求的服务器*/
  getClientRequest(connection,client_request,ss);
  /*需要获取服务器的回应，并且发送回客户端**/
  getServerResponse(client_request,ss);

}

// the following two methods needs to be completed 
// once you incorporate your HTTPCache into your HTTPRequestHandler
void HTTPRequestHandler::clearCache() {}
void HTTPRequestHandler::setCacheMaxAge(long maxAge) {}
