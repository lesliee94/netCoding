//epoll yyds linux下的，甚至可以修改文件上限 vim /etc/security/limits.conf   写*  soft nofile 65536  * hard nofile 100000
#include"../head/mycs.h"
#include"../head/mycs.cpp"
#include<sys/epoll.h>
#include<vector>
#include<fcntl.h>
#include<sstream>
#include<iostream>
//这玩意貌似用的红黑树 
//先写一个EPOLLIN 默认水平触发(level triggered)，其实工作模式就和select/poll差不多 内核会通知你哪些文件描述符就绪，如果你不处理 他会继续提示你 
//当读到的长度小于请求的长度 就认为这个事件已经处理完成了
// typedef union epoll_data
// {
//   void *ptr;
//   int fd;
//   uint32_t u32;
//   uint64_t u64;
// } epoll_data_t;

// struct epoll_event
// {
//   uint32_t events;	/* Epoll events */
//   epoll_data_t data;	/* User data variable */
// } __EPOLL_PACKED;
//哪个脑子有包的写的这个

//1.int epoll_create (int __size)  。建立监听树 返回的是操作的句柄

//2.int epoll_ctl (int __epfd, int __op, int __fd,epoll_event *__event)
// 第一个参数是epoll_create的返回值
// 第二个参数表示要执行的动作，有以下几种参数可填：
// EPOLL_CTL_ADD：注册新的文件标识符到epfd中
// EPOLL_CTL_MOD：修改已经注册的fd的监听事件
// EPOLL_CTL_DEL：从epfd中删除一个文件标识符
// 第三个参数是需要监听的文件标识符
// 第四个参数是告诉内核需要监听什么事，

//3.int epoll_wait (int __epfd, struct epoll_event *__events,int __maxevents, int __timeout)
// 第一个参数就是前面epoll_create函数的返回值
// 参数events用来从系统内核得到事件的集合
// maxevents告知内核这个events有多大，不能大于创建epoll_create()时的size
// 参数timeout是超时时间（毫秒，0会立即返回，-1将永久阻塞（直到有事件发生）
// 该函数返回需要处理的事件数目，如返回0表示已超时。

//先写一个LT 阻塞的
//思路基本和poll差不多 
class pool{
public:
    int nReady;
    //int maxIndex;       //感觉这里不太需要这个玩意，鸡肋设计
    int capacity;
    int epfd;       //句柄 只创建一次
    bool isLt;
    //vector<int>clients;     //鸡肋设计 意义不大 让本来很快的东西变得很慢  你这个数组与 红黑树毫无对应关系 ，
    vector<struct epoll_event>totalEvents;
    //vector<rio_t> clientRioBuf;
    struct epoll_event tmp; 
    int cnt ;
    //listenfd 要设置成LT  阻塞
    //cnfd 如果ET 那么就非阻塞
    /***********************************/
    pool(int cap ,bool islt,int lsfd){
        this->capacity = cap;
        this->isLt =islt;
        this->nReady = 0;
        //this->maxIndex = -1;
        this->cnt = 0;
        //clients.resize(cap,-1);                  //我感觉这个数据结构还是需要的  仅仅是为了用缓冲 不然真的没什么用
        //clientRioBuf.resize(cap);                //感觉这个缓冲机制要么就废掉把，我感觉在epoll这里弊大于利
        totalEvents.resize(cap);
        epfd = epoll_create(cap);
        if(epfd < 0){
            err("Epoll create");
        }
        tmp.events = EPOLLIN ;
        tmp.data.fd = lsfd;
        int res;
        res = epoll_ctl(epfd,EPOLL_CTL_ADD,lsfd,&tmp);          //目前位置 ，只是把监听listenfd这活丢给了内核
        if(res < 0){
            err("Epoll add");
        }
    }
    ~pool(){
        Close(epfd);
    }
    void addClient(int cnfd){           //一次只添加一个连接 处理连接请求
        if(!isLt){            //需要阻塞
            int flag = fcntl(cnfd,F_GETFL);
            flag |= O_NONBLOCK;
            fcntl(cnfd,F_SETFL,flag);
        }
                
        tmp.events = isLt ? EPOLLIN : (EPOLLIN |EPOLLET);
        tmp.data.fd = cnfd;
        int res = epoll_ctl(epfd,EPOLL_CTL_ADD,cnfd,&tmp);
        if(res < 0){
            err("Add to tree.");
        }
    }
    void handleClients(int i){
        int numsRead ,tmpfd;
        char buf[MAXLINE];
        tmpfd = totalEvents[i].data.fd;
    again:
        numsRead = read(tmpfd,buf,MAXLINE);
        if(numsRead == -1){
            if(errno == EINTR){
                goto again;
            }else{
                err("Read.");
            }
        }else if(numsRead == 0){
            int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,tmpfd,NULL);
            if(ret < 0){
                err("Epoll_ctl");
            }
            Close(tmpfd);
            --cnt;
        }else{
            for(int i = 0 ; i < numsRead ; ++i){
                buf[i] = tolower(buf[i]);
            }
            Write(tmpfd,buf,numsRead);
        }
    }
};

int main(int argc ,char** argv){
    if(argc <3){
        err("invalid input.\n");
    }
    struct sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int lsfd ,cnfd,numsRead;
    stringstream ss;
    ss<<argv[1];
    string s1 = ss.str();
    int cap = stoi(s1);
    ss<<argv[2];
    s1 = ss.str();
    bool islt = (bool)stoi(s1);
   
    char str[16];
    ServInit(server_addr,cli_addr);
    lsfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(lsfd,server_addr);
    Listen(lsfd,128);
    
    static pool p(cap,islt,lsfd);               //lsfd在树根监听
    while(1){
        p.nReady = epoll_wait(p.epfd,&p.totalEvents[0],cap,-1);
        if(p.nReady == -1){
            err("epoll_wait.");
        }
        int i ,tmpfd;
        //敲黑板 这里是epoll与poll /select 最不同的地方，他给你返回的这个nready 你只需要去找你的结构体数组
        for ( i = 0 ; i < p.nReady ;++i){
            if(!(p.totalEvents[i].events & EPOLLIN)){
                continue;
            }
            //连接事件          不需要数组把，感觉需要一个cnt就完事了
            if(p.totalEvents[i].data.fd == lsfd){
                tmpfd = Accept(lsfd,cli_addr,cli_len);
                std::cout<<"connection from "<<inet_ntop(AF_INET,&cli_addr,str,cli_len)
                <<"at port "<<ntohs(cli_addr.sin_port)<<"established.\n";
                ++p.cnt;
                if(p.cnt >= p.capacity){
                    err("Too many clients,bye.\n");
                }
                p.addClient(tmpfd);
            }else{ //读事件
                p.handleClients(i);
            }
        }
    }
    Close(lsfd);
    return 0;
}