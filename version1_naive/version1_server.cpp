#include<iostream>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
using namespace std;

const int MAXLINE = 80;
const int SERVPORT = 6666;

//最简单的version
//
int main(){
    struct sockaddr_in sadd,carr;
    socklen_t cadd_len;
    int lsfd ,cnfd;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];
    int i ,n ;
    // 创建 listenfd;
    lsfd = socket(AF_INET,SOCK_STREAM,0);
    if(lsfd < 0){
        cerr<<"listen error"<<endl;
        exit(-1);
    }
    //初始化server

    bzero(&sadd,sizeof(sadd));
    sadd.sin_family = AF_INET;
    sadd.sin_addr.s_addr = htonl(INADDR_ANY);
    sadd.sin_port = htons(SERVPORT);

    //bind 把listenfd 绑定到server的结构体

    bind(lsfd,(struct sockaddr*)&sadd,sizeof(sadd));
    listen(lsfd,20);//listen 20个
    //到这已经开始监听了

    cout<<" I am listening ,huhu"<<endl;

    while(true){        //连上一个处理一个
        cadd_len = sizeof(carr);  //感觉在前面也可以初始化
        cnfd = accept(lsfd,(struct sockaddr *)&carr,&cadd_len);//阻塞在accept  
        //返回的时候 这个客户端的carr结构体作为传出参数 ，里面的内容已经填写好了 ，但是是网络格式的
        
        //所以这也是后面的改动  阻塞等待连接 一次连一个 读I/O 然后业务进程也在这里，又得等他读完处理，写回 这一套下来，然后又关掉这个连接

        //业务
        n = read(cnfd,buf,MAXLINE);             //读到这个程序的缓冲里面   这里也不检查一下 n
        cout<< "received from "<<inet_ntop(AF_INET,&carr.sin_addr,str,sizeof(str))<<"at port"<<ntohs(carr.sin_port)<<endl;
        for( i = 0 ; i < n ; ++i){
            buf[i] = toupper(buf[i]);
        }
        write(cnfd,buf,n);


        close(cnfd);
    }
    return 1;
}