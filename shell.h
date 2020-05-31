#pragma once

#include "/home/exhina/MyLearn/MyInclude/perror.h"
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define NULLFLAG 0
#define OUT      1
#define IN       2
#define PIPE     3




class shell
{
public:

    shell() :  oflag(NULLFLAG), iflag(NULLFLAG), pflag(NULLFLAG), pnum(0), dnum(0)
    { init(); }

    ~shell() { shutdonwn(); }



    void Shell();

    void show()
    {
        std::cout << "读到的输入为" << rbuf << std::endl;
        for(int i = 0; i <= dnum; ++i)
        {
            std::cout << "解析到命令行参数为" ;
            std::cout << argv[i] << std::endl;
        }
    }
    
    

private:

    char* rbuf;  //从用户输入读到的待处理字符串缓冲区
    char** argv; //解析完的命令存放地址
    int oflag;   //重定向符 ">" 标志
    int iflag;   //重定向符 "<" 标志
    int pflag;   //管道符   "|" 标志
    int pnum;    //管道数量
    
    int dnum;    //命令行参数数量

    inline void arg_init()
    {
        dnum = 0; 
        pnum = 0;
        oflag = NULLFLAG;        
        iflag = NULLFLAG;       
        pflag = NULLFLAG;

    }

    inline void init()
    {
        rbuf = new char[1024];
        memset(rbuf ,0, 1024);
        argv = new char*[32];
        memset(argv, 0, 32);
    }

    inline void shutdonwn()
    {
        delete [] rbuf;
        delete [] argv;
    }

    bool GetStr(FILE* stream);              //从标准输入中读取字符串 - 考虑调用fgets()函数

    void Partition();                       //解析分割读到的字符串

    void Child(int& std_out, int& std_in);  //子进程执行
    
    void OPutRedirect(int& std_out);        //输出重定向

    void Pipe(int& std_out, int& std_in);   //管道的处理
};

