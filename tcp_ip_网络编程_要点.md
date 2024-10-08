## ch1理解网络系统和套接字

服务端 socket bind listen accept

客户端 socket connect 

```
#include <unistd.h>   //提供os访问接口       close  read write
#include <arpa/inet.h>   //处理ip
#include <sys/socket.h>   

#include <fcntl.h>  //文件处理函数  open

#include <stdio.h>
#include <stdlib.h>    exit() 
```

==套接字和open一个文件一样，都会得到一个描述符 对描述符fd可以进行读写==

```open()  write() close() read() ```

## ch2 套接字类型和协议设置

![image-20240704162001944](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704162001944.png)

![image-20240704162022913](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704162022913.png)

#### 两种type

面向连接的SOCK_STREAM    ：按序  没有数据边界 （write和read的次数没有太大的意义，因为有IO缓冲buffer）

面向消息的SOCK_DGRAM    : 不按序 不可靠 以快速为目的

## ch3 地址族和数据序列

```
/* 在头文件<netinet/in.h>中定义 */
struct in_addr
{
    in_addr_t s_addr;
};
struct sockaddr_in
{
    uint8_t         sin_len; /* POSIX不要求这个字段，它是OSI协议中新增的 */
    sa_family_t     sin_family;
    in_port_t       sin_port;
    struct in_addr  sin_addr;
    char            sin_zero[8]; /* 未使用 */
};
```

套接字地址结构的每个成员都是以**sin_**开头的。表示的就是socket internet。

- sin_family地址族字段：IPv4为**AF_INET**
- sin_zero这个字段一般置为0。
- ![image-20240704164854238](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704164854238.png)

### 字节序

intel 和 AMD 的cpu都是小端序  但是网络字节序是大端序

```c
/* 有些系统中#include <netinet/in.h> */
#include <arpa/inet.h>
uint16_t htons(uint16_t host16bitvalue);
uint32_t htonl(uint32_t host32bitvalue);
uint16_t ntohs(uint16_t net16bitvalue);
uint32_t ntohl(uint32_t net32bitvalue);
```

函数名中

- h:host(主机序)
- n:net(网络序)
- s：short类型。16位
- l:long类型。32位。*有的系统（如Digital Alpha）尽管long是64位，但htonl和ntohl返回的仍然是32位值*

上面四个函数，进行主机序和网络序之间16和32位的转换。即IP地址和端口号的转换。

### 网络地址初始化

```c
#include <arpa/inet.h>
/*将一个点分十进制串转换位网络字节顺序的IP地址，字符串有效返回1，否则为0*/
int inet_aton(const char *cp,struct in_addr *inp);
/*若字符串有效则返回32位二进制网络序IPv4地址，否则为INADDR_NONE*/
in_addr_t inet_addr(const char *cp);
/*将一个网络字节顺序的IP地址转换位它所对应的点分十进制串*/
char *inet_ntoa(struct in_addr in);
```



## ch4 基于TCP的服务端/客户端（1）

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240612105424147.png" alt="image-20240612105424147" style="zoom:67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240612105439091.png" alt="image-20240612105439091" style="zoom:67%;" />

accept会生成一个套接字

基于此实现迭代的回声服务器（多次受理请求）

```c
/*
回声客户端 存在缺陷（主要是因为tcp没有数据边界） 
问题1：数据太长 服务端分两次发送 客户端还未收到所有数据却从缓存区read
问题2：客户端多次write 服务端可能一次接受并返回
ch5解决该问题
*/
```

## ch5 基于TCP的服务端/客户端（2）

```c
// echo_client.c
// 记录累计接受的字节数
    while(1){
        fputs("input message (Q to quit)\n",stdout);
        fgets(message,BUFF_SIZE,stdin);

        if(!strcmp(message,"q\n")||!strcmp(message,"Q\n"))
            break;
        
        str_len=write(sock,message,strlen(message));
        printf("str_len: %d\n",str_len);
        int recv_len=0,recv_cnt=0;
        while(recv_len<str_len){
            recv_cnt=read(sock,&message[recv_len],BUFF_SIZE-1);
            printf("recv_cnt:%d\n",recv_cnt);
            if(recv_cnt==-1)
                error_handling("read() error");
            recv_len+=recv_cnt;
        }
        printf("message from server: %s\n",message);

    }
```

###  TCP 原理

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240611152653515.png" alt="image-20240611152653515" style="zoom:67%;" />	

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240611152721786.png" alt="image-20240611152721786" style="zoom: 80%;" />

#### 三次握手：

A: 你好，请求连接 ；

B: 好的 ，我已就绪；

 A: 谢谢

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704173727269.png" alt="image-20240704173727269" style="zoom:80%;" />

#### 数据交付过程

按序，超时重传，快速重传

![image-20240704173931801](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704173931801.png)

#### 四次挥手

A: 我想断开连接  

B: 稍等   B： 我就绪，可断开  

A:好的 谢谢

![image-20240704174011613](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240704174011613.png)

## ch6基于UDP

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708150755350.png" alt="image-20240708150755350" style="zoom:80%;" />

#### UDP仅需一个套接字

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708150901034.png" alt="image-20240708150901034" style="zoom:80%;" />

#### IO函数

UDP不会保持连接状态，每次传输都要添加目的地址

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620222210838.png" alt="image-20240620222210838" style="zoom:67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620222226006.png" alt="image-20240620222226006" style="zoom:67%;" />

#### UDP有数据边界

三次sendto发送的数据，需要三次recvfrom才能接收完

#### connect 创建连接的UDP

![image-20240708152025956](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708152025956.png)

## ch7优雅的断开套接字连接

![image-20240613173209232](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240613173209232.png)

### 为什么要半关闭

考虑一种需求，客户端连接服务端后，服务端向客户端传输一个大文件，客户端接收后发送“接收完毕”。

服务端只需发送即可，发送完成后，若等待客户端接收完发送信息后再断开，有点浪费时间；但又不能直接断开，必须等客户端回复才行。

客户端在接收时不能一直read，这样会造成阻塞。因此客户端必须知道何时接受完毕，之后回复“接收完毕”。

问题来了，服务端不断开输出流，客户端则会阻塞在read函数；服务端直接close，则无法接受客户端的回复。

![image-20240620160914234](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620160914234.png)

传输EOF后，客户端read会返回0，即停止接受

## ch8域名系统

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240613225705993.png" alt="image-20240613225705993" style="zoom:80%;" />

```
struct hostent
{
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  int h_length;			/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
#ifdef __USE_MISC
# define	h_addr	h_addr_list[0] /* Address, for backward compatibility.*/
#endif
};
```

## ch9套接字可选项

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240614164602814.png" alt="image-20240614164602814" style="zoom:80%;" />

### getsockop&setsockopt

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708153538697.png" alt="image-20240708153538697" style="zoom: 67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708153626950.png" alt="image-20240708153626950" style="zoom: 67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240614164611838.png" alt="image-20240614164611838" style="zoom:80%;" />

### SO_REUSEADDR

#### Time-wait状态

先断开的主机会进入该状态 造成端口有几分钟不能用

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708154208872.png" alt="image-20240708154208872" style="zoom:67%;" />

### TCP_NODELAY

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240614170950088.png" alt="image-20240614170950088" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708154133197.png" alt="image-20240708154133197" style="zoom:67%;" />

#### 大文件传输要禁用Nagle算法

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708154516176.png" alt="image-20240708154516176" style="zoom:67%;" />

## ch10多进程服务器端

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240614180712411.png" alt="image-20240614180712411" style="zoom:80%;" />

### 父和子进程的全局变量

==父进程和子进程并非共享全局变量==

子进程复制父进程的虚拟内存空间（包括堆栈等） 全局变量在父子进程中指向的地址是同一虚拟地址 但不是同一个物理地址

如果父子进程只是对全局变量做读操作，则父子进程在内存共享同一份全局变量。

如果父子进程中的任何一个对变量做修改操作，会在内存中拷贝一份副本，然后在这个副本上进行修改，修改完成后再通过MMU（内存管理单元）映射回去。【因此，会发现父子进程在同一虚拟地址的全局变量的值不同】

### 进程和僵尸进程

只有父进程主动要求获取子进程的结束状态值（return或者exit）时，子进程才会结束
#### 1 kill杀死元凶父进程
  严格的说，僵尸进程并不是问题的根源，罪魁祸首是产生大量僵死进程的父进程。因此，我们可以直接除掉元凶，通过kill发送SIGTERM或者SIGKILL信号。元凶死后，僵尸进程进程变成孤儿进程，由init充当父进程，并回收资源。命令为：kill -9 父进程的pid值强制杀死父进程。init进程就是所有进程的根进程

#### 2 父进程用wait或waitpid去回收资源
  父进程通过wait或waitpid等函数去等待子进程结束，但是不好，会导致父进程一直等待被挂起，相当于一个进程在干活，没有起到多进程的作用。

#### 3 通过信号机制，在处理函数中调用wait，回收资源
  通过信号机制，子进程退出时向父进程发送SIGCHLD信号，父进程调用signal(SIGCHLD,sig_child)去处理SIGCHLD信号，在信号处理函数sig_child()中调用wait进行处理僵尸进程。什么时候得到子进程信号，什么时候进行信号处理，父进程可以继续干其他活，不用去阻塞等待。

#### 几个销毁僵尸进程的函数

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240618162827362.png" alt="image-20240618162827362" style="zoom:67%;" />

==如果没有终止的子进程，wait会阻塞==

==而waitpid则不会阻塞==

![image-20240618162841545](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240618162841545.png)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240618162850458.png" alt="image-20240618162850458" style="zoom:67%;" />

### 信号处理

==为什么需要信号==

==因为父进程不能一直调用wait或者waitpid来查询子进程是否终止，因此利用信号技术，子进程终止时由操作系统告知父进程，父进程自动调用信号处理函数，在函数中wait/waitpid回收子进程==

#### signal函数

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240709235144263.png" alt="image-20240709235144263" style="zoom:67%;" />

==理解返回值为函数指针==，  

其中```void (*func)(int)```是函数指针

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240709234634339.png" alt="image-20240709234634339" style="zoom:67%;" />

```
signal(SIGCHLD,chld_end)
# 自己编写一个int chld_end(int)处理函数	
```



#### sigaction函数

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240709235809274.png" alt="image-20240709235809274" style="zoom:67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240710000131284.png" alt="image-20240710000131284" style="zoom:67%;" />

### 基于多任务的并发服务器

==为每一个客户端提供一个子进程==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240619231435420.png" alt="image-20240619231435420" style="zoom: 80%;" />

==fork之后，子进程会拥有父进程的文件描述符==。每个端口对应唯一的套接字。==套接字有自己的描述符，被复制之后，一个端口对应多个套接字，只有多个套接字被关掉，套接字才会销毁。==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620141237175.png" alt="image-20240620141237175" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620141243556.png" alt="image-20240620141243556" style="zoom:80%;" />

### IO分割的客户端



<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620141417793.png" alt="image-20240620141417793" style="zoom:80%;" />

## ch11 进程通信 (IPC)

### 基于管道

==管道与套接字一样，属于OS的资源==

==记住1入 0出==



<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620160453112.png" alt="image-20240620160453112" style="zoom:67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620161334784.png" alt="image-20240620161334784" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620161408718.png" alt="image-20240620161408718" style="zoom:80%;" />

### 双向通信需两个管道

==管道的问题：数据进入管道后成为无主数据，先从管道read的进程获取数据==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240620161434714.png" alt="image-20240620161434714" style="zoom: 67%;" />

## ch12 IO复用

==多进程意味着占用资源多==	

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240708153313947.png" alt="image-20240708153313947" style="zoom:67%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240623155134793.png" alt="image-20240623155134793" style="zoom:80%;" />

==IO复用服务器模型，只有服务端一个进程==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240623155144816.png" alt="image-20240623155144816" style="zoom:80%;" />

### select函数

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240710004050540.png" alt="image-20240710004050540" style="zoom:67%;" />

fd_set存储要监控的文件描述符，第0位设置为0表示要监视描述符0

==其中fd0是标准输入或输出（console）==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240711144643789.png" alt="image-20240711144643789" style="zoom:67%;" />

FD_ZERO  FD_SET FD_CLR 用于设置描述符

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716153835327.png" alt="image-20240716153835327" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240711144602403.png" alt="image-20240711144602403"  />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240711144807976.png" alt="image-20240711144807976"  />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240711144818283.png" alt="image-20240711144818283"  />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240711144827451.png" alt="image-20240711144827451" style="zoom:80%;" />

一定要在select函数调用之前设置timeout的值![image-20240716143141483](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716143141483.png)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716154048125.png" alt="image-20240716154048125" style="zoom:80%;" />

==select函数调用后，发生变化的描述符的标志位还是为1==

### 实现IO复用服务器

==原理是重复调用select函数，查询哪个fd发生了改变，然后对这个fd进行处理==

## ch13 多种IO函数

### send() & recv()    (linux下的)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716161547922.png" alt="image-20240716161547922"  />

![image-20240716163411264](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716163411264.png)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716163451144.png" alt="image-20240716163451144" style="zoom:80%;" />

#### MSG_OOB 发送紧急消息

带外传输  

结合示例代码：

```send(fd,"890",strlen("890"),MSG_OOB)```

![image-20240716204244881](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716204244881.png)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240716204252485.png" alt="image-20240716204252485" style="zoom:80%;" />

紧急指针前面一个字节会被`recv(fd,buf,buf_size,MSG_OOB)`读取，其余部分会被常用输入函数读取

#### MSG_PEEK和MSG_DONTWAIT

==PEEK用于查询输入缓冲是否有数据，调用recv()并设置PEEK后，读取了输入缓冲的数据，但缓冲中的数据仍然不会被删除==

### readv() & writev()函数

==writev()可以将多个数据合并之后一起发送 减少数据包的个数 有利于网络传输==

readv()可以把接受的数据保存到多个缓冲里（如多个char* buf)

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717145726445.png" alt="image-20240717145726445" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717145737294.png" alt="image-20240717145737294"  />

![image-20240717145750846](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717145750846.png)

![image-20240717145719179](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717145719179.png)

## ch14多播与广播

### 多播



![image-20240717205705586](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717205705586.png)

![image-20240717205629967](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717205629967.png)

### 广播

![image-20240717211749413](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240717211749413.png)

### 二者区别

只要加入多播组，不同网络中的主机都能收到多播数据。

而广播只能面向某个网络（指某个局域网)，只有该网络内的主机可以收到数据。

## ch15套接字和标准IO

### 标准IO函数的优点

移植性好，可以利用缓冲提高性能

创建套接字时，OS生成用于IO的缓冲；而标准IO函数将提供额外的另一缓冲的支持

![image-20240718161826295](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240718161826295.png)

其中套接字的缓冲主要是为了TCP，如数据重发时，重发的数据则存储在套接字的输出缓冲中

而标准IO函数的缓冲则是为了提高性能

### 文件描述符转换FILE结构体指针

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240718210819025.png" alt="image-20240718210819025" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240718210828414.png" alt="image-20240718210828414" style="zoom:80%;" />

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240718210834630.png" alt="image-20240718210834630" style="zoom:80%;" />

### 基于套接字的标准IO函数使用

将sock转换为两个FILE*分别用于读写

```c

        //套接字转换为FILE*
        readfp=fdopen(clnt_sock,"r");
        writefp=fdopen(clnt_sock,"w");
        while(!feof(readfp)){
            fgets(message,BUFF_SIZE,readfp);
            fputs(message,writefp);
            fflush(writefp);
        }
        fclose(readfp);
        fclose(writefp);
```

## ch16 关于IO分流的其他内容

#### 之前的分流方式

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240718212544130.png" alt="image-20240718212544130" style="zoom:80%;" />

#### 流分离的目的

* FILE指针按照读和写模式区分
* 区分IO缓冲 提高缓冲性能

#### 如何半关闭FILE

套接字文件描述符可以用shutdown半关闭，从而发送EOF，关闭写服务，但不关闭读服务

fclose(FILE* )会完全终止套接字 而不是半关闭

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719115056105.png" alt="image-20240719115056105" style="zoom:80%;" />

==调用fclose销毁任何一个FILE* 都会销毁描述符，同时也销毁了套接字==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719120413794.png" alt="image-20240719120413794" style="zoom:80%;" />

##### 如何让FILE*之一销毁 而套接字仍然存在呢

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719120604700.png" alt="image-20240719120604700" style="zoom:80%;" />

==简而言之  复制文件描述符 基于两个描述符分别创建读写FILE*==

==问题是，销毁一个FILE*之后，销毁一个描述符，但还存在另一个描述符，该描述符可以IO，因此并没有半关闭，也就不会发送EOF，还需要调用shutdown函数==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719120628744.png" alt="image-20240719120628744" style="zoom:80%;" />

#### 基于dup()和dup2()函数完成描述符的复制和半关闭

fork是创建两个进程，两个进程中分别有文件描述符原件和副本

==如何在一个进程中完成复制呢==

![image-20240719133541326](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719133541326.png)

两个描述符的值不同

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719133640018.png" alt="image-20240719133640018" style="zoom:80%;" />

==调用shutdown函数后，无论复制多少描述符，都会进入半关闭 并传递EOF==

## ch17 优于select的epoll

##### select的缺点

![image-20240719153739641](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719153739641.png)

==由于select需要向OS传递监视对象的信息 这会带来很大的负担 且无法通过优化代码来解决==

==selec要监视套接字的变化 因此要借助OS才能完成功能==

##### 如何降低开销呢

==仅向OS传递一次监视对象 当监视范围发送变化时 才通知OS变化的事项==

==linux对这种处理的支持方式是epoll  windows是IOCP==

##### epoll的优点

![image-20240719161528840](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719161528840.png)

### epoll相关函数

类别select  fd_set 存储监视范围 

FD_SET()函数添加或删除监视  

select查询监视对象是否变化

![image-20240719161617942](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719161617942.png)

epoll用epoll_create创建epoll描述符空间 

==epoll_event结构体用于保存变化的文件描述符==

![image-20240719184338460](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719184338460.png)

```
#include <sys/epoll.h>
int epoll_create(int size);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

int epoll_wait(int epfd, struct epoll_event *events,
               int maxevents, int timeout);
               
```

#### epoll_create

==向OS请求epfd的存储空间==

<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719171258542.png" alt="image-20240719171258542" style="zoom:80%;" />

#### epol_ctl

注册要监视的epoll描述符



<img src="./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719171307528.png" alt="image-20240719171307528" style="zoom:80%;" />

![image-20240719171315894](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719171315894.png)

##### op

表示动作，它由三个宏来表示

- EPOLL_CTL_ADD：注册新的fd到epfd中；
- EPOLL_CTL_MOD：修改已经注册的fd的监听事件；
- EPOLL_CTL_DEL：从epfd中删除一个fd；

##### event

可以是以下几个宏`逻辑或`的组合

| 宏   | 描述 |
| :--- | :--- |
| EPOLL**IN** |表示对应的文件描述符可以读（包括对端SOCKET正常关闭|
|EPOLL**OUT**|表示对应的文件描述符可以写|
|EPOLL**PRI**|表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）|
|EPOLL**ERR**|表示对应的文件描述符发生错误|
|EPOLL**HUP**|表示对应的文件描述符被挂断|
|EPOLL**ET**| 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的|
|EPOLL**ONESHOT**|只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里|

#### epoll_wait

![image-20240719172250385](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719172250385.png)

==使用malloc为最终存储发生事件的文件描述符集合epoll_event申请空间==

![image-20240719184509668](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240719184509668.png)

### 基于epoll的回声服务器

主要代码

```c
 //epoll相关变量
    struct epoll_event *ep_event;  //用于设置epoll_wait的event参数 ，保存监视的结果
    struct epoll_event event;  //用于设置epoll_ctl的event参数
    int epfd,event_cnt;
 //epoll配置
    epfd=epoll_create(EPOLL_SIZE);   //向OS申请epoll描述符的空间
    ep_event=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);  //为ep_event申请空间
    event.events=EPOLLIN;
    event.data.fd=serv_sock;
    //在epfd中注册serv_sock 监视event中的事件
    epoll_ctl(epfd,EPOLL_CTL_ADD,serv_sock,&event); 

 while (1)
    {
        //等待事件发生
        event_cnt=epoll_wait(epfd,ep_event,EPOLL_SIZE,-1);
        if(event_cnt==-1){
            puts("epoll_wait() error");
            break;
        }   

        //依次处理事件
        for(int i=0;i<event_cnt;i++){
            //serv_sock 收到数据 扩展epfd监视范围
            if(ep_event[i].data.fd==serv_sock){
                clnt_adr_sz=sizeof(clnt_adr);
                clnt_sock=accept(serv_sock,(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
                event.events=EPOLLIN;
                event.data.fd=clnt_sock;
                epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event); 
                printf("new connect clnt sock :%d \n",clnt_sock);
            }
            //其他sock收到数据 回声服务
            else{
                str_len=read(ep_event[i].data.fd,message,BUFF_SIZE);
                //EOF  关闭对应clnt_sock
                if(str_len==0) {
                    epoll_ctl(epfd,EPOLL_CTL_DEL,ep_event[i].data.fd,NULL); 
                    close(ep_event[i].data.fd);
                    printf("close clnt sock:%d \n",ep_event[i].data.fd);
                }
                //回声
                else{
                    printf("read str_len:%d \n",str_len);
                    write(ep_event[i].data.fd,message,str_len);
                }  
            }
        }
    }
```

### 条件触发和边缘触发

==LT（Level Trigger，水平触发或条件触发）模式 和 ET（Edge Trigger，边缘触发或边沿触发）==

条件触发：输入缓冲中只要有数据就会一直通知该事件（如：收到了一次数据但没有读取完，则后续还会继续通知

边缘触发：输入缓冲收到数据时仅注册一次事件

#### 实现边缘触发的回声服务器

```c
//套接字设置非阻塞模式
int flag=fcntl(fd,F_GETFL,0);
fcntl(fd,F_SETFL,flag|O_NONBLOCK);
```

`int errno`用于查看函数错误

边缘触发仅注册一次，因此需要读取完缓冲中的全部数据，read在输入缓冲为空时会返回-1，此时errno的值为EAGAIN

#### 条件触发和边缘触发对比

条件触发：收到数据后如果要延迟处理（不读取完或者不读取），则每次调用epoll_wait都会注册事件，事件会累计

而边缘触发：事件只会注册一次，可以灵活调整收到数据和处理数据的时间点。

## ch18 多线程服务器端实现

##### 多进程模型的缺点：

* 创建进程需要一定的内存和时间开销
* 进程间数据交换需要IPC技术
* 进程切换时需要频繁 的上下文切换（eg.进程A切换B进程时，需要 A的信息移除内存，读入B的信息，这就是上下文切换），会耗费很多时间

##### 多线程对应的优点：

* 创建和上下文切换更快
* 线程交换数据无需特殊技术

##### 多进程和多线程内存结构

多线程共享数据区和堆区

* ![image-20240722144744772](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722144744772.png)

![image-20240722144738351](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722144738351.png)

![image-20240722144846837](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722144846837.png)

### 线程的创建和运行

线程id  线程属性attr 线程函数地址 线程函数参数

![image-20240722144949189](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722144949189.png)

==gcc编译会失败==

问题的原因：pthread不是Linux下的默认的库，也就是在链接的时候，无法找到phread库中哥函数的入口地址，于是链接会失败。

解决：==在gcc编译的时候，附加要加 -lpthread参数即可解决。==

![image-20240722150823801](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722150823801.png)

==为了防止进程结束导致其线程意外终止，需要让进程等待线程==

对二级指针的理解见代码 thread2.c

#### 可在临界区内调用的函数

临界资源：一次仅允许一个进程使用的资源

临界区：进程中访问临界资源的代码

==gcc 加`-D_REENTRANT`可以使函数改为线程安全函数==

#### 当多线程访问同一变量时 存在问题

### 线程同步

#### 互斥量创建 销毁 使用

==调用lock函数时，若有其他线程进入临界区，则lock函数不会返回==

![image-20240722195415569](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722195415569.png)

![image-20240722195503014](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722195503014.png)

![image-20240722195543933](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722195543933.png)

#### 信号量

==此处只讨论二进制信号量 即只有0和1的信号量==

![image-20240722205907476](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722205907476.png)

![image-20240722205950756](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240722205950756.png)

==信号量为1时，调用wait会使信号量减1，然后进入临界区，信号量为0时，调用wait会阻塞==

### 多线程聊天室实现

![image-20240723120046445](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240723120046445.png)

==gcc编译时,添加`-D_REENTRANT -lpthread`参数== 

## ch24 制作http服务器端

![image-20240723152217005](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240723152217005.png)

![image-20240723152207521](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240723152207521.png)

![image-20240723155118703](./md_image/image_tcp_ip_%E7%BD%91%E7%BB%9C%E7%BC%96%E7%A8%8B_%E8%A6%81%E7%82%B9/image-20240723155118703.png)
