//this is the multithread server ,basically no much diffrences with multiprocess
#include"mycs.h"
#include"mycs.cpp"
#include<functional>

//要注意线程同步的问题
//客户端增加可能会导致某些线程不能及时得到cpu
//也可以 ps -t -pts/6 -o pid,ppid,tty,stat,args,wchan

//服务器启动之后 可以用netstat -a 查看服务器情况
struct s_info{
    struct sockaddr_in cli_addr;
    int cnfd;
};
//写一个回调
//补充一点函数指针的知识，
//c风格的就是函数指针
//lamda
//函数模板
void *do_business(void* arg){       //我即不知道他的返回值 也不知道他的参数？
    int n,i;
    struct s_info* ts =(struct s_info*)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    Pthread_detach(pthread_self());
    while(true){
        n = Read(ts->cnfd,buf,MAXLINE);
        if(n == 0){
            std::cout<<"client closed.\n";
            break;
        }
        std::cout<<"receive from"<<inet_ntop(AF_INET,&(*ts).cli_addr.sin_addr,str,sizeof(str))<<
        "at port "<<ntohs(ts->cli_addr.sin_port)<<" .\n";
        //懒得搞了直接写回
        Write(ts->cnfd,buf,1);
        sleep(1);       //这么做防止对面已经断开 
        Write(ts->cnfd,buf+1,n -1);
    }
    Close(ts->cnfd);

}

// class worker{

// public:


//     struct sockaddr_in addr;
//     int cnfd;


// }

// 思考 以上的这两个东西是不是可以合并成一个类 函数和数据呢



int main(void){
    struct sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int lsfd ,cnfd;
    int i = 0;
    pthread_t tid;
    struct s_info ts[256];

    ServInit(server_addr,cli_addr);
    lsfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(lsfd,server_addr);
    Listen(lsfd,128);

    std::cout<<"Waiting for connections.\n";

    while(true){
        cnfd = Accept(lsfd,cli_addr,cli_len);
        //这个代码有漏洞的 ，达到最大线程怎么办？ 只能continue；那些线程要是完事了 也不会
        if(i>=256){
            break;
        }
        ts[i].cli_addr = cli_addr;
        ts[i].cnfd = cnfd;
        //void Pthread_create(pthread_t *tidp, pthread_attr_t *attrp, void * (*routine)(void *), void *argp);

        Pthread_create(&tid,NULL,do_business,(void*)&ts[i]);//第四个参数取的结构体的地址

        //其实你这里又没有什么变化 你调用的无非是一个行为的东西而已 传的东西也没有太大的区别
        //你无非想把client的这个sockin_addr结构体的信息 以及 connectionfd传进去

        i++;
    }
    return 0;
}


