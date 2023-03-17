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

