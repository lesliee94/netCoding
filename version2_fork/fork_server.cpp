//this server is basically using fork to create child porcess
//并的不是很发

#include "../head/mycs.h"
void do_work( int confd);
void sigchld(int signo){
    pid_t pid;
    int stat;
    while((pid = waitpid(-1,&stat,WNOHANG))>0){
        std::cout<<"child "<<pid<<" has be reaped.\n";
    }
    return;
}
int main(void){
    //some variables
    sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    pid_t pid;
    int cnfd;
    void sigchld(int);
    char str[INET_ADDRSTRLEN];  //assert ... = 16 
    //void ServInit(struct sockaddr_in&ser,struct sockaddr_in&cli);
    ServInit(server_addr,cli_addr);         //ser port 9999 

    int lsfd = Socket(AF_INET,SOCK_STREAM,0); //ipv4 ,tcp ,生成listenfd句柄

    Bind(lsfd,server_addr);//当时设计的不太好 这里不太需要返回值 如果出错的话 会直接报错的
    Listen(lsfd,128);       //监听128个

    std::cout<<"waiting for connections...\n";


    Signal(SIGCHLD,sigchld);//注册回收子进程的信号，不然fork出来的子进程多了 死掉会有一堆僵尸


    //主循环 
    while(true){
        //int Accept(int&lsfd,struct sockaddr_in&cli,socklen_t &addrlen) return cnfd;
       
        cnfd = Accept(lsfd,cli_addr,cli_len);       //cnfd是被内核接受
        //printInfo(); 懒得写了
        std::cout<<"Received from "<<inet_ntop(AF_INET,&cli_addr.sin_addr,str,sizeof(str))
                <<" "<<"at port"<<ntohs(cli_addr.sin_port)<<"\n";
        //阻塞等待连接
        //返回就说明有连接了
        pid = Fork();
        if(pid >0){
            Close(cnfd);    //父进程关掉cnfd
        }else{
            //pid == 0
            Close(lsfd);// 子进程关掉listenfd
            do_work(cnfd);           //这里还是没有体现 业务与接受的分离，不过也可以自己修改这个handle函数，这样子的话
                                        //有一定的业务逻辑吧，但还是没有reactor这种模式强
            Close(cnfd);
            exit(0);
        }
        Close(lsfd);
        return 0;
    }
}
void do_work( int confd){
    int msg_nums;
    char buf[MAXLINE];          //assert MAXLINE =8192
    while(true){
        msg_nums = Read(confd,buf,MAXLINE); //这个函数是封装过的，考虑了  errno==ECONNABORTED||errno==EINTR
        //具体怎么处理这里也可以写一个函数 这里就随便做点事情就好
        if(msg_nums <0){
            return;
        }
        for(int i = 0 ; i < msg_nums ;++i){
            buf[i] = toupper(buf[i]);
        }
        Write(confd,buf,msg_nums);
    }
}