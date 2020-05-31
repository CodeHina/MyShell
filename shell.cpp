#include "shell.h"


bool shell::GetStr(FILE* stream)
{
    
    /*
     * char* fgets(char* s,size_t size, FILE* stream);
     *
     * s - 缓冲区首地址 size - 从输入读入size-1个字符 文件流
     *
     * n <= 0 失败返回NULL - n = 1 返回空串"" - 成功返回首地址
     *
     * n > 当前读入的字符串长度 - 读到末尾换行符就返回并在末尾插入'\0' 
     *      
     *      如 123ab(\n) 实际enter后读入为6个字符  - strlen(s) == 6 - 抹除'\n' -> s[strlen[s]-1] = '\0'
     * 
     * n < 当前行读入字符长度 - 由于没有读到'\n'
     *      
     *      123abc
     *      char s[5];
     *      此时只读入4个字符 - 123a
     *      以及会在末尾添加'\0'
     *
     */

    char* ret = fgets(rbuf,1024, stream);

    rbuf[strlen(rbuf) - 1] = '\0';  //抹除末尾的'\n'

    return (ret == nullptr) ? false : true;
}

void shell::Partition()
{
    int i = 0;
    while(rbuf[i] != '\0')
    {
        if(rbuf[i] == ' ')          //当前字符为空格
        {
            i++;
            continue;               //考察下个字符
        }
        else
        {
            if(rbuf[i] == '>')      //判断是否需重定向
                oflag = OUT;    
            if(rbuf[i] == '|')      //判断是否需要用到管道
            { 
                pflag = PIPE; pnum++; 
            }

            argv[dnum] = rbuf + i; //定位命令字符串首地址
            i++;
            while(rbuf[i] != '\0') //将此字符串末尾的空格置为'\0'
            {
                if(rbuf[i] == ' ')
                {
                    rbuf[i] = '\0'; 
                    i++;
                    dnum++;
                    break;
                }
                else
                {
                    i++;
                }
            }
        }
    }
    argv[dnum + 1] = NULL;       //末尾字符置为NULL
}

void shell::Shell()
{

    while(1)
    {

        arg_init();                     //参数标志初始化
        std::cout << "MyShell : ";      //提示字符串
        GetStr(stdin);                  //读取用户输入
        Partition();                    //拆分解析命令
            
        int std_out = -1;               //STDOUT_FILENO备份
        int std_in = -1;                //STDIN_FILENO备份

        int ret = fork();               //创建子进程执行命令
        if(ret == 0)                    //子进程
        {
            Child(std_out,std_in);
        }
        else if(ret > 0)                //父进程
        {
            wait(NULL);                 //阻塞等待回收子进程
            
            if(oflag == OUT)            //进行过文件重定向输出
            {
                dup2(std_out, STDOUT_FILENO);//恢复标准输出
            }   
            if(pflag == PIPE)
            {
                dup2(std_in, STDIN_FILENO);
                dup2(std_out, STDOUT_FILENO);
            }
            
        }

    }


}

void shell::Child(int& std_out, int& std_in)
{
    if(strcmp(argv[0],"\n") == 0)//跳出换行输出
        exit(EXIT_SUCCESS);

    if(oflag == OUT)            //若需要重定向输出 - 将">"后内容截断
    {
        OPutRedirect(std_out);  //重定向输出
    }

    if(pflag == PIPE)           //若需要用到管道
    {
        Pipe(std_out,std_in);
        return;
    }

    execvp(argv[0], argv);  //执行相关命令
    perror("execvp error");


}

void shell::OPutRedirect(int& std_out)
{

    int i = 0;              //字符位置标签
    for(; i <= dnum; ++i)   //获取待输出文件在命令参数字符数组的位置
    {
        if( strcmp(">",argv[i]) == 0 )
            break;
    }   
        
    int fd = open(argv[i+1], O_CREAT | O_RDWR, 0644);   //打开此文件 - 不存在创建之
    Perror("open",fd);

    std_out = dup(STDOUT_FILENO);                       //拷贝一份标准输出(实际为执行标准输出的指针)

    int ret = dup2(fd, STDOUT_FILENO);                  //将标准输出重定向到此文件中
    Perror("dup2", ret);    
           
    close(fd);                      
        
    argv[i] = NULL;         //将">"置为NULL - 截断">"后的输出命令
}

void shell::Pipe(int& std_out, int& std_in)
{
    int i = 0;              //字符'|'位置标签
    for(; i <= dnum; ++i)   //获取待输出文件在命令参数字符数组的位置
    {
        if( strcmp("|",argv[i]) == 0 )
            break;
    }   
    
    argv[i] = NULL;

    char** argv2 = argv + i + 1;
    
    std_out = dup(STDOUT_FILENO);
    std_in = dup(STDIN_FILENO);

    //切分了命令 - argv("ls","-l",NULL) - argv2("grep","-r","PATH",NULL)
    
    //创建管道
    int pipefd[2] = {0};
    int ret = pipe(pipefd);
    Perror("pipe",ret);
    //创建两个子进程
    int k = 0;
    for(; k < 2; ++k)
    {
        int ret = fork();
        if(ret == 0)
            break;
    }

    if(k == 0)//1号子进程
    {
        close(pipefd[0]); //关闭读端

        dup2(pipefd[1], STDOUT_FILENO); //将标准输出重定向到写端

        execvp(argv[0], argv);
    }
    if(k == 1)//2号子进程
    {
        close(pipefd[1]); //关闭写端

        dup2(pipefd[0], STDIN_FILENO); //将标准输入重定向到读端
        
        execvp(argv2[0], argv2);
    }
    else //父进程
    {
        close(pipefd[0]);
        close(pipefd[1]); //关闭读写两端 - 无需向管道输入输出
        
        wait(NULL);       //回收子进程

    }   

}

