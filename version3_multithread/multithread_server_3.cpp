//this demo using cpp thread;

#include"mycs.h"
#include"mycs.cpp"
#include<thread>
#include<mutex>

//做了一下测试，巨傻逼，表面多线程 其实根本不是 要是一个
//占用着port根本，下一个线程根本就没法进入，因为前一个还没有join

//先写一个有参数的函数吧

class worker{
public:
    int cnfd;
    struct sockaddr_in cli_addr;
    worker(int c,struct sockaddr_in cli):cnfd(c),cli_addr(cli){}
    char buf[MAXLINE];
    char str[16];
    void echo(){
        std::cout<<"connected with "<<inet_ntop(AF_INET,&cli_addr,str,16);
        std::cout<<" at port"<<ntohs(cli_addr.sin_port)<<" .\n";
        int numsRead = 0;
        while(true){
            numsRead = Read(cnfd,buf,MAXLINE);
            if(numsRead ==0){
                break;
            }
            Write(cnfd,buf,numsRead);
        }
        Close(cnfd);
    }

};


int main(void){
    struct sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int lsfd ,cnfd;
    int i = 0;
    
    ServInit(server_addr,cli_addr);
    lsfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(lsfd,server_addr);
    Listen(lsfd,128);

    std::cout<<"Waiting for connections.\n";

    while(true){
        cnfd = Accept(lsfd,cli_addr,cli_len);
        //这个代码有漏洞的 ，达到最大线程怎么办？ 只能continue；那些线程要是完事了 也不会
        if(i >=256){
            break;
        }
        worker w1(cnfd,cli_addr);
        std::thread tmp(&worker::echo,w1);

        //搁这干等
        tmp.join();
        
        //其实你这里又没有什么变化 你调用的无非是一个行为的东西而已 传的东西也没有太大的区别
        //你无非想把client的这个sockin_addr结构体的信息 以及 connectionfd传进去

        i++;
    }
    return 0;
}