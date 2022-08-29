#include"../head/mycs.h"
#include"../head/mycs.cpp"
#include<sys/select.h>
#include<sys/fcntl.h>
#include<cmath>
//int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout); 
//void FD_CLR(int FD,fd_set*SET) 相当于是 set[FD] = 0;
//int FD_ISSET(int fd,fd_set *set) 相当于是 set.test(fd); 相当于return set[i];
//void FD_SET(int fd, fd_set * set) 相当于 set[fd] =1;
//void FD_ZERO( fd_set * set) 相当于 set.clear();

//所以 如果是 FD_ISSET(lsfd,&set)的话 相当于是 有连接请求来了
//如果是 FD_ISSET(cnfd,&set) 箱单于是 有读请求来了；
//把监听交给了内核，然后程序只需要管的事情是处理这些cnfd 添加什么的，读写，没响应的踢出去

//采取csapp的做法吧，有点pool的感觉了
//我的理解是 fd_set  是一个bitset<1024> set 
typedef struct {
    int maxFd;                  //read_set 的最大文件描述符
    fd_set readSet;             //所有的活跃描述符集合
    fd_set readyToReadSet;      //所有的可读集合，是上面的子集
    int nReady;                 //select 选出的多少个ready 文件描述符
    int maxIndex ;              //client 数组的高位
    int clientFd[FD_SETSIZE];   //1024
    rio_t clientRioBuf[FD_SETSIZE];   //缓冲
}pool;

int byteCnt = 0;

void initPool(int lsfd,pool *p){
    p->maxIndex = -1;
    for(int i = 0 ; i < FD_SETSIZE ;++i){
        p->clientFd[i] = -1;
    }
    //初始化槽位
    //刚刚开始的时候，lsfd只是readset的成员；
    p->maxFd = lsfd;           //刚开始是不是在3？
    FD_ZERO(&p->readSet);      //readset 全部置为0
    FD_SET(lsfd,&p->readSet);   //把listenfd挂到 全集里面去
}

void addClient(int cnfd, pool*p){
    int i;
    p->nReady--;                //处理掉一个连接请求
    //遍历 clientFd数组 去找一个 -1 的槽位给他用，如果你遍历完都没找到，说明满了
    for(i = 0 ; i < FD_SETSIZE;++i){
        if(p->clientFd[i] == -1){
            //找到了一个空的槽位
            //把连接的cnfd填进去
            p->clientFd[i] = cnfd;
            Rio_readinitb(&p->clientRioBuf[i],cnfd);        //把cnfd的信息填入对应的buffer

            //挂在readyToReadSet 里面
            FD_SET(cnfd,&p->readyToReadSet);                //挂到活跃的这个set里面
            //下面这个只是为了待会遍历更快，更新一下最大已用的文件描述符 ，也就是说在待会轮询，不要往上走，节约时间
            //经过很多次的这样插入删除之后 cnfd和i是错配的
            
            p->maxFd =max(cnfd,p->maxFd);
            p->maxIndex = max(i,p->maxIndex);
            break;
        }
    }
    if(i == FD_SETSIZE){
        err("Too many clients too handle...Bye");
    }
}
void refreshClients(pool *p){
    int index,tmpcnfd,numsRead;
    char buf[MAXLINE];
    rio_t tmprio;

    for(index = 3 ; (index <= p->maxIndex && p->nReady > 0) ;++index){          //好像可以从3 开始吧 
           tmpcnfd = p->clientFd[index];
           tmprio = p->clientRioBuf[index];                                 //搞得跟缓存池一样
           if(tmpcnfd > 0 && FD_ISSET(tmpcnfd,&p->readyToReadSet)){     //空槽的不要  ，这里只处理读写请求
                p->nReady--;    //处理掉一个读写请求
                if((numsRead = Rio_readlineb(&tmprio,buf,MAXLINE))){    //rio里面内置了 cnfd
                    byteCnt += numsRead;
                    std::cout<<"Server has received "<<numsRead<<"on port "<<tmpcnfd<<"total : "<<byteCnt<<".\n";
                    Rio_writen(tmpcnfd,buf,numsRead);
                }else{
                    Close(tmpcnfd);
                    FD_CLR(tmpcnfd,&p->readSet);               //这里就看出为什么主函数里面要把readyset 重新赋值了
                    p->clientFd[index] = -1;
                }    
           } 
    }
}

int main(){
    struct sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int lsfd ,cnfd;
    

    ServInit(server_addr,cli_addr);
    lsfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(lsfd,server_addr);
    Listen(lsfd,128);
    
    static pool p;
    initPool(lsfd,&p);

    while(true){
        p.readyToReadSet =  p.readSet;
        //select表演
        p.nReady = Select(p.maxFd + 1 ,&p.readyToReadSet,NULL,NULL,NULL);
        //参数一 你想监测多少个文件描述符，告诉内核  
        //参数二 要监听读事件 因为你是服务器
        //参数三 监听写事件 因为你是服务器，所以与你无瓜
        //参数四 监听异常事件，这仨都是fd_set类型 而且都是取地址
        //参数五，NULL 永远等，要么设置timeval 设置等一个固定值 如果是0  我也不到啊
        

        //我就干两件事情 碰到想连接的 让他连
        //碰到想读写的就读写呗
        if(FD_ISSET(lsfd,&p.readyToReadSet)){
            cnfd = Accept(lsfd,cli_addr,cli_len);
            addClient(cnfd,&p);                     //1024 不太大，这样做可以 ，但是不太适合poll epoll
            
        }                                           //相当于每轮只会连接一个 处理多个 
        refreshClients(&p);
    }
    return 0;
}