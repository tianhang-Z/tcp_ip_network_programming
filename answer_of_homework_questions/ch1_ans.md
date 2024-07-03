底层文件I/O和ANSI标准I/O的区别
====================

 

**一、先来了解下什么是文件I/O和标准I/O：**

文件I/O：文件I/O称之为不带缓存的IO（unbuffered I/O)。不带缓存指的是每个read，write都调用内核中的一个系统调用。也就是一般所说的低级I/O——[操作系统](http://lib.csdn.net/base/operatingsystem "操作系统知识库")提供的基本IO服务，与os绑定，特定于linix或unix平台。

标准I/O：标准I/O是ANSI C建立的一个标准I/O模型，是一个标准函数包和stdio.h头文件中的定义，具有一定的可移植性。标准I/O库处理很多细节。例如缓存分配，以优化长度执行I/O等。标准的I/O提供了三种类型的缓存。

（1）全缓存：当填满标准I/O缓存后才进行实际的I/O操作。   
（2）行缓存：当输入或输出中遇到新行符时，标准I/O库执行I/O操作。   
（3）不带缓存：stderr就是了。

  

**二、二者的区别**

      文件I/O 又称为低级磁盘I/O，遵循POSIX相关标准。任何兼容POSIX标准的操作系统上都支持文件I/O。标准I/O被称为高级磁盘I/O，遵循ANSI C相关标准。只要开发环境中有标准I/O库，标准I/O就可以使用。（Linux 中使用的是GLIBC，它是标准C库的超集。不仅包含ANSI C中定义的函数，还包括POSIX标准中定义的函数。因此，Linux 下既可以使用标准I/O，也可以使用文件I/O）。
    
      通过文件I/O读写文件时，每次操作都会执行相关系统调用。这样处理的好处是直接读写实际文件，坏处是频繁的系统调用会增加系统开销，标准I/O可以看成是在文件I/O的基础上封装了缓冲机制。先读写缓冲区，必要时再访问实际文件，从而减少了系统调用的次数。
    
      文件I/O中用文件描述符表现一个打开的文件，可以访问不同类型的文件如普通文件、设备文件和管道文件等。而标准I/O中用FILE（流）表示一个打开的文件，通常只用来访问普通文件。

  


**三、最后来看下他们使用的函数**

|      | 标准ＩＯ                              | 文件ＩＯ(低级IO) |
| ---- | ------------------------------------- | ---------------- |
| 打开 | fopen,freopen,fdopen                  | open             |
| 关闭 | fclose                                | close            |
| 读   | getc,fgetc,getchar fgets,gets fread   | read             |
| 写   | putc,fputc,putchar fputs,puts, fwrite | write            |

**1.fopen与open**

标准I/O使用fopen函数打开一个文件：

FILE\* fp=fopen(const char\* path,const char \*mod)

其中path是文件名，mod用于指定文件打开的模式的字符串，比如"r","w","w+","a"等等，可以加上字母b用以指定以二进制模式打开（对于 \*nix系统，只有一种文件类型，因此没有区别）,如果成功打开，返回一个FILE文件指针，如果失败返回NULL,这里的文件指针并不是指向实际的文 件，而是一个关于文件信息的数据包，其中包括文件使用的缓冲区信息。

文件IO使用open函数用于打开一个文件：

int fd=open(char \*name,int how);

与fopen类似，name表示文件名字符串，而how指定打开的模式：O\_RDONLY(只读),O\_WRONLY(只写）,O\_RDWR （可读可写),还有其他模式请man 2 open。成功返回一个正整数称为文件描述符，这与标准I/O显著不同，失败的话返回-1，与标准I/O返回NULL也是不同的。

  

**2.fclose与close**

与打开文件相对的，标准I/O使用fclose关闭文件，将文件指针传入即可，如果成功关闭，返回0，否则返回EOF  
比如：

if(fclose(fp)!=0)    
            printf("Error in closing file");

而文件IO使用close用于关闭open打开的文件，与fclose类似，只不过当错误发生时返回的是-1，而不是EOF，成功关闭同样是返回0。[C语言](http://lib.csdn.net/base/c "C语言知识库")用error code来进行错误处理的传统做法。

  

**3\. 读文件，getc,fscanf,fgets和read**

标 准I/O中进行文件读取可以使用getc，一个字符一个字符的读取，也可以使用gets（读取标准io读入的）、fgets以字符串单位进行读取（读到遇 到的第一个换行字符的后面），gets（接受一个参数，文件指针）不判断目标数组是否能够容纳读入的字符，可能导致存储溢出(不建议使用），而fgets使用三个参数：  
char \* fgets(char \*s, int size, FILE \*stream);

第一个参数和gets一样，用于存储输入的地址，第二个参数为整数，表示输入字符串的最大长度，最后一个参数就是文件指针，指向要读取的文件。最 后是fscanf，与scanf类似，只不过增加了一个参数用于指定操作的文件，比如fscanf(fp,"%s",words)  
文件IO中使用read函数用于读取open函数打开的文件，函数原型如下：

ssize\_t numread=read(int fd,void \*buf,size\_t qty);

其中fd就是open返回的文件描述符，buf用于存储数据的目的缓冲区，而qty指定要读取的字节数。如果成功读取，就返回读取的字节数目（小于等于qty）

  

**4\. 判断文件结尾**

如果尝试读取达到文件结尾，标准IO的getc会返回特殊值EOF，而fgets碰到EOF会返回NULL,而对于\*nix的read函数，情况有所不 同。read读取qty指定的字节数，最终读取的数据可能没有你所要求的那么多（qty），而当读到结尾再要读的话，read函数将返回0.

  

**5\. 写文件：putc,fputs,fprintf和write**

与读文件相对应的，标准C语言I/O使用putc写入字符，比如：

putc(ch,fp);

第一个参数是字符，第二个是文件指针。而fputs与此类似：

fputs(buf,fp);

仅仅是第一个参数换成了字符串地址。而fprintf与printf类似，增加了一个参数用于指定写入的文件，比如：

fprintf(stdout,"Hello %s.\\n","dennis");

切记fscanf和fprintf将FILE指针作为第一个参数，而putc,fputs则是作为第二个参数。

在文件IO中提供write函数用于写入文件，原型与read类似：

ssize\_t result=write(int fd,void \*buf ,size\_t amt);

fd是文件描述符，buf是将要写入的内存数据，amt是要写的字节数。如果写入成功返回写入的字节数，通过result与amt的比较可以判断是否写入正常，如果写入失败返回-1

  

**6\. 随机存取：fseek()、ftell()和lseek()**

标准I/O使用fseek和ftell用于文件的随机存取，先看看fseek函数原型

int fseek(FILE \*stream, long offset, int whence);

第一个参数是文件指针，第二个参数是一个long类型的偏移量（offset），表示从起始点开始移动的距离。第三个参数就是用于指定起始点的模式，stdio.h指定了下列模式常量：

SEEK\_SET            文件开始处   
SEEK\_CUR            当前位置   
SEEK\_END            文件结尾处

看几个调用例子：   
 fseek(fp,0L,SEEK\_SET); //找到文件的开始处   
 fseek(fp,0L,SEEK\_END); //定位到文件结尾处   
 fseek(fp,2L,SEEK\_CUR); //文件当前位置向前移动2个字节数

而ftell函数用于返回文件的当前位置，返回类型是一个long类型，比如下面的调用：

fseek(fp,0L,SEEK\_END);//定位到结尾   
        long last=ftell(fp); //返回当前位置

那么此时的last就是文件指针fp指向的文件的字节数。

与标准I/O类似，\*nix系统提供了lseek来完成fseek的功能，原型如下：

off\_t lseek(int fildes, off\_t offset, int whence);

fildes是文件描述符，而offset也是偏移量，whence同样是指定起始点模式，唯一的不同是lseek有返回值，如果成功就 返回指针变化前的位置，否则返回-1。whence的取值与fseek相同：SEEK\_SET,SEEK\_CUR,SEEK\_END，但也可以用整数 0,1,2相应代替。

**四、系统调用与库函数**

         上面我们一直在讨论文件I/O与标准I/O的区别，其实可以这样说，文件I/O是系统调用、标准I/O是库函数，看下面这张图：

![](https://img-blog.csdn.net/20160104131240831?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQv/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center)

POSIX：Portable Operating System Interface  可移植操作系统接口

ANSI：American National Standrads Institute  美国国家标准学会

  

**1、系统调用**

       操作系统负责管理和分配所有的计算机资源。为了更好地服务于应用程序，操作系统提供了一组特殊接口——系统调用。通过这组接口用户程序可以使用操作系统内核提供的各种功能。例如分配内存、创建进程、实现进程之间的通信等。
    
       为什么不允许程序直接访问计算机资源？答案是不安全。单片机开发中，由于不需要操作系统，所以开发人员可以编写代码直接访问硬件。而在32位嵌入式系统中通常都要运行操作系统，程序访问资源的方式都发生了改变。操作系统基本上都支持多任务，即同时可以运行多个程序。如果允许程序直接访问系统资源，肯定会带来很多问题。因此，所有软硬件资源的管理和分配都有操作系统负责。程序要获取资源（如分配内存，读写串口）必须由操作系统来完成，即用户程序向操作系统发出服务请求，操作系统收到请求后执行相关的代码来处理。
    
       用户程序向操作系统提出请求的接口就是系统调用。所有的操作系统都会提供系统调用接口，只不过不同的操作系统提供的系统调用接口各不相同。Linux 系统调用接口非常精简，它继承了Unix 系统调用中最基本的和最有用的部分。这些系统调用按照功能大致可分为进程控制、进程间通信、文件系统控制、存储管理、网络管理、套接字控制、用户管理等几类。

  



**2、库函数**

      库函数可以说是对系统调用的一种封装，因为系统调用是面对的是操作系统，系统包括Linux、Windows等，如果直接系统调用，会影响程序的移植性，所以这里使用了库函数，比如说C库，这样只要系统中安装了C库，就都可以使用这些函数，比如printf()  scanf()等，C库相当于对系统函数进行了翻译，使我们的APP可以调用这些函数；

  



**3、用户编程接口API**

     前面提到利用系统调用接口程序可以访问各种资源，但在实际开发中程序并不直接使用系统调用接口，而是使用用户编程接口（API）。为什么不直接使用系统调用接口呢？

原因如下：

1）系统调用接口功能非常简单，无法满足程序的需求。

2）不同操作系统的系统调用接口不兼容，程序移植时工作量大。

    用户编程接口通俗的解释就是各种库（最重要的就是C库）中的函数。为了提高开发效率，C库中实现了很多函数。这些函数实现了常用的功能，供程序员调用。这样一来，程序员不需要自己编写这些代码，直接调用库函数就可以实现基本功能，提高了代码的复用率。使用用户编程接口还有一个好处：程序具有良好的可移植性。几乎所有的操作系统上都实现了C库，所以程序通常只需要重新编译一下就可以在其他操作系统下运行。
    
    用户编程接口（API）在实现时，通常都要依赖系统调用接口。例如，创建进程的API函数fork()对应于内核空间的sys\_fork()系统调用。很多API函数西亚我哦通过多个系统调用来完成其功能。还有一些API函数不要调用任何系统调用。
    
     在Linux 中用户编程接口（API）遵循了在Unix中最流行的应用编程界面标准——POSIX标准。POSIX标准是由IEEE和ISO/IEC共同开发的标准系统。该标准基于当时想用的Unix 实践和经验，描述了操作系统的系统调用编程接口（实际上就是API），用于保证应用程序可以在源代码一级商多种操作系统上运行。这些系统调用编程接口主要是通过C库（libc )实现的。

  
