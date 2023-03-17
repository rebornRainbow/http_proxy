#### 任务1
熟悉所有的class
得到它的class图
模仿研究他的分类设计

#### 
  1、大概知道他的代码的流程了。
    - 要了解系统还是要直接看代码
    - 我稍微改了一点，他的设计还是好，面向对象设计

  2、出错原因找到
    - 对域名解析错误。
    - 在request ingestRequestLine方法中

####
- 出错的原因不可知
- 但是我知道怎么做了。

#### 自定义的异常
对于需要自己定义异常的时候，可以使用以下的方式，
```c++
class HTTPProxyException: public std::exception {
  public:
    HTTPProxyException() throw() {}

    HTTPProxyException(const std::string& message) throw() : message(message) {}

    const char *what() const throw() { return message.c_str(); }
  protected:
    std::string message;
};
```

这里定义了两张构造方法，都需要抛出异常

- 无参构造

- 以错误信息为参数的构造方法

- 重构了what函数，返回自己的错误信息。

- 其中message用来保存不同错误的引发原因.

之后可以让其他基本的类继承这个函数

```c++
class HTTPBadRequestException: public HTTPProxyException {
  public:
    HTTPBadRequestException() throw() {}
    HTTPBadRequestException(const std::string& message) throw() : HTTPProxyException(message) {}
};
```

#### 已基本实现

如有需要可以以后添加额外的功能



#### 问题记录

##### 问题1:用http网站的一些图片资源无法获取

原因这些图片是以超链接的形式出现的，但是这种图片用的是https协议


这是无法显示的图盘的标签
```html
<div class="uw-hero-image uw-homepage-slider slide-31 darktext darktextmobile slide-order-0" data-id="31" style="background: url(&quot;https://uw-s3-cdn.s3.us-west-2.amazonaws.com/wp-content/uploads/sites/81/2023/03/10165533/03-11-22-macarthur-genius-desktopF.jpg&quot;) center center / cover no-repeat transparent;"><div class="mobile-image" style="background-image:url('https://uw-s3-cdn.s3.us-west-2.amazonaws.com/wp-content/uploads/sites/81/2023/03/10165553/03-11-22-macarthur-genius-mobileB.jpg')"></div></div>
```

当前的代理还无法支持这种形式的数据，所以出现了错误

解决方案：

目前无法实现，估计需要支持https以后才能解决



#### 里程碑2


##### 黑名单
实现依靠c++中的regex类

这个类的使用需要我们知道正则表达式怎么书写

这里有一个有趣的[正则练习网站](https://regexlearn.com/zh-cn/learn/regex101)
这是一个简单的[正则练习的网站](https://tool.oschina.net/regex/)


简单的说明
这是简单的网站黑名单的例子

匹配黑名单网站的正则表达式写在blocked-domains.txt文件中
```python
(.*)\.microsoft.com(.*)#匹配所有含有microsoft.com的url
```

()：是正则中的分组
.*:表示匹配任意的字符串




##### cache 设计（单线程版）




主要需要三个方法

1、检查当前的请求和回应是否应该cache？shouldCache
```c++
  return maxAge != 0 && //这是用来设置缓存时间的变量
  request.getMethod() == "GET" && 
  response.getResponseCode() == 200 && 
  //上面两句表示我们只应该缓存成功GET的请求和他的回应
  response.permitsCaching();
  //上面的是检查服务器的请求是这个数据保是否可以缓存
```


2、当前是否有这个缓存？containsCacheEntry 

缓存以文件的形式存在服务器上

我们可以用二进制的方式打开文件，然后利用response的序列化功能得到所有的信息。


3、将当前的请求和回应添加到缓存中 cacheEntry

补充在http回应response中如果当前的数据头可以被缓存的话(存在"Cache-Control")，
请求中会有一个"max-age="属性，这个属性标注了这个缓存可以存在的时间。


检查控制台太麻烦，可以用一个


##### 错误记录，客户端放弃请求

客户端关闭了连接，这里出现了一个错误
每次出现可以缓存的时候，都
会引发一个客户端关闭连接错误
这样会造成一些严重的错误

我怀疑是因为在cache请求和回应的时候花费了太多的时间，
导致超时客户端关闭了链接，
所以尝试在回应了客户端之后在将内容记入缓存中

![这是效果图](2023-03-17-20-19-42.png)


我也不知道为什么好像中文网站会出错，但是英文网站不会

测试缓存时需要把浏览器自己的缓存关闭，要不然浏览器自己都有缓存，不会
向服务器发送请求

