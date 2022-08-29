#include"../head/mycs.h"
#include"../head/mycs.cpp"
#include<vector>
//勉为其难写一个poll
#include<poll.h>

// int poll(struct pollfd* fds,nfds_t nfds,int timeout)
// struct pollfd{
//     int fd;
//     short events;     
//     short revents;
// }
//poll 监控的是一堆pollfd 第二个参数是 你要监控多少个 第三个参数 -1 阻塞等 0 立即返回 不阻塞  >0 等待指定毫秒数
//这些 事件 POLLIN = POLLRDNORM | POLLRDBAND  普通数据可读 或者优先带数据可读  POLLPRI 高优先级可读数据
//POLLOUT  POLLWRNORM  POLLWRBAND 
//POLLERR
class pool{
public:
    vector<struct pollfd> clientPfds;           //这里面自带fd 不用再申请一个数组来存了
    vector<rio_t>clientRioBuf;
    int cap;                // <= 65536
    int maxIndex;
    int maxFd;
    int nReady;
    pool(int cap_,int lsfd):cap(cap_){
        if(cap_ < 0){
            throw "invalid input.\n";
            exit(-1);
        }
        clientPfds.resize(cap_);
        clientRioBuf.resize(cap_);
        nReady = 0;
        maxIndex = 0;
        clientPfds[0].fd = lsfd;
        clientPfds[0].events = POLLRDNORM;      //监听普通读事件
        for(int i = 1;i < cap_;++i){
            clientPfds[i].fd = -1;
        }
    }

    //这里还没有写析构函数

    
    void addClient(int cnfd){
        int i ;
        --nReady;
        for(i = 0 ; i < cap;++i){
            if(clientPfds[i].fd == -1){
                clientPfds[i].fd = cnfd;
                rio_readinitb(&clientRioBuf[i],cnfd);
                clientPfds[i].events = POLLRDNORM;      //cnfd添加到普通读事件
                maxFd = max(maxFd,cnfd);
                maxIndex = max(maxIndex,i);
                break;
            }
        }
        if(i == cap){
            err("??????? so many ");
        }
    }
    void refreshClients(){
        int tmpfd ;
        rio_t tmprio;
        char buf[MAXLINE];
        int numsRead;
        int index = 1;
        for(;index < maxIndex ;++index){
            tmpfd = clientPfds[index].fd;
            if(tmpfd <0){
                continue;
            }
            if(clientPfds[index].revents&(POLLRDNORM | POLLERR)){
                numsRead = Rio_readlineb(&tmprio,buf,MAXLINE);  //这是处理过的函数
                if(numsRead < 0){
                    if(errno == ECONNRESET){
                        //RST
                        Close(tmpfd);
                        clientPfds[index].fd = -1;
                        std::cout<<tmpfd <<"has reset ,bye.\n";
                    }else{
                        err("read");
                    }
                }else if(numsRead == 0){
                    Close(tmpfd);
                    clientPfds[index].fd = -1;
                    std::cout<<tmpfd <<"has closed ,bye.\n";
                }else{
                    for(int i = 0 ; i < numsRead ;++i){
                        buf[i] = toupper(buf[i]);
                    }
                    Rio_writen(tmpfd,buf,numsRead);
                }
            }
            if(--nReady <= 0){
                break;
            }
        }
    }



   
};

int main(){
    struct sockaddr_in server_addr,cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int lsfd ,cnfd;
    char str[16];
    
    ServInit(server_addr,cli_addr);
    lsfd = Socket(AF_INET,SOCK_STREAM,0);
    Bind(lsfd,server_addr);
    Listen(lsfd,128);
    static pool pool(2048,lsfd);
    while(true){                                                   
        pool.nReady = poll(&pool.clientPfds[0],pool.maxIndex +1 ,-1);  //内核阻塞等，初始只监听一个 因为还没有连接

        if(pool.clientPfds[0].revents & POLLRDNORM){                 //实际上poll和select的逻辑差不多 一次只能添加一个
            //connection request
            cnfd = Accept(lsfd,cli_addr,cli_len);               //然后去轮询clientPfds 数组，看谁有事 ，处理 更新
            std::cout<<"new connection from"<<inet_ntop(AF_INET,&cli_addr,str,cli_len)
            <<"at port "<<ntohs(cli_addr.sin_port)<<"established.\n";
            pool.addClient(cnfd);
        }
        if(pool.nReady <=0){
            continue;
        }
        pool.refreshClients();
    }
    return 0;
}