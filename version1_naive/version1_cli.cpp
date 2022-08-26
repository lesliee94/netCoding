#include<iostream>
#include<cstdlib>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>

const MAXLINE =80;
const SERPORT = 6666;

int main (int argc ,char*argv[]){
    struct sockaddr_in ser_addr;
    char buf[MAXLINE];
    int sockfd,n;
    char* str;
    if(argc != 2){
        cout<<"invalid input"<<endl;
        exit(-1);
    }
    str = argv[1];

    //简单把server的结构体填一下
    bzero(ser_addr,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    inet_pton(AF_INET,"81.68.190.180",&ser_addr.sin_addr);//不能直接填，只能用这个填他的ip
    ser_addr.sin_port = htons(SERVPORT);
    //不用bind  因为不需要固定端口号
    //所以第一步直接socket

    sockfd = socket(AF_INET,SOCK_STREAM,0);

    //第二步 阻塞连接
    connect(sockfd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));

    //然后就是业务

    write(sockfd,str,strlen(str));
    sleep(2);
    n = read(sockfd,buf,MAXLINE);
    cout<<"response from server:"<<endl;
    write(STOUT_FILENO,buf,n);

    //关闭句柄
    close(sockfd);

    return 0;

}